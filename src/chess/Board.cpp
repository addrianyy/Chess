#include "Board.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>

using namespace chess;

Color chess::other_color(Color color) {
  switch (color) {
  case Color::White:
    return Color::Black;

  case Color::Black:
    return Color::White;

  default:
    return Color::None;
  }
}

static std::optional<Position> find_piece(const Board& board, Color color, Piece piece) {
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      const auto field = board.get_field(x, y);

      if (field.piece == piece && field.color == color) {
        return Position(x, y);
      }
    }
  }

  return std::nullopt;
}

static bool is_within_board(int x, int y) { return (x >= 0 && x < 8) && (y >= 0 && y < 8); }

static int pawn_move_direction(Color color) { return color == Color::White ? 1 : -1; }

static bool simple_movement_to(const Board& board, Position from, Position to, Field field,
                               std::vector<Move>& moves) {
  const auto target_field = board.get_field(to);
  if (target_field.is_solid_piece()) {
    if (target_field.color != field.color) {
      moves.push_back(Move{
        .from = from,
        .to = to,
        .captures = true,
      });
    }

    return false;
  } else {
    // En passant is only allowed for pawns so this move never captures (even if we move to
    // `PawnGhost` piece).
    moves.push_back(Move{
      .from = from,
      .to = to,
    });

    return true;
  }
}

static void straight_moves(const Board& board, int x, int y, Field field,
                           std::vector<Move>& moves) {
  for (const auto [ox, oy] :
       {std::pair{1, 0}, std::pair{0, 1}, std::pair{-1, 0}, std::pair{0, -1}}) {
    int fx = x + ox;
    int fy = y + oy;

    while (is_within_board(fx, fy)) {
      if (!simple_movement_to(board, Position(x, y), Position(fx, fy), field, moves)) {
        break;
      }

      fx += ox;
      fy += oy;
    }
  }
}

static void diagonal_moves(const Board& board, int x, int y, Field field,
                           std::vector<Move>& moves) {
  for (const auto [ox, oy] :
       {std::pair{1, 1}, std::pair{-1, -1}, std::pair{1, -1}, std::pair{-1, 1}}) {
    int fx = x + ox;
    int fy = y + oy;

    while (is_within_board(fx, fy)) {
      if (!simple_movement_to(board, Position(x, y), Position(fx, fy), field, moves)) {
        break;
      }

      fx += ox;
      fy += oy;
    }
  }
}

static void knight_moves(const Board& board, int x, int y, Field field, std::vector<Move>& moves) {
  for (int offset1 : {-2, 2}) {
    for (int offset2 : {-1, 1}) {
      {
        const int fx = x + offset1;
        const int fy = y + offset2;
        if (is_within_board(fx, fy)) {
          simple_movement_to(board, Position(x, y), Position(fx, fy), field, moves);
        }
      }

      {
        const int fx = x + offset2;
        const int fy = y + offset1;
        if (is_within_board(fx, fy)) {
          simple_movement_to(board, Position(x, y), Position(fx, fy), field, moves);
        }
      }
    }
  }
}

static void king_moves(const Board& board, int x, int y, Field field, std::vector<Move>& moves) {
  for (int oy = -1; oy <= 1; ++oy) {
    for (int ox = -1; ox <= 1; ++ox) {
      if (ox == 0 && oy == 0) {
        continue;
      }

      const int fx = x + ox;
      const int fy = y + oy;
      if (is_within_board(fx, fy)) {
        simple_movement_to(board, Position(x, y), Position(fx, fy), field, moves);
      }
    }
  }
}

