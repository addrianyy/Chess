#include "BotIntegration.hpp"

#include <array>
#include <filesystem>
#include <iostream>
#include <string>

using namespace chess;

void BotIntegration::synchronize() {
  process.write_line("isready");

  while (true) {
    if (process.read_line() == "readyok") {
      return;
    }
  }
}

std::string BotIntegration::wait_for_best_move() {
  while (true) {
    const auto line = process.read_line();

    if (line.starts_with("bestmove")) {
      const auto substring = line.substr(line.find(' ') + 1);
      return substring.substr(0, substring.find(' '));
    }
  }
}

void BotIntegration::queue_best_move_calculation(std::string fen) {
  std::unique_lock<std::mutex> lock(fen_lock);

  if (!queued_fen.empty() || is_calculation_queued) {
    std::exit(1);
  }

  best_move_atomic = invalid_best_move;
  queued_fen = std::move(fen);
  fen_cv.notify_one();

  is_calculation_queued = true;
}

BotIntegration::BotIntegration(const std::string& engine_path) : process(engine_path) {
  const auto engine = process.read_line();
  (void)engine;

  process.write_line("uci");
  synchronize();

  const std::array parameters = {
    std::pair{"Threads", "4"},
    std::pair{"Hash", "16"},
    std::pair{"Ponder", "false"},
    std::pair{"UCI_LimitStrength", "true"},

    // 1350 - 2850
    std::pair{"UCI_Elo", "2850"},
  };

  for (const auto [param, value] : parameters) {
    process.write_line("setoption name " + std::string(param) + " value " + std::string(value));
  }

  synchronize();

  thread = std::thread([this] {
    while (true) {
      std::string fen;
      {
        std::unique_lock<std::mutex> lock(fen_lock);
        fen_cv.wait(lock, [&]() { return exit_thread || !queued_fen.empty(); });

        fen = std::move(queued_fen);
        queued_fen.clear();
        best_move_atomic = invalid_best_move;

        if (exit_thread) {
          return;
        }
      }

      process.write_line("ucinewgame");
      synchronize();

      process.write_line("position fen " + fen);
      synchronize();

      process.write_line("go movetime 200");
      const auto best_move = wait_for_best_move();
      synchronize();

      // This should never happen as mates are detected before calling bot integration functions.
      if (best_move == "(none)") {
        std::exit(1);
      }

      const auto parse_position = [&](std::string_view s) {
        return uint32_t(s[0] - 'a') | (uint32_t(s[1] - '1') << 4);
      };

      auto move = (parse_position(best_move.substr(0, 2)) << 0) |
                  (parse_position(best_move.substr(2, 4)) << 8);

      // Promotion.
      if (best_move.size() == 5) {
        Piece piece = Piece::None;

        const auto c = best_move[4];
        if (c == 'q' || c == 'Q') {
          piece = Piece::Queen;
        } else if (c == 'n' || c == 'N') {
          piece = Piece::Knight;
        } else if (c == 'b' || c == 'B') {
          piece = Piece::Bishop;
        } else if (c == 'r' || c == 'R') {
          piece = Piece::Rook;
        }

        move |= uint32_t(piece) << 16;
      }

      best_move_atomic.store(move);
    }
  });
}

BotIntegration::~BotIntegration() {
  exit_thread = true;
  fen_cv.notify_one();
  thread.join();

  process.write_line("quit");
}

void BotIntegration::queue_best_move_calculation(const Board& board, Color player_turn) {
  auto fen = board.get_fen_string(player_turn);

  if (!is_calculation_queued) {
    queue_best_move_calculation(board.get_fen_string(player_turn));
  } else {
    pending_fen = std::move(fen);
  }
}

std::optional<PlayerMove>
BotIntegration::get_best_move(const std::vector<chess::Move>& all_possible_moves) {
  const auto best_move = best_move_atomic.load();
  if (best_move == invalid_best_move) {
    return std::nullopt;
  }

  best_move_atomic.store(invalid_best_move);
  is_calculation_queued = false;

  if (!pending_fen.empty()) {
    queue_best_move_calculation(std::move(pending_fen));
    pending_fen.clear();

    return std::nullopt;
  }

  const auto from = Position(int((best_move >> 0) & 0xf), int((best_move >> 4) & 0xf));
  const auto to = Position(int((best_move >> 8) & 0xf), int((best_move >> 12) & 0xf));
  const auto promotion = Piece((best_move >> 16) & 0xf);

  for (const auto move : all_possible_moves) {
    if (move.from == from && move.to == to) {
      return PlayerMove{move, promotion};
    }
  }

  // This is bad.
  std::exit(1);
}

std::unique_ptr<BotIntegration> chess::create_bot_integration() {
  for (const auto& entry : std::filesystem::directory_iterator(".")) {
    if (!entry.is_regular_file()) {
      continue;
    }

    const auto path = entry.path().filename();
    if (!path.string().starts_with("stockfish") || path.extension() != ".exe") {
      continue;
    }

    std::cout << "Using " << path << " engine." << std::endl;

    return std::make_unique<BotIntegration>(path.string());
  }

  return nullptr;
}