#include "Renderer.hpp"

sf::Color Renderer::convert_color(uint32_t color) {
  return sf::Color((color >> 24) & 0xff, (color >> 16) & 0xff, (color >> 8) & 0xff,
                   (color >> 0) & 0xff);
}

Renderer::Renderer(sf::RenderWindow& window) : window(window) {
  textured_rectangle.setOutlineThickness(0.f);

  filled_rectangle.setOutlineColor(sf::Color(0, 0, 0, 0));
  filled_rectangle.setOutlineThickness(0.f);

  outlined_rectangle.setFillColor(sf::Color(0, 0, 0, 0));
  outlined_rectangle.setOutlineThickness(1.f);

  circle.setPointCount(64);

  ring.setPointCount(64);
  ring.setFillColor(sf::Color(0, 0, 0, 0));
}

void Renderer::clear(uint32_t color) { window.clear(convert_color(color)); }

void Renderer::outline_rect(const Rectangle& rect, uint32_t color, int thickness) {
  outlined_rectangle.setPosition({float(rect.x), float(rect.y)});
  outlined_rectangle.setSize({float(rect.w), float(rect.h)});
  outlined_rectangle.setOutlineColor(convert_color(color));
  outlined_rectangle.setOutlineThickness(float(-thickness));

  window.draw(outlined_rectangle);
}

void Renderer::fill_rect(const Rectangle& rect, uint32_t color) {
  filled_rectangle.setPosition({float(rect.x), float(rect.y)});
  filled_rectangle.setSize({float(rect.w), float(rect.h)});
  filled_rectangle.setFillColor(convert_color(color));

  window.draw(filled_rectangle);
}

void Renderer::blit_texture(sf::Texture& texture, const Rectangle& source, const Rectangle& dest) {
  sf::IntRect source_{};
  source_.left = source.x;
  source_.top = source.y;
  source_.width = source.w;
  source_.height = source.h;

  textured_rectangle.setTexture(&texture);
  textured_rectangle.setTextureRect(source_);
  textured_rectangle.setPosition(float(dest.x), float(dest.y));
  textured_rectangle.setSize({float(dest.w), float(dest.h)});

  window.draw(textured_rectangle);
}

void Renderer::draw_circle(int x, int y, int radius, uint32_t color) {
  circle.setRadius(float(radius));
  circle.setPosition({float(x), float(y)});
  circle.setFillColor(convert_color(color));

  window.draw(circle);
}

void Renderer::draw_ring(int x, int y, int radius, int thickness, uint32_t color) {
  ring.setOutlineThickness(float(-thickness));
  ring.setOutlineColor(convert_color(color));
  ring.setRadius(float(radius));
  ring.setPosition({float(x), float(y)});

  window.draw(ring);
}

void Renderer::draw_text(const sf::Text& text) { window.draw(text); }