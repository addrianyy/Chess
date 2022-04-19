#include "Window.hpp"

Window::Window(sf::RenderWindow& window) : window(window), renderer(window) {
  arrow_cursor.loadFromSystem(sf::Cursor::Arrow);
  hand_cursor.loadFromSystem(sf::Cursor::Hand);
}

void Window::set_cursor_shape(Window::CursorShape cursor_shape) {
  if (cursor_shape != last_cursor_shape) {
    last_cursor_shape = cursor_shape;

    switch (cursor_shape) {
    case CursorShape::Arrow:
      window.setMouseCursor(arrow_cursor);
      break;

    case CursorShape::Hand:
      window.setMouseCursor(hand_cursor);
      break;
    }
  }
}

void Window::set_title(const std::string& title) { window.setTitle(title); }

bool Window::is_focused() const { return focused; }

void Window::update_cursor_position(int x, int y) {
  view_manager.get_view()->update_cursor_position(x, y);
}

void Window::on_resize_event(int width, int height) {
  for (auto& view : get_view_manager().get_all_views()) {
    view->on_resize_event(width, height);
  }
}

void Window::on_window_enter_event(bool entered) {
  focused = entered;
  
  view_manager.get_view()->on_window_enter_event(entered);
}

void Window::on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x, int y) {
  view_manager.get_view()->on_mouse_button_event(button, pressed, x, y);
}

void Window::on_key_event(sf::Keyboard::Key key, bool pressed) {
  view_manager.get_view()->on_key_event(key, pressed);
}

void Window::on_update() { view_manager.get_view()->on_update(); }
void Window::on_render() { view_manager.get_view()->on_render(); }
