#pragma once
#include <string>

enum class MessageIcon {
  Warning = 0x30,
  Error = 0x10,
};

void message_box(const std::string& title, const std::string& text, MessageIcon icon);