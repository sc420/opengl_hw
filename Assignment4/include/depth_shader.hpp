#pragma once

#include "shader.hpp"

namespace shader {
class DepthShader : public Shader {
 public:
  /* GL Initializations */

  void Init() override;
};
}  // namespace shader
