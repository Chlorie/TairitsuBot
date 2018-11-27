#include <filesystem>

#include "othello_game.h"
#include "codename_tairitsu.h"
#include "../../utility/utility.h"

Result OthelloGame::give_up(const cq::Target& current_target, const std::string& message)
{
    if (!std::regex_match(message, give_up_regex)) return Result{};
    const int64_t group_id = *current_target.group_id;
    if (!games.contains(group_id)) return Result{};
    const MatchingPair pair = matching_pairs.get(group_id);
    const int64_t user_id = *current_target.user_id;
    if (user_id != pair.first_player && user_id != pair.second_player) return Result{};
    games.manipulate(group_id, [&](Game& game_info)
    {
        Othello& game = game_info.first;
        const std::string file_name = "othello" + std::to_string(group_id) + ".bmp";
        const std::string file_path = utility::image_path + file_name;
        board.save_board(file_path, game.get_state());
        std::ostringstream stream;
        stream << u8"由于" << (user_id == pair.first_player ? u8"黑方" : u8"白方") << u8"认输，本局结束。最终的局面如下，黑棋:白棋="
            << game.get_black_count() << ':' << game.get_white_count() << u8"。\n" << "[CQ:image,file=" << file_name << ']';
        send_message(current_target, stream.str(), false);
    });
    games.remove(group_id);
    matching_pairs.remove(group_id);
    end_game(group_id);
    return Result{ true, true };
}

Result OthelloGame::show_state(const cq::Target& current_target, const std::string& message)
{
    if (!std::regex_match(message, show_state_regex)) return Result{};
    const int64_t group_id = *current_target.group_id;
    if (!games.contains(group_id)) return Result{};
    const MatchingPair pair = matching_pairs.get(group_id);
    return games.manipulate(group_id, [&](Game& game_info)
    {
        Othello& game = game_info.first;
        const int64_t user_id = *current_target.user_id;
        if (user_id != pair.first_player && user_id != pair.second_player) return Result{};
        const std::string file_name = "othello" + std::to_string(group_id) + ".bmp";
        const std::string file_path = utility::image_path + file_name;
        board.save_board(file_path, game.get_state());
        std::ostringstream stream;
        stream << u8"目前的局面如下，黑棋:白棋=" << game.get_black_count() << ':' << game.get_white_count() << u8"。\n"
            << "[CQ:image,file=" << file_name << ']';
        cqc::api::send_group_msg(group_id, stream.str());
        return Result{ true, true };
    });
}

Result OthelloGame::solve_end_game(const cq::Target& current_target, const std::string& message)
{
    if (!std::regex_match(message, solve_end_game_regex)) return Result{};
    const int64_t group_id = *current_target.group_id;
    if (!games.contains(group_id)) return Result{};
    const MatchingPair pair = matching_pairs.get(group_id);
    return games.manipulate(group_id, [&](Game& game_info)
    {
        Othello& game = game_info.first;
        const int64_t user_id = *current_target.user_id;
        if (user_id != pair.first_player && user_id != pair.second_player) return Result{};
        const codename_tairitsu::EndGameTriplet result =
            codename_tairitsu::perfect_end_game_solution(game.get_state(), game.is_black());
        std::ostringstream stream;
        if (result.action == -1)
            stream << u8"ごめんね、目前棋盘上剩余空位还很多，我还没办法很快地计算出来最佳的终盘策略……";
        else
        {
            const int black = game.is_black() ? result.maximizer : result.minimizer;
            const int white = game.is_black() ? result.minimizer : result.maximizer;
            stream << u8"目前状态的完美终盘结果是，黑棋:白棋=" << black << ':' << white << u8"。";
        }
        send_message(current_target, stream.str(), false);
        return Result{ true, true };
    });
}

void OthelloGame::start_game(const int64_t group_id)
{
    const MatchingPair pair = randomize_pair(group_id);
    games.manipulate(group_id, [&](Game& game_info)
    {
        Othello& game = game_info.first;
        const std::string file_name = "othello" + std::to_string(group_id) + ".bmp";
        const std::string file_path = utility::image_path + file_name;
        board.save_board(file_path, game.get_state());
        std::ostringstream stream;
        stream << u8"那就开始了！" << utility::group_at(pair.first_player) << u8" 执黑，" << utility::group_at(pair.second_player)
            << u8" 执白。目前的局面如下，黑棋:白棋=" << game.get_black_count() << ':' << game.get_white_count() << u8"。\n"
            << "[CQ:image,file=" << file_name << ']';
        cqc::api::send_group_msg(group_id, stream.str());
        if (pair.first_player == utility::self_id) self_play(group_id);
    });
}

