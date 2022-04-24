#include "MessageBox.hpp"
#include <Windows.h>

void message_box(const std::string& title, const std::string& text, MessageIcon icon) {
  MessageBoxA(nullptr, text.c_str(), title.c_str(), DWORD(icon));
}
