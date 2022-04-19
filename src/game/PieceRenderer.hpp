#pragma once
#include "Renderer.hpp"

#include <chess/Board.hpp>

class PieceScreenSizeToken {
  friend class PieceRenderer;

  size_t texture_index = 0;
  int screen_size = 0;

  PieceScreenSizeToken(size_t texture_index, int screen_size)
      : texture_index(texture_index), screen_size(screen_size) {}

public:
  PieceScreenSizeToken() = default;
};

class PieceRenderer {
  struct PiecesTexture {
    sf::Texture texture;
    int tile_size = 0;
  };

  Renderer& renderer;

  std::vector<PiecesTexture> textures;

  size_t texture_index_for_size(int screen_size) const;

public:
  explicit PieceRenderer(Renderer& renderer);

  PieceScreenSizeToken get_screen_size_token(int screen_size);

  void draw_piece(chess::Color color, chess::Piece piece, int screen_x, int screen_y,
                  PieceScreenSizeToken screen_size_token);
};