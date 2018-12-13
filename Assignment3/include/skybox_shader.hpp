#pragma once

#include "as/gl/shader_app.hpp"

#include "scene_shader.hpp"

namespace shader {
class SkyboxShader : public SceneShader {
 public:
  std::string GetId() const override;

  void InitUniformBlocks() override;

 private:
};
}  // namespace shader