static void pawn_moves(const Board& board, int x, int y, Field field, std::vector<Move>& moves) {
  const auto from = Position(x, y);

  const int move_direction = pawn_move_direction(field.color);
  const int max_movement = field.moved ? 1 : 2;

  for (int i = 1; i <= max_movement; ++i) {
    const int dy = y + i * move_direction;
    if (dy < 0 || dy >= 8) {
      break;
    }

    const auto target_field = board.get_field(x, dy);
    if (target_field.is_solid_piece()) {
      break;
    }

    const auto promotion = dy == 0 || dy == 7;
    moves.push_back(Move{
      .from = from,
      .to = Position(x, dy),
      .promotes = promotion,
    });
  }

  const auto diagonal_capture = [&](int x_offset) {
    const int dx = x + x_offset;
    const int dy = y + move_direction;
    if (!is_within_board(dx, dy)) {
      return;
    }

    const auto target_field = board.get_field(dx, dy);

    // Allow en passant capture.
    if (target_field.piece == Piece::None || target_field.color == field.color) {
      return;
    }

    const auto promotion = dy == 0 || dy == 7;
    moves.push_back(Move{
      .from = from,
      .to = Position(dx, dy),
      .captures = true,
      .promotes = promotion,
    });
  };

  diagonal_capture(-1);
  diagonal_capture(1);
}

static void calculate_moves_for_field(const Board& board, int x, int y, Field field,
                                      std::vector<Move>& moves) {
  switch (field.piece) {
  case Piece::Pawn:
    pawn_moves(board, x, y, field, moves);
    break;

  case Piece::Bishop:
    diagonal_moves(board, x, y, field, moves);
    break;

  case Piece::Knight:
    knight_moves(board, x, y, field, moves);
    break;

  case Piece::Rook:
    straight_moves(board, x, y, field, moves);
    break;

  case Piece::Queen:
    straight_moves(board, x, y, field, moves);
    diagonal_moves(board, x, y, field, moves);
    break;

  case Piece::King:
    king_moves(board, x, y, field, moves);
    break;

  default:
    break;
  }
}

Board::Board() {
  const auto set_piece = [&](int x, int y, Piece piece) {
    set_field(x, y, Field{Color::White, piece, false});
    set_field(x, 7 - y, Field{Color::Black, piece, false});
  };

  set_piece(0, 0, Piece::Rook);
  set_piece(7, 0, Piece::Rook);

  set_piece(1, 0, Piece::Knight);
  set_piece(6, 0, Piece::Knight);

  set_piece(2, 0, Piece::Bishop);
  set_piece(5, 0, Piece::Bishop);

  set_piece(3, 0, Piece::Queen);
  set_piece(4, 0, Piece::King);

  for (int x = 0; x < 8; ++x) {
    set_piece(x, 1, Piece::Pawn);
  }
}

void Board::make_move(const Move& move, Piece promotion) {
  const auto from_field = get_field(move.from);
  const auto to_field = get_field(move.to);

  if (from_field.piece == Piece::Pawn || move.captures) {
    half_move_counter = 0;
  } else {
    half_move_counter++;
  }

  if (from_field.color == Color::Black) {
    full_move_number++;
  }

  // Move piece from `from` to `to`. Promote it if needed.
  set_field(move.from, Field{});
  set_field(move.to, Field{from_field.color, move.promotes ? promotion : from_field.piece, true});

  // Handle en passant capture.
  if (move.captures && to_field.piece == Piece::PawnGhost) {
    set_field(Position(move.to.x, move.to.y + pawn_move_direction(to_field.color)), Field{});
    pawn_ghosts--;
  }

  // Pawn ghosts are valid for only one turn. Remove them.
  if (pawn_ghosts > 0) {
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        if (get_field(x, y).piece == Piece::PawnGhost) {
          set_field(x, y, Field{});
        }
      }
    }

    pawn_ghosts = 0;
  }

  // Moving pawn by 2 places leaves PawnGhost behind to handle en passant capture.
  if (from_field.piece == Piece::Pawn) {
    if (std::abs(int(move.from.y) - int(move.to.y)) == 2) {
      set_field(Position(move.to.x, move.to.y - pawn_move_direction(from_field.color)),
                Field{from_field.color, Piece::PawnGhost, true});
      pawn_ghosts++;
    }
  }

  // Handle castling.
  if (move.castles) {
    // -1 = left
    // +1 = right
    const int direction = (int(move.to.x) - int(move.from.x)) / 2;

    const auto rook_pos = Position(direction == -1 ? 0 : 7, move.from.y);
    const auto rook_dest = Position(move.from.x + direction, move.from.y);
    const auto rook = get_field(rook_pos);

    // Move piece from `rook_pos` to `rook_dest`.
    set_field(rook_pos, Field{});
    set_field(rook_dest, Field{rook.color, rook.piece, true});
  }
}

