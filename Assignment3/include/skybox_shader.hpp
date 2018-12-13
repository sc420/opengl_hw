#pragma once

#include "as/gl/shader_app.hpp"

#include "scene_shader.hpp"

namespace shader {
class SkyboxShader : public SceneShader {
 public:
  /* GL initialization methods */

  void Init() override;

  void InitUniformBlocks();

  /* Name management */

  std::string GetId() const override;

 private:
};
}  // namespace shader
