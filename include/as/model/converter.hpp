#pragma once

#include "as/common.hpp"

namespace as {
std::vector<GLubyte> ConvertDataChannels(const int old_num_channels,
                                         const int new_num_channels,
                                         const std::vector<GLubyte> &data);

glm::vec3 ConvertAiVectorToVec(const aiVector3D &ai_color);

glm::vec4 ConvertAiColorToVec(const aiColor4D &ai_color);

}  // namespace as