void Board::make_move(const PlayerMove& player_move) {
  make_move(player_move.move, player_move.promotion);
}

std::vector<Move> Board::calculate_moves_without_castling(Color player_turn) const {
  std::vector<Move> moves;

  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      const auto field = get_field(x, y);
      if (!field.is_solid_piece() || field.color != player_turn) {
        continue;
      }

      calculate_moves_for_field(*this, x, y, field, moves);
    }
  }

  return moves;
}

std::vector<Move> Board::calculate_moves_with_castling(Color player_turn) const {
  auto moves = calculate_moves_without_castling(player_turn);

  const auto king_opt = find_piece(*this, player_turn, Piece::King);
  if (!king_opt) {
    return moves;
  }

  const auto king_pos = *king_opt;
  const auto king_field = get_field(king_pos);
  if (king_field.moved) {
    return moves;
  }

  bool castling_left_blocked = false;
  bool castling_right_blocked = false;

  for (const auto move : calculate_moves_without_castling(other_color(player_turn))) {
    const auto to = move.to;

    // We cannot castle if the king is being attacked.
    if (to == king_pos) {
      return moves;
    }

    // Castling left would make king move through the attacked field.
    if (to == Position(king_pos.x - 1, king_pos.y) || to == Position(king_pos.x - 2, king_pos.y)) {
      castling_left_blocked = true;
    }

    // Castling right would make king move through the attacked field.
    if (to == Position(king_pos.x + 1, king_pos.y) || to == Position(king_pos.x + 2, king_pos.y)) {
      castling_right_blocked = true;
    }
  }

  if (castling_left_blocked && castling_right_blocked) {
    return moves;
  }

  const auto left_rook = get_field(Position(0, king_pos.y));
  const auto right_rook = get_field(Position(7, king_pos.y));

  const auto is_valid_rook = [&](Field field) {
    return !field.moved && field.piece == Piece::Rook && field.color == player_turn;
  };

  // [from, to)
  const auto is_path_clear = [&](int from, int to) {
    for (int x = from; x < to; ++x) {
      if (get_field(x, king_pos.y).is_solid_piece()) {
        return false;
      }
    }

    return true;
  };

  const auto add_castling_move = [&](int offset) {
    moves.push_back(Move{
      .from = king_pos,
      .to = Position(king_pos.x + offset, king_pos.y),
      .castles = true,
    });
  };

  if (!castling_left_blocked && is_valid_rook(left_rook) && is_path_clear(1, king_pos.x)) {
    add_castling_move(-2);
  }

  if (!castling_right_blocked && is_valid_rook(right_rook) && is_path_clear(king_pos.x + 1, 7)) {
    add_castling_move(2);
  }

  return moves;
}

std::vector<Move> Board::calculate_legal_moves(Color player_turn) const {
  auto moves = calculate_moves_with_castling(player_turn);

  std::erase_if(moves, [&](const Move& move) {
    // Simulate this move.
    auto after_move = *this;
    after_move.make_move(move, Piece::Queen);

    // Disallow moves which would expose this player's king.
    return after_move.is_king_under_attack(player_turn);
  });

  return moves;
}

bool Board::is_king_under_attack(Color player_turn) const {
  // Castling won't affect the king anyway so just skip it.
  for (const auto& move : calculate_moves_without_castling(other_color(player_turn))) {
    const auto field = get_field(move.to);
    if (field.piece == chess::Piece::King && field.color == player_turn) {
      return true;
    }
  }

  return false;
}

