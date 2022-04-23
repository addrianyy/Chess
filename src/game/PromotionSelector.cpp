#include "PromotionSelector.hpp"
#include "ChessGame.hpp"
#include "Utils.hpp"

void PromotionSelector::update_cursor_position(int x, int y) {
  hovered_piece = std::nullopt;
  if (!cursor_in_window) {
    return;
  }

  int fx, fy;
  if (utils::map_coordinates(x, y, padding_x, padding_y, piece_size, pieces_count, 1, fx, fy)) {
    hovered_piece = fx;
  }
}

void PromotionSelector::on_resize_event(int width, int height) {
  screen_width = width;
  screen_height = height;

  piece_size = std::min(width / (pieces_count + 2), 350);
  padding_y = (height - piece_size) / 2;
  padding_x = (width - piece_size * pieces_count) / 2;

  size_token = piece_renderer.get_screen_size_token(piece_size);
}

void PromotionSelector::on_window_enter_event(bool entered) { cursor_in_window = entered; }

void PromotionSelector::on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x,
                                              int y) {
  if (!hovered_piece || button != sf::Mouse::Left || pressed) {
    return;
  }

  selected_piece = available_pieces[*hovered_piece];
  view_manager.set_view(chess_views.chess_game);
}

void PromotionSelector::on_key_event(sf::Keyboard::Key key, bool pressed) {
  if (key == sf::Keyboard::Escape && pressed) {
    view_manager.set_view(chess_views.chess_game);
  }
}

void PromotionSelector::on_update() {}

void PromotionSelector::on_render() {
  chess_views.chess_game->on_render();
  renderer.fill_rect(Rectangle{0, 0, screen_width, screen_height}, colors::overlay);

  for (int i = 0; i < std::size(available_pieces); ++i) {
    const auto x = padding_x + i * piece_size;
    const auto y = padding_y;

    piece_renderer.draw_piece(piece_color, available_pieces[i], x, y, size_token);

    if (i == hovered_piece) {
      const Rectangle rect{
        .x = x,
        .y = y,
        .w = piece_size,
        .h = piece_size,
      };

      renderer.outline_rect(rect, colors::outline, 6);
    }
  }
}

void PromotionSelector::on_switched_view(View* before) {
  window.set_cursor_shape(Window::CursorShape::Arrow);

  selected_piece = std::nullopt;
  hovered_piece = std::nullopt;
  cursor_in_window = window.is_focused();
}