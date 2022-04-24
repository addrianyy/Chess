#pragma once

class ChessViews {
public:
  class ChessGame* chess_game;
  class GameOver* game_over;
  class PromotionSelector* promotion_selector;
  class WaitingForPlayerView* waiting_for_player;
};