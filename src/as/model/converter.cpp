#include "as\model\converter.hpp"

std::vector<GLubyte> as::ConvertDataChannels(const int old_num_channels,
                                             const int new_num_channels,
                                             const std::vector<GLubyte>& data) {
  if (old_num_channels == new_num_channels) {
    return data;
  }
  const size_t size = data.size();
  const size_t num_pixel = size / old_num_channels;
  const size_t new_size = num_pixel * new_num_channels;
  const size_t smallest_num_channels =
      std::min(old_num_channels, new_num_channels);
  std::vector<GLubyte> output(new_size, 0);
  for (size_t i = 0; i < num_pixel; i++) {
    for (size_t c = 0; c < smallest_num_channels; c++) {
      output[new_num_channels * i + c] = data[old_num_channels * i + c];
    }
  }
  return output;
}
