#include "ChessGame.hpp"
#include "Colors.hpp"
#include "GameOver.hpp"
#include "PromotionSelector.hpp"
#include "Utils.hpp"

#include <algorithm>

bool ChessGame::is_board_flipped() { return false; }

void ChessGame::make_move(chess::Move move, chess::Piece promotion) {
  state.last_move = {move.from, move.to};
  pending_move = std::nullopt;

  state.board.make_move(move, promotion);

  state.player_turn = chess::other_color(state.player_turn);

  on_piece_return();
  on_turn_begin();
}

void ChessGame::on_turn_begin() {
  // If it's not the last entry then shrink the history.
  if (history.current_index + 1 != history.states.size()) {
    history.states.resize(history.current_index + 1);
  }

  history.states.push_back(state);
  history.current_index++;

  on_turn_begin_no_history();
}

void ChessGame::on_first_turn_begin() {
  history.states.push_back(state);

  on_turn_begin_no_history();
}

void ChessGame::on_turn_begin_no_history() {
  all_possible_moves = state.board.calculate_legal_moves(state.player_turn);
  king_under_attack = state.board.is_king_under_attack(state.player_turn);

  const auto game_over = [&](const std::string& reason) {
    chess_views.game_over->set_text(reason);
    view_manager.set_view(chess_views.game_over);
  };

  if (all_possible_moves.empty()) {
    if (king_under_attack) {
      const auto winner =
        chess::other_color(state.player_turn) == chess::Color::White ? "White" : "Black";

      game_over("Checkmate! " + std::string(winner) + " wins!");
    } else {
      game_over("Draw via stalemate");
    }
  } else if (state.board.is_material_insufficient()) {
    game_over("Draw via insufficient material");
  } else if (state.board.get_moves_since_capture_or_pawn_move() >= 50) {
    game_over("Draw via 50 move rule");
  } else {
    const auto player = state.player_turn == chess::Color::White ? "white" : "black";
    const auto title = "Chess [" + std::string(player) + " are playing]";
    window.set_title(title);
  }
}

void ChessGame::on_piece_return() {
  moved_position = std::nullopt;
  current_move_destinations.clear();
}

void ChessGame::on_piece_grab(chess::Position position) {
  const auto field = state.board.get_field(position);
  if (!field.is_solid_piece() || field.color != state.player_turn) {
    return;
  }

  moved_position = hovered_position;

  current_move_destinations.clear();

  for (const auto& move : all_possible_moves) {
    if (move.from == position) {
      current_move_destinations.insert(std::pair{move.to, &move});
    }
  }
}

void ChessGame::on_piece_drop(chess::Position from, chess::Position to) {
  const auto possible_move_it = current_move_destinations.find(to);
  if (possible_move_it == end(current_move_destinations)) {
    return;
  }

  const auto& move = *possible_move_it->second;

  if (move.promotes) {
    pending_move = move;

    // Move cursor to the center of `to` field so it looks better.
    cursor_x = board_padding_x + int(to.x) * board_field_size + board_field_size / 2;
    cursor_y = board_padding_y + int(is_board_flipped() ? to.y : (7 - to.y)) * board_field_size +
               board_field_size / 2;

    chess_views.promotion_selector->set_color(state.player_turn);
    view_manager.set_view(chess_views.promotion_selector);
  } else {
    make_move(move, chess::Piece::Queen);
  }
}

void ChessGame::set_historical_state(size_t state_index) {
  if (history.current_index != state_index) {
    history.current_index = state_index;
    state = history.states[state_index];
    on_turn_begin_no_history();
  }
}

void ChessGame::reset_game() {
  on_piece_return();

  state = State{};
  history = History{};
  pending_move = std::nullopt;

  on_first_turn_begin();
}

void ChessGame::draw_board() {
  const bool flipped = is_board_flipped();

  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      const auto position = chess::Position(x, y);
      const auto moving_this_piece = position == moved_position;
      const auto part_of_last_move = state.last_move && (state.last_move->first == position ||
                                                         state.last_move->second == position);
      const auto highlighted = moving_this_piece || part_of_last_move;

      const auto field = state.board.get_field(x, y);

      const int screen_x = board_padding_x + x * board_field_size;
      const int screen_y = board_padding_y + (flipped ? y : (7 - y)) * board_field_size;

      const auto colors = highlighted ? colors::board_highlighted : colors::board_normal;
      const auto color =
        (king_under_attack && field.piece == chess::Piece::King && field.color == state.player_turn)
          ? colors::board_attacked_king
          : colors[(x + y) % 2];

      const Rectangle rect{
        .x = screen_x,
        .y = screen_y,
        .w = board_field_size,
        .h = board_field_size,
      };

      renderer.fill_rect(rect, color);

      if (!moving_this_piece) {
        piece_renderer.draw_piece(field.color, field.piece, screen_x, screen_y, size_token);
      }

      const auto it = current_move_destinations.find(position);
      if (it != end(current_move_destinations)) {
        if (it->second->captures) {
          const int radius = board_field_size / 2 - 2;
          const int thickness = board_field_size / 12;

          renderer.draw_ring(screen_x + board_field_size / 2 - radius,
                             screen_y + board_field_size / 2 - radius, radius, thickness,
                             colors::move_marker);
        } else {
          const int radius = board_field_size / 6;

          renderer.draw_circle(screen_x + board_field_size / 2 - radius,
                               screen_y + board_field_size / 2 - radius, radius,
                               colors::move_marker);
        }
      }

      if (moved_position && hovered_position == position) {
        renderer.outline_rect(rect, colors::outline, std::clamp(board_field_size / 24, 2, 8));
      }
    }
  }

  if (moved_position) {
    const auto field = state.board.get_field(*moved_position);
    piece_renderer.draw_piece(field.color, field.piece, cursor_x - board_field_size / 2,
                              cursor_y - board_field_size / 2, size_token);
  }
}

