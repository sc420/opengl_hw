#include "as\model\converter.hpp"

std::vector<GLubyte> as::ConvertDataChannels3To4(
    const std::vector<GLubyte>& data) {
  const size_t size = data.size();
  const size_t num_pixel = size / 3;
  const size_t new_size = num_pixel * 4;
  std::vector<GLubyte> output(new_size, 0);
  for (size_t i = 0; i < num_pixel; i++) {
    for (size_t c = 0; c < 3; c++) {
      output[4 * i + c] = data[3 * i + c];
    }
  }
  return output;
}
