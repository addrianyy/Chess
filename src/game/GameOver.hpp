#pragma once
#include "ChessView.hpp"

class GameOver : public ChessView {
  sf::Font font;
  sf::Text text;

  int screen_width = 1, screen_height = 1;

  void update_text_position();

public:
  GameOver(Window& window, ChessViews& chess_views, PieceRenderer& piece_renderer);

  void set_text(const std::string& string);

  void update_cursor_position(int x, int y) override;

  void on_resize_event(int width, int height) override;
  void on_window_enter_event(bool entered) override;

  void on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x, int y) override;
  void on_key_event(sf::Keyboard::Key key, bool pressed) override;

  void on_update() override;
  void on_render() override;

  void on_switched_view(View* before) override;
};