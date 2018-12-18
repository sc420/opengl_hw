#pragma once

#include "as/common.hpp"

namespace as {
std::vector<GLubyte> ConvertDataChannels(const int old_num_channels,
                                         const int new_num_channels,
                                         const std::vector<GLubyte>& data);
}