bool Board::is_material_insufficient() const {
  using Pieces = std::vector<std::pair<Piece, bool>>;

  Pieces p1, p2;
  {
    Pieces pieces[2];

    // Collect pieces of every color.
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        const auto field = get_field(x, y);
        if (!field.is_solid_piece()) {
          continue;
        }

        pieces[field.color == Color::White ? 0 : 1].push_back({field.piece, (x + y) % 2 == 0});
      }
    }

    p1 = std::move(pieces[0]);
    p2 = std::move(pieces[1]);
  }

  if (p1.size() > p2.size()) {
    std::swap(p1, p2);
  }

  // Assumption: king is always present.

  if (p1.size() == 1) {
    // king against king
    if (p2.size() == 1) {
      return true;
    }

    // king against king and bishop
    // king against king and knight
    if (p2.size() == 2) {
      const auto a = p2[0].first;
      const auto b = p2[1].first;
      if (a == Piece::Bishop || b == Piece::Bishop || a == Piece::Knight || b == Piece::Knight) {
        return true;
      }
    }
  }

  if (p1.size() == 2 && p2.size() == 2) {
    // king and bishop against king and bishop, with both bishops on squares of the same color
    const auto a = p1[0].first == Piece::King ? p1[1] : p1[0];
    const auto b = p2[0].first == Piece::King ? p2[1] : p2[0];
    if (a.first == Piece::Bishop && b.first == Piece::Bishop && a.second == b.second) {
      return true;
    }
  }

  return false;
}

std::string Board::get_fen_string(Color player_turn) const {
  std::string result;

  const auto piece_to_char = [&](Color color, Piece piece) -> char {
    char c = ' ';

    switch (piece) {
    case Piece::Pawn:
      c = 'p';
      break;
    case Piece::Bishop:
      c = 'b';
      break;
    case Piece::Knight:
      c = 'n';
      break;
    case Piece::Rook:
      c = 'r';
      break;
    case Piece::Queen:
      c = 'q';
      break;
    case Piece::King:
      c = 'k';
      break;
    default:
      break;
    }

    return color == Color::White ? char(std::toupper(c)) : c;
  };

  // Pieces placement.
  for (int y = 7; y >= 0; --y) {
    for (int x = 0; x < 8; ++x) {
      const auto field = get_field(x, y);
      if (field.is_solid_piece()) {
        result += piece_to_char(field.color, field.piece);
      } else {
        int count = 1;

        while (true) {
          const int nx = x + 1;
          if (nx >= 8 || get_field(nx, y).is_solid_piece()) {
            break;
          }

          count++;
          x++;
        }

        result += std::to_string(count);
      }
    }

    if (y != 0) {
      result += "/";
    }
  }

  // Player turn.
  result += player_turn == Color::White ? " w " : " b ";

  // Castling rights.
  bool any_castling_rights = false;
  for (const auto color : {Color::White, Color::Black}) {
    const auto y = color == Color::White ? 0 : 7;
    const auto king = get_field(4, y);
    if (king.piece != Piece::King || king.moved) {
      continue;
    }

    const auto king_side = get_field(7, y);
    const auto queen_side = get_field(0, y);

    if (king_side.piece == Piece::Rook && !king_side.moved) {
      result += piece_to_char(color, Piece::King);
      any_castling_rights = true;
    }

    if (queen_side.piece == Piece::Rook && !queen_side.moved) {
      result += piece_to_char(color, Piece::Queen);
      any_castling_rights = true;
    }
  }

  if (any_castling_rights) {
    result += ' ';
  } else {
    result += "- ";
  }

  // En passant.
  bool en_passant_available = false;
  if (pawn_ghosts > 0) {
    if (const auto ghost = find_piece(*this, other_color(player_turn), Piece::PawnGhost)) {
      const char chars[] = "abcdefgh";
      result += chars[ghost->x];
      result += std::to_string(int(ghost->y) + 1);
      en_passant_available = true;
    }
  }

  if (en_passant_available) {
    result += ' ';
  } else {
    result += "- ";
  }

  result += std::to_string(half_move_counter) + " " + std::to_string(full_move_number);

  return result;
}