#pragma once
#include "ChessView.hpp"

#include <optional>

class PromotionSelector : public ChessView {
  int screen_width = 1, screen_height = 1;

  int padding_x = 0, padding_y = 0;
  int piece_size = 1;

  PieceScreenSizeToken size_token;

  bool cursor_in_window = false;

  std::optional<int> hovered_piece;

  std::optional<chess::Piece> selected_piece;
  chess::Color piece_color = chess::Color::White;

  constexpr static chess::Piece available_pieces[4] = {chess::Piece::Queen, chess::Piece::Knight,
                                                       chess::Piece::Rook, chess::Piece::Bishop};

public:
  using ChessView::ChessView;

  void set_color(chess::Color color) { piece_color = color; }
  std::optional<chess::Piece> get_selected_piece() { return selected_piece; }

  void update_cursor_position(int x, int y) override;

  void on_resize_event(int width, int height) override;
  void on_window_enter_event(bool entered) override;

  void on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x, int y) override;
  void on_key_event(sf::Keyboard::Key key, bool pressed) override;

  void on_update() override;
  void on_render() override;

  void on_switched_view(View* before) override;
};