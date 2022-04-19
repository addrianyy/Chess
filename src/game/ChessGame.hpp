#pragma once
#include "ChessView.hpp"

#include <chess/Board.hpp>

#include <optional>
#include <unordered_map>

class ChessGame : public ChessView {
  struct State {
    chess::Board board;
    chess::Color player_turn = chess::Color::White;
    std::optional<std::pair<chess::Position, chess::Position>> last_move = std::nullopt;
  };

  struct History {
    std::vector<State> states;
    size_t current_index = 0;
  };

  int screen_width = 1, screen_height = 1;
  int board_padding_x = 0, board_padding_y = 1;
  int board_field_size = 1;

  PieceScreenSizeToken size_token;

  bool cursor_in_window = false;
  int cursor_x = 0;
  int cursor_y = 0;

  std::optional<chess::Position> hovered_position;
  std::optional<chess::Position> moved_position;

  State state;
  History history;

  std::vector<chess::Move> all_possible_moves;
  bool king_under_attack = false;

  std::unordered_map<chess::Position, const chess::Move*> current_move_destinations;

  std::optional<chess::Move> pending_move;

  bool is_board_flipped();

  void make_move(chess::Move move, chess::Piece promotion);

  void on_turn_begin();
  void on_first_turn_begin();
  void on_turn_begin_no_history();

  void on_piece_grab(chess::Position position);
  void on_piece_return();
  void on_piece_drop(chess::Position from, chess::Position to);

  void set_historical_state(size_t state_index);
  void reset_game();

  void draw_board();

public:
  ChessGame(Window& window, ChessViews& chess_views, PieceRenderer& piece_renderer);

  void update_cursor_position(int x, int y) override;

  void on_resize_event(int width, int height) override;
  void on_window_enter_event(bool entered) override;

  void on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x, int y) override;
  void on_key_event(sf::Keyboard::Key key, bool pressed) override;

  void on_update() override;
  void on_render() override;

  void on_switched_view(View* before) override;
};