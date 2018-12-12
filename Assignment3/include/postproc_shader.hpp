#pragma once

#include "as/gl/shader_app.hpp"

namespace shader {
class PostprocShader : public app::ShaderApp {
 public:
  std::string GetId() const override;
};
}  // namespace shader