Result OthelloGame::process_play(const cq::Target& current_target, const std::string& message)
{
    if (message.length() != 2) return Result{};
    const char column_char = message[0], row_char = message[1];
    if ((column_char > 'h' || column_char < 'a') && (column_char > 'H' || column_char < 'A')) return Result{};
    if (row_char > '8' || row_char < '1') return Result{};
    const int64_t group_id = *current_target.group_id;
    if (!games.contains(group_id)) return Result{};
    const MatchingPair pair = matching_pairs.get(group_id);
    return games.manipulate(group_id, [&](Game& game_info)
    {
        Othello& game = game_info.first;
        const int64_t user_id = *current_target.user_id;
        if (user_id != (game.is_black() ? pair.first_player : pair.second_player)) return Result{};
        const int row = row_char - '1';
        const int column = column_char > 'H' ? column_char - 'a' : column_char - 'A';
        if (!(game.get_playable_spots() << (row * 8 + column) >> 63))
        {
            send_message(current_target, u8"你不能在这个地方落子。");
            return Result{ true };
        }
        play_at(group_id, row, column);
        return Result{ true, true };
    });
}

void OthelloGame::self_play(const int64_t group_id)
{
    games.manipulate(group_id, [&](Game& game_info)
    {
        Othello& game = game_info.first;
        const int decision = codename_tairitsu::take_action(game.get_state(), game.is_black());
        play_at(group_id, decision / 8, decision % 8);
    });
}

void OthelloGame::play_at(const int64_t group_id, const int row, const int column)
{
    const MatchingPair pair = matching_pairs.get(group_id);
    games.manipulate(group_id, [&](Game& game)
    {
        Othello& othello = game.first;
        std::ostringstream& history = game.second;
        const bool previous_black = othello.is_black();
        const Othello::Result result = othello.play(row, column);
        history << char('a' + column) << row + 1;
        const bool current_black = othello.is_black();
        const int64_t current_player = current_black ? pair.first_player : pair.second_player;
        const std::string file_name = "othello" + std::to_string(group_id) + ".bmp";
        const std::string file_path = utility::image_path + file_name;
        board.save_board(file_path, othello.get_state(), row, column);
        std::ostringstream stream;
        switch (result)
        {
        case Othello::Result::NotFinished:
            if (current_black == previous_black)
                stream << u8"由于" << (previous_black ? u8"白方" : u8"黑方") << u8"没有可以落子的位置，所以由"
                << (previous_black ? u8"黑方" : u8"白方") << utility::group_at(current_player)
                << u8" 继续落子。";
            else
                stream << u8"轮到" << (current_black ? u8"黑方" : u8"白方") << utility::group_at(current_player) << u8" 了。";
            stream << u8"目前";
            break;
        case Othello::Result::BlackWin:
            stream << u8"由于双方都无子可下，本局结束。黑方" << utility::group_at(pair.first_player) << u8" 获胜。最终";
            break;
        case Othello::Result::WhiteWin:
            stream << u8"由于双方都无子可下，本局结束。白方" << utility::group_at(pair.second_player) << u8" 获胜。最终";
            break;
        case Othello::Result::Draw:
            stream << u8"由于双方都无子可下，本局结束。双方打成平局。最终";
            break;
        }
        stream << "的局面如下，黑棋:白棋=" << othello.get_black_count() << ':' << othello.get_white_count() << u8"。\n"
            << "[CQ:image,file=" << file_name << ']';
        if (result != Othello::Result::NotFinished)
        {
            stream << u8"\n复盘：" << history.str();
            cqc::api::send_group_msg(group_id, stream.str());
            games.remove(group_id);
            matching_pairs.remove(group_id);
            end_game(group_id);
            return;
        }
        cqc::api::send_group_msg(group_id, stream.str());
        if (utility::self_id == (othello.is_black() ? pair.first_player : pair.second_player)) self_play(group_id);
    });
}

Result OthelloGame::process(const cq::Target& current_target, const std::string& message)
{
    if (!current_target.group_id.has_value()) return Result{};
    Result result = prepare_game(current_target, message);
    if (result.matched) return result;
    result = accept_challenge(current_target, message);
    if (result.matched) return result;
    result = cancel_challenge(current_target, message);
    if (result.matched) return result;
    result = give_up(current_target, message);
    if (result.matched) return result;
    result = show_state(current_target, message);
    if (result.matched) return result;
    result = solve_end_game(current_target, message);
    if (result.matched) return result;
    result = process_play(current_target, message);
    return result;
}

Result OthelloGame::process_creator(const std::string& message)
{
    if (message == "$activate othello game")
    {
        set_active(true);
        utility::private_send_creator(u8"有人要下黑白棋吗？");
        return Result{ true, true };
    }
    if (message == "$deactivate othello game")
    {
        set_active(false);
        utility::private_send_creator(u8"没什么人玩啊……");
        return Result{ true, true };
    }
    return Result{};
}
