#pragma once
#include "ChessViews.hpp"
#include "PieceRenderer.hpp"
#include "Renderer.hpp"
#include "View.hpp"
#include "ViewManager.hpp"
#include "Window.hpp"

class ChessView : public View {
protected:
  Window& window;
  Renderer& renderer;
  ViewManager& view_manager;
  ChessViews& chess_views;
  PieceRenderer& piece_renderer;

public:
  ChessView(Window& window, ChessViews& chess_views, PieceRenderer& piece_renderer)
      : window(window), renderer(window.get_renderer()), view_manager(window.get_view_manager()),
        chess_views(chess_views), piece_renderer(piece_renderer) {}
};