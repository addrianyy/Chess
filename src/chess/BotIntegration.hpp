#pragma once
#include "Board.hpp"

#include <core/Process.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace chess {

class BotIntegration {
  constexpr static uint32_t invalid_best_move = 0xffffffff;

  Process process;
  std::thread thread;

  std::mutex fen_lock;
  std::condition_variable fen_cv;
  std::string queued_fen;

  std::atomic_uint32_t best_move_atomic = invalid_best_move;
  std::atomic_bool exit_thread = false;

  bool is_calculation_queued = false;
  std::string pending_fen;

  void synchronize();

  std::string wait_for_best_move();

  void queue_best_move_calculation(std::string fen);

public:
  BotIntegration(const std::string& engine_path);
  ~BotIntegration();

  void queue_best_move_calculation(const Board& board, Color player_turn);
  std::optional<PlayerMove> get_best_move(const std::vector<chess::Move>& all_possible_moves);
};

std::unique_ptr<BotIntegration> create_bot_integration();

} // namespace chess