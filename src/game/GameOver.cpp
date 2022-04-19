#include "GameOver.hpp"
#include "ChessGame.hpp"
#include "Colors.hpp"
#include "binaries/Binaries.hpp"

GameOver::GameOver(Window& window, ChessViews& chess_views, PieceRenderer& piece_renderer)
    : ChessView(window, chess_views, piece_renderer) {
#ifndef USE_BUILTIN_BINARIES
  // TODO: Adjust font path for Linux.
  font.loadFromFile("C:/Windows/Fonts/arial.ttf");
#else
  font.loadFromMemory(binaries::font_data, binaries::font_data_size);
#endif

  text.setFont(font);
  text.setFillColor(sf::Color::White);
  text.setStyle(sf::Text::Bold);
}

void GameOver::update_text_position() {
  const int preferred_size = 64;

  text.setCharacterSize(preferred_size);

  {
    auto current_size = float(preferred_size);
    sf::FloatRect text_rect = text.getLocalBounds();

    while (text_rect.left + text_rect.width >= float(screen_width) * 0.8f) {
      current_size *= 0.8f;

      text.setCharacterSize(int(current_size));
      text_rect = text.getLocalBounds();
    }
  }

  const sf::FloatRect text_rect = text.getLocalBounds();

  text.setOrigin(text_rect.left + text_rect.width * 0.5f, text_rect.top + text_rect.height * 0.5f);
  text.setPosition(float(screen_width) * 0.5f, float(screen_height) * 0.5f);
}

void GameOver::set_text(const std::string& string) {
  text.setString(string);
  update_text_position();
}

void GameOver::update_cursor_position(int x, int y) {}

void GameOver::on_resize_event(int width, int height) {
  screen_width = width;
  screen_height = height;

  update_text_position();
}

void GameOver::on_window_enter_event(bool entered) {}

void GameOver::on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x, int y) {
  if (button == sf::Mouse::Left && pressed) {
    view_manager.set_view(chess_views.chess_game);
  }
}

void GameOver::on_key_event(sf::Keyboard::Key key, bool pressed) {
  if ((key == sf::Keyboard::Enter || key == sf::Keyboard::Escape || key == sf::Keyboard::Space) &&
      pressed) {
    view_manager.set_view(chess_views.chess_game);
  }
}

void GameOver::on_update() {}

void GameOver::on_render() {
  chess_views.chess_game->on_render();
  renderer.fill_rect(Rectangle{0, 0, screen_width, screen_height}, colors::overlay);

  renderer.draw_text(text);
}

void GameOver::on_switched_view(View* before) {
  window.set_cursor_shape(Window::CursorShape::Arrow);
  window.set_title("Chess [game over]");
}