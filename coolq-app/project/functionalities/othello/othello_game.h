#pragma once

#include <sstream>
#include <utility>
#include <regex>

#include "../../processing/two_player_game.h"
#include "../../utility/dictionary.h"
#include "othello_board.h"

class OthelloGame final : public TwoPlayerGame
{
    using Game = std::pair<Othello, std::ostringstream>;
private:
    const OthelloBoard board;
    const std::regex show_state_regex{ u8"[ \t]*(?:显示)?当前局(?:势|面)[ \t]*" };
    const std::regex solve_end_game_regex{ u8"[ \t]*(?:显示)?(?:完美)?终盘解算[ \t]*" };
    Dictionary<Game> games;
    Result give_up(const cq::Target& current_target, const std::string& message);
    Result show_state(const cq::Target& current_target, const std::string& message);
    Result solve_end_game(const cq::Target& current_target, const std::string& message);
    void start_game(int64_t group_id) override;
    Result process_play(const cq::Target& current_target, const std::string& message);
    void self_play(int64_t group_id);
    void play_at(int64_t group_id, int row, int column);
protected:
    Result process(const cq::Target& current_target, const std::string& message) override;
    Result process_creator(const std::string& message) override;
public:
    OthelloGame() : TwoPlayerGame(u8"黑白棋", true) {}
    ~OthelloGame() override = default;
};
