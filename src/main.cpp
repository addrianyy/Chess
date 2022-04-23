#include "game/Run.hpp"

#ifdef USE_WINMAIN

#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  run();
  return 0;
}

#else

int main() { run(); }

#endif