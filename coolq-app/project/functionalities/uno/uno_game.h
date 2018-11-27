#pragma once

#include <vector>
#include <regex>
#include <utility>

#include "uno.h"
#include "../../processing/message_received.h"
#include "../../utility/dictionary.h"
#include "../../utility/utility.h"

class UnoGame final : public MessageReceived
{
private:
    const std::regex prepare_game_regex{ u8"[ \t]*来打[Uu][Nn][Oo]吧(?:(?:！|!| |\t))*" };
    const std::regex start_game_regex{ u8"[ \t]*那就开始吧(?:(?:！|!| |\t))*" };
    std::vector<Uno> games;
    Dictionary<std::pair<bool, std::vector<int64_t>>> players;
    Dictionary<bool> playing;
    int find_player_in_game(int64_t user_id) const;
    int get_player_id(int game_id, int64_t user_id) const;
    void send_other_players(int game_id, int player_id, const std::string& message) const;
    void send_self(const int game_id, const int player_id, const std::string& message) const
    {
        cqc::api::send_private_msg(games[game_id].get_players()[player_id].first, message);
    }
    void draw_card(int game_id, int player_id, int draw_amount);
    std::string show_cards(int game_id, int player_id, bool just_show = true) const;
    void notice_player(int game_id) const;
    void start_game(int game_id);
    void end_game(int game_id, bool forced = false);
    Result prepare_game(const cq::Target& current_target, const std::string& message);
    Result join_game(const cq::Target& current_target, const std::string& message);
    Result game_ready(const cq::Target& current_target, const std::string& message);
    Result check_help(const cq::Target& current_target, const std::string& message) const;
    Result check_list(const cq::Target& current_target, const std::string& message) const;
    Result check_more(const cq::Target& current_target, const std::string& message) const;
    Result check_give_up(const cq::Target& current_target, const std::string& message);
    Result check_send_message(const cq::Target& current_target, const std::string& message) const;
    Result check_play(const cq::Target& current_target, const std::string& message);
protected:
    Result process(const cq::Target& current_target, const std::string& message) override;
    Result process_creator(const std::string& message) override;
public:
    UnoGame() = default;
    ~UnoGame() override = default;
};
