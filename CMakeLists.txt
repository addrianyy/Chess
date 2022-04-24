cmake_minimum_required(VERSION 3.15)
project(Chess)

set(CMAKE_CXX_STANDARD 20)

if (NOT (CMAKE_BUILD_TYPE MATCHES Debug))
    set(SFML_STATIC_LIBRARIES TRUE)
endif ()

set(SFML_DIR deps/SFML/lib/cmake/SFML)
find_package(SFML 2.5 COMPONENTS system graphics window REQUIRED)

add_library(ChessLib src/game/ChessGame.cpp src/game/ChessGame.hpp src/chess/Board.cpp src/chess/Board.hpp src/game/Renderer.cpp src/game/Renderer.hpp src/game/View.cpp src/game/View.hpp src/game/ViewManager.cpp src/game/ViewManager.hpp src/game/Window.cpp src/game/Window.hpp src/game/ChessViews.cpp src/game/ChessViews.hpp src/game/GameOver.cpp src/game/GameOver.hpp src/game/PieceRenderer.cpp src/game/PieceRenderer.hpp src/game/PromotionSelector.cpp src/game/PromotionSelector.hpp src/game/ChessView.cpp src/game/ChessView.hpp src/game/Colors.cpp src/game/Colors.hpp src/game/Utils.cpp src/game/Utils.hpp src/game/binaries/PiecesData.cpp src/game/binaries/Binaries.hpp src/game/binaries/Font.cpp src/game/Run.cpp src/game/Run.hpp src/chess/BotIntegration.cpp src/chess/BotIntegration.hpp src/core/Process.cpp src/core/Process.hpp src/game/WaitingForPlayerView.cpp src/game/WaitingForPlayerView.hpp src/core/MessageBox.cpp src/core/MessageBox.hpp)
target_include_directories(ChessLib PRIVATE src)
target_link_libraries(ChessLib sfml-system sfml-window sfml-graphics)

set(NO_WINDOW TRUE)

if (WIN32 AND NO_WINDOW)
    add_executable(Chess WIN32 src/main.cpp)
    target_compile_definitions(Chess PRIVATE USE_WINMAIN)
else ()
    add_executable(Chess src/main.cpp)
endif ()

target_link_libraries(Chess ChessLib)