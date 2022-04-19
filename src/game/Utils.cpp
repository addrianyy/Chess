#include "Utils.hpp"

bool utils::map_coordinates(int x, int y, int padding_x, int padding_y, int element_size, int width,
                            int height, int& out_x, int& out_y) {
  out_x = 0;
  out_y = 0;

  int fx = x - padding_x;
  int fy = y - padding_y;
  if (fx < 0 || fy < 0) {
    return false;
  }

  fx /= element_size;
  fy /= element_size;

  const bool outside = (fx < 0 || fx >= width) || (fy < 0 || fy >= height);
  if (!outside) {
    out_x = fx;
    out_y = fy;

    return true;
  }

  return false;
}