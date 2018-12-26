#pragma once

#include "scene_shader.hpp"
#include "shader.hpp"

namespace shader {
class DepthShader : public Shader {
 public:
  /* Shader Registrations */

  void RegisterSceneShader(SceneShader &scene_shader);

  /* GL Initializations */

  void Init() override;

  void InitFramebuffers();

  void InitDepthTexture();

  /* GL Drawing Methods */

  void Draw() override;

  /* Name Management */

  std::string GetId() const override;

 protected:
  /* Name Management */

  std::string GetDepthFramebufferName() const;

  std::string GetDepthTextureName() const;

  std::string GetDepthTextureUnitName() const;

 private:
  /* Constants */

  static const glm::ivec2 kDepthMapSize;

  /* Shaders */

  SceneShader *scene_shader_;
};
}  // namespace shader
