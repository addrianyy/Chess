#include "WaitingForPlayerView.hpp"
#include "ChessGame.hpp"

float WaitingForPlayerView::get_current_time() { return clock.getElapsedTime().asSeconds(); }
float WaitingForPlayerView::get_animation_t() {
  return (get_current_time() - animation_start_t) / (animation_end_t - animation_start_t);
}

void WaitingForPlayerView::exit_view() {
  chess_views.chess_game->hidden_position = std::nullopt;
  view_manager.set_view(chess_views.chess_game);
}

void WaitingForPlayerView::update_cursor_position(int x, int y) {}

void WaitingForPlayerView::on_resize_event(int width, int height) {
  if (player_move) {
    exit_view();
  }
}

void WaitingForPlayerView::on_window_enter_event(bool entered) {}

void WaitingForPlayerView::on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x,
                                                 int y) {}

void WaitingForPlayerView::on_key_event(sf::Keyboard::Key key, bool pressed) {
  if (pressed && key == sf::Keyboard::R) {
    interrupted = true;

    exit_view();
    chess_views.chess_game->reset_game();
  }
}

void WaitingForPlayerView::on_update() {
  if (!player_move) {
    const auto chess_game = chess_views.chess_game;

    if ((player_move =
           chess_game->bot_integration->get_best_move(chess_game->all_possible_moves))) {
      const auto& move = player_move->move;

      chess_game->state.last_move = {move.from, move.to};
      chess_game->hidden_position = move.from;

      const auto map_coords = [&](chess::Position position) {
        const auto field_size = float(chess_game->board_field_size);

        const float x = float(chess_game->board_padding_x) + float(position.x) * field_size;
        const float y =
          float(chess_game->board_padding_y) +
          float(chess_game->is_board_flipped() ? position.y : (7 - position.y)) * field_size;

        return std::pair{x, y};
      };

      std::tie(animation_start_x, animation_start_y) = map_coords(move.from);
      std::tie(animation_end_x, animation_end_y) = map_coords(move.to);

      animation_start_t = get_current_time();
      animation_end_t = get_current_time() + 0.12f;
    }
  }

  if (player_move && get_animation_t() > 1.f) {
    exit_view();
  }
}

void WaitingForPlayerView::on_render() {
  chess_views.chess_game->on_render();

  if (player_move) {
    const auto t = std::clamp(get_animation_t(), 0.f, 1.f);
    const auto x = std::lerp(animation_start_x, animation_end_x, t);
    const auto y = std::lerp(animation_start_y, animation_end_y, t);

    const auto field = chess_views.chess_game->state.board.get_field(player_move->move.from);
    piece_renderer.draw_piece(field.color, field.piece, int(x), int(y),
                              chess_views.chess_game->size_token);
  }
}

void WaitingForPlayerView::on_switched_view(View* before) {
  window.set_cursor_shape(Window::CursorShape::Arrow);
  player_move = std::nullopt;
  interrupted = false;
}
