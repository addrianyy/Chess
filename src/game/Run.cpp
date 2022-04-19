#include "Run.hpp"

#include <SFML/Graphics.hpp>

#include "ChessGame.hpp"
#include "GameOver.hpp"
#include "PromotionSelector.hpp"

#include "ChessViews.hpp"
#include "Window.hpp"

void run() {
  sf::ContextSettings context_settings{};
  context_settings.antialiasingLevel = 8;

  const auto desktop_mode = sf::VideoMode::getDesktopMode();
  const auto window_size = std::min(desktop_mode.width, desktop_mode.height) / 2;

  sf::RenderWindow window(sf::VideoMode(window_size, window_size), "Chess", sf::Style::Default,
                          context_settings);

  window.setFramerateLimit(144);

  Window game_window(window);
  ChessViews chess_views{};
  PieceRenderer piece_renderer(game_window.get_renderer());

  {
    auto& vm = game_window.get_view_manager();

    chess_views.chess_game =
      vm.add_view(std::make_unique<ChessGame>(game_window, chess_views, piece_renderer));
    chess_views.game_over =
      vm.add_view(std::make_unique<GameOver>(game_window, chess_views, piece_renderer));
    chess_views.promotion_selector =
      vm.add_view(std::make_unique<PromotionSelector>(game_window, chess_views, piece_renderer));

    vm.set_view(chess_views.chess_game);
  }

  {
    const auto size = window.getSize();
    game_window.on_resize_event(int(size.x), int(size.y));
  }

  while (window.isOpen()) {
    const auto update_cursor_position = [&]() {
      const auto cursor = sf::Mouse::getPosition(window);
      game_window.update_cursor_position(cursor.x, cursor.y);
    };

    update_cursor_position();

    for (sf::Event event{}; window.pollEvent(event);) {
      switch (event.type) {
      case sf::Event::MouseButtonPressed:
      case sf::Event::MouseButtonReleased:
        game_window.on_mouse_button_event(event.mouseButton.button,
                                          event.type == sf::Event::MouseButtonPressed,
                                          event.mouseButton.x, event.mouseButton.y);
        break;

      case sf::Event::MouseEntered:
      case sf::Event::MouseLeft:
        game_window.on_window_enter_event(event.type == sf::Event::MouseEntered);
        break;

      case sf::Event::KeyPressed:
      case sf::Event::KeyReleased:
        game_window.on_key_event(event.key.code, event.type == sf::Event::KeyPressed);
        break;

      case sf::Event::Resized:
        game_window.on_resize_event(int(event.size.width), int(event.size.height));
        window.setView(
          sf::View(sf::FloatRect(0.f, 0.f, float(event.size.width), float(event.size.height))));
        break;

      case sf::Event::Closed:
        window.close();
        break;

      default:
        break;
      }
    }

    update_cursor_position();

    game_window.on_update();
    game_window.on_render();

    window.display();
  }
}