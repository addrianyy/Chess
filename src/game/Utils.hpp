#pragma once

namespace utils {
bool map_coordinates(int x, int y, int padding_x, int padding_y, int element_size, int width,
                     int height, int& out_x, int& out_y);

}