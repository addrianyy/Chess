#pragma once
#include <SFML/Graphics.hpp>

class View {
public:
  virtual ~View() = default;

  virtual void update_cursor_position(int x, int y) = 0;

  virtual void on_resize_event(int width, int height) = 0;
  virtual void on_window_enter_event(bool entered) = 0;

  virtual void on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x, int y) = 0;
  virtual void on_key_event(sf::Keyboard::Key key, bool pressed) = 0;

  virtual void on_update() = 0;
  virtual void on_render() = 0;

  virtual void on_switched_view(View* before) = 0;
};