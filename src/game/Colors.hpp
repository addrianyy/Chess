#pragma once
#include <cstdint>

namespace colors {

constexpr uint32_t overlay = 0x101010d8;
constexpr uint32_t outline = 0xFFFFFFFF;
constexpr uint32_t background = 0x323232FF;

constexpr uint32_t board_normal[2] = {0xB58863FF, 0xF0D9B5FF};
constexpr uint32_t board_highlighted[2] = {0xDAC332FF, 0xF7EC5BFF};
constexpr uint32_t board_attacked_king = 0xFF3030FF;

constexpr uint32_t move_marker = 0x45454550;

} // namespace colors