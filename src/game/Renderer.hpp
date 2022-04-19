#pragma once
#include "Colors.hpp"

#include <SFML/Graphics.hpp>

struct Rectangle {
  int x{};
  int y{};
  int w{};
  int h{};
};

class Renderer {
  sf::RenderWindow& window;

  sf::RectangleShape textured_rectangle;
  sf::RectangleShape filled_rectangle;
  sf::RectangleShape outlined_rectangle;

  sf::CircleShape circle;
  sf::CircleShape ring;

  static sf::Color convert_color(uint32_t color);

public:
  explicit Renderer(sf::RenderWindow& window);

  void clear(uint32_t color);

  void outline_rect(const Rectangle& rect, uint32_t color, int thickness = 1);
  void fill_rect(const Rectangle& rect, uint32_t color);
  void blit_texture(sf::Texture& texture, const Rectangle& source, const Rectangle& dest);

  void draw_circle(int x, int y, int radius, uint32_t color);
  void draw_ring(int x, int y, int radius, int thickness, uint32_t color);

  void draw_text(const sf::Text& text);
};