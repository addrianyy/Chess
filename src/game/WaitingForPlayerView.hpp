#pragma once
#include "ChessView.hpp"

#include <chess/Board.hpp>

#include <optional>

class WaitingForPlayerView : public ChessView {
  std::optional<chess::PlayerMove> player_move;

  sf::Clock clock;

  float animation_start_t = 0.f;
  float animation_end_t = 0.f;

  float animation_start_x = 0.f, animation_start_y = 0.f;
  float animation_end_x = 0.f, animation_end_y = 0.f;

  bool interrupted = false;

  float get_current_time();
  float get_animation_t();

  void exit_view();

public:
  using ChessView::ChessView;

  std::optional<chess::PlayerMove> get_player_move() { return player_move; }
  bool was_interrupted() const { return interrupted; }

  void update_cursor_position(int x, int y) override;

  void on_resize_event(int width, int height) override;
  void on_window_enter_event(bool entered) override;

  void on_mouse_button_event(sf::Mouse::Button button, bool pressed, int x, int y) override;
  void on_key_event(sf::Keyboard::Key key, bool pressed) override;

  void on_update() override;
  void on_render() override;

  void on_switched_view(View* before) override;
};