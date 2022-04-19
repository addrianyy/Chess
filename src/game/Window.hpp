#pragma once
#include "Renderer.hpp"
#include "ViewManager.hpp"

#include <SFML/Graphics.hpp>

class Window {
public:
  enum class CursorShape {
    Arrow,
    Hand,
  };

private:
  sf::RenderWindow& window;
  Renderer renderer;
  ViewManager view_manager;

  CursorShape last_cursor_shape = CursorShape::Arrow;
  sf::Cursor arrow_cursor;
  sf::Cursor hand_cursor;

  bool focused = false;

public:
  explicit Window(sf::RenderWindow& window);

  Renderer& get_renderer() { return renderer; }
  ViewManager& get_view_manager() { return view_manager; }

  void set_cursor_shape(CursorShape cursor_shape);
  void set_title(const std::string& title);

  bool is_focused() const;

  void update_cursor_position(int x, int y);

  void on_resize_event(int width, int height);
  void on_window_enter_event(bool entered);
  void on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x, int y);
  void on_key_event(sf::Keyboard::Key key, bool pressed);

  void on_update();
  void on_render();
};