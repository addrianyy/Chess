#include "PieceRenderer.hpp"
#include "binaries/Binaries.hpp"

struct PiecesTextureFile {
  struct PiecesTextureEntry {
    uint32_t tile_size;
    uint32_t offset;
    uint32_t size;
  };

  uint32_t magic;
  uint32_t count;
  PiecesTextureEntry entries[];
};

#ifndef USE_BUILTIN_BINARIES
#include <fstream>

static std::vector<uint8_t> read_binary_file(const std::string& path) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (file) {
    std::vector<uint8_t> buffer;
    file.seekg(0, std::ios::end);
    buffer.resize(file.tellg());

    file.seekg(0, std::ios::beg);
    file.read((char*)buffer.data(), int(buffer.size()));

    return buffer;
  }

  std::exit(1);
}
#endif

static int tile_index_for_piece(chess::Color color, chess::Piece piece) {
  const auto piece_tile_index = [&]() {
    switch (piece) {
    case chess::Piece::Pawn:
      return 0;
    case chess::Piece::Knight:
      return 1;
    case chess::Piece::Bishop:
      return 2;
    case chess::Piece::Rook:
      return 3;
    case chess::Piece::Queen:
      return 4;
    case chess::Piece::King:
      return 5;
    default:
      return -1;
    }
  }();

  if (piece_tile_index == -1) {
    return -1;
  }

  return (piece_tile_index * 2 + (color == chess::Color::Black ? 1 : 0));
}

size_t PieceRenderer::texture_index_for_size(int screen_size) const {
  if (screen_size <= textures.front().tile_size) {
    return 0;
  }

  if (screen_size >= textures.back().tile_size) {
    return textures.size() - 1;
  }

  size_t smaller_index = 0;
  size_t bigger_index = 0;

  for (size_t i = 0; i < textures.size(); ++i) {
    const auto current_size = textures[i].tile_size;

    if (screen_size == current_size) {
      return i;
    }

    if (screen_size < current_size) {
      smaller_index = i - 1;
      bigger_index = i;
      break;
    }
  }

  const auto smaller_size = textures[smaller_index].tile_size;
  const auto bigger_size = textures[bigger_index].tile_size;

  // Score both textures where lower score = better.
  // Smaller textures get 3x penalty.
  const auto smaller_score = (screen_size - smaller_size) * 3;
  const auto bigger_score = bigger_size - screen_size;

  if (smaller_score < bigger_score) {
    return smaller_index;
  } else {
    return bigger_index;
  }
}

PieceRenderer::PieceRenderer(Renderer& renderer) : renderer(renderer) {
#ifndef USE_BUILTIN_BINARIES
  const auto file = read_binary_file("pieces.dat");
  const auto bytes = file.data();
#else
  const auto bytes = binaries::pieces_data;
#endif

  const auto header = (const PiecesTextureFile*)bytes;

  if (header->magic != 'DCBA') {
    std::exit(1);
  }

  for (uint32_t i = 0; i < header->count; ++i) {
    const auto entry = header->entries[i];

    sf::Texture texture;
    texture.loadFromMemory(bytes + entry.offset, entry.size);
    texture.setSmooth(true);

    textures.push_back(PiecesTexture{
      .texture = texture,
      .tile_size = int(entry.tile_size),
    });
  }
}

PieceScreenSizeToken PieceRenderer::get_screen_size_token(int screen_size) {
  return {texture_index_for_size(screen_size), screen_size};
}

void PieceRenderer::draw_piece(chess::Color color, chess::Piece piece, int screen_x, int screen_y,
                               PieceScreenSizeToken screen_size_token) {
  const auto tile_index = tile_index_for_piece(color, piece);
  if (tile_index == -1) {
    return;
  }

  auto& pieces_texture = textures[screen_size_token.texture_index];

  Rectangle source{};
  source.x = 0;
  source.y = tile_index * pieces_texture.tile_size;
  source.w = pieces_texture.tile_size;
  source.h = pieces_texture.tile_size;

  Rectangle dest{};
  dest.x = screen_x;
  dest.y = screen_y;
  dest.w = screen_size_token.screen_size;
  dest.h = screen_size_token.screen_size;

  renderer.blit_texture(pieces_texture.texture, source, dest);
}