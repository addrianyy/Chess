#pragma once
#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace chess {

enum class Color : uint8_t {
  None = 0b00,
  White = 0b01,
  Black = 0b10,
};

Color other_color(Color color);

enum class Piece : uint8_t {
  None = 0b000,
  Pawn = 0b001,
  Bishop = 0b010,
  Knight = 0b011,
  Rook = 0b100,
  Queen = 0b101,
  King = 0b110,
  PawnGhost = 0b111,
};

struct Field {
  Color color : 2 = Color::None;
  Piece piece : 3 = Piece::None;
  bool moved : 1 = false;

  bool is_solid_piece() const { return piece != Piece::None && piece != Piece::PawnGhost; }
};
static_assert(sizeof(Field) == 1, "Field must be one byte");

struct Position {
  uint8_t x : 4 = 0;
  uint8_t y : 4 = 0;

  Position(int x, int y) : x(x), y(y) {}

  bool operator==(const Position other) const { return x == other.x && y == other.y; }
  bool operator!=(const Position other) const { return !(*this == other); }
};
static_assert(sizeof(Position) == 1, "Position must be one byte");

struct Move {
  Position from;
  Position to;
  bool captures : 1 = false;
  bool promotes : 1 = false;
  bool castles : 1 = false;
};

struct PlayerMove {
  Move move;
  Piece promotion;
};

class Board {
  std::array<Field, 64> fields{};

  /// The number of halfmoves since the last capture or pawn advance.
  int half_move_counter = 0;

  /// The number of the full move. It starts at 1, and is incremented after Black's move.
  int full_move_number = 1;

  uint8_t pawn_ghosts = 0;

  static inline size_t index_from_position(int x, int y) { return x + y * 8; }

  void set_field(int x, int y, Field field) { fields[index_from_position(x, y)] = field; }
  void set_field(Position position, Field field) { set_field(position.x, position.y, field); }

  std::vector<Move> calculate_moves_without_castling(Color player_turn) const;
  std::vector<Move> calculate_moves_with_castling(Color player_turn) const;

public:
  Board();

  std::vector<Move> calculate_legal_moves(Color player_turn) const;
  bool is_king_under_attack(Color player_turn) const;
  bool is_material_insufficient() const;

  void make_move(const Move& move, Piece promotion);
  void make_move(const PlayerMove& player_move);

  std::string get_fen_string(Color player_turn) const;

  int get_moves_since_capture_or_pawn_move() const { return half_move_counter / 2; }

  Field get_field(int x, int y) const { return fields[index_from_position(x, y)]; }
  Field get_field(Position position) const { return get_field(position.x, position.y); }
};

} // namespace chess

namespace std {

template <> struct hash<chess::Position> {
  std::size_t operator()(const chess::Position& k) const {
    return std::hash<uint8_t>()(k.x) ^ (std::hash<uint8_t>()(k.y) << 1);
  }
};

} // namespace std