ChessGame::ChessGame(Window& window, ChessViews& chess_views, PieceRenderer& piece_renderer)
    : ChessView(window, chess_views, piece_renderer) {
  reset_game();
}

void ChessGame::update_cursor_position(int x, int y) {
  hovered_position = std::nullopt;
  if (!cursor_in_window) {
    return;
  }

  cursor_x = x;
  cursor_y = y;

  int fx, fy;
  if (utils::map_coordinates(x, y, board_padding_x, board_padding_y, board_field_size, 8, 8, fx,
                             fy)) {
    hovered_position = chess::Position(fx, is_board_flipped() ? fy : (7 - fy));
  }
}

void ChessGame::on_resize_event(int width, int height) {
  const auto bx = float(cursor_x - board_padding_x) / float(board_field_size * 8);
  const auto by = float(cursor_y - board_padding_y) / float(board_field_size * 8);
  const auto inside_board = (bx >= 0.f && bx < 1.f) && (by >= 0.f && by < 1.f);

  screen_width = width;
  screen_height = height;

  const auto screen_size = std::min(screen_width, screen_height);

  board_field_size = screen_size / 8;
  board_padding_x = (screen_width - (board_field_size * 8)) / 2;
  board_padding_y = (screen_height - (board_field_size * 8)) / 2;

  size_token = piece_renderer.get_screen_size_token(board_field_size);

  if (view_manager.get_view() == this) {
    on_piece_return();
  } else {
    // Fixup the cursor so it's in the same position on the board.
    if (inside_board) {
      cursor_x = int(float(board_padding_x) + bx * float(board_field_size * 8));
      cursor_y = int(float(board_padding_y) + by * float(board_field_size * 8));
    } else {
      on_piece_return();
    }
  }
}

void ChessGame::on_window_enter_event(bool entered) {
  cursor_in_window = entered;
  on_piece_return();
}

void ChessGame::on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x, int y) {
  if (button != sf::Mouse::Left || !hovered_position) {
    on_piece_return();
    return;
  }

  if (pressed) {
    on_piece_return();
    on_piece_grab(*hovered_position);
  } else {
    if (moved_position && *moved_position != *hovered_position) {
      on_piece_drop(*moved_position, *hovered_position);
    }

    if (!pending_move) {
      on_piece_return();
    }
  }
}

void ChessGame::on_key_event(sf::Keyboard::Key key, bool pressed) {
  if (!pressed) {
    return;
  }

  if (key == sf::Keyboard::R) {
    reset_game();
  }

  if (key == sf::Keyboard::Left && history.current_index > 0) {
    set_historical_state(history.current_index - 1);
  }
  if (key == sf::Keyboard::Right && history.current_index + 1 < history.states.size()) {
    set_historical_state(history.current_index + 1);
  }
}

void ChessGame::on_update() {
  bool playable_piece = false;

  if (moved_position) {
    playable_piece = true;
  } else if (hovered_position) {
    const auto field = state.board.get_field(*hovered_position);
    playable_piece = field.is_solid_piece() && field.color == state.player_turn;
  }

  window.set_cursor_shape(playable_piece ? Window::CursorShape::Hand : Window::CursorShape::Arrow);
}

void ChessGame::on_render() {
  renderer.clear(colors::background);
  draw_board();
}

void ChessGame::on_switched_view(View* before) {
  cursor_in_window = window.is_focused();
  hovered_position = std::nullopt;

  if (before == chess_views.game_over) {
    reset_game();
  }

  if (before == chess_views.promotion_selector) {
    if (const auto promotion_piece = chess_views.promotion_selector->get_selected_piece()) {
      make_move(*pending_move, *promotion_piece);
    } else {
      pending_move = std::nullopt;
      on_piece_return();
    }
  }
}