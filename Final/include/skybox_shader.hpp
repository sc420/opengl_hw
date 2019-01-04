#pragma once

#include "scene_shader.hpp"
#include "shader.hpp"

namespace shader {
class SceneShader;

class SkyboxShader : public Shader {
 public:
  /* Shader Registrations */

  void RegisterSceneShader(const SceneShader &scene_shader);

  /* Model Handlers */

  void LoadModel();

  /* GL Initializations */

  void Init();

  void InitVertexArrays();

  void InitUniformBlocks();

  void InitTextures();

  /* GL Drawing Methods */

  void Draw();

  /* Name Management */

  std::string GetId() const override;

  std::string GetTextureName() const;

 protected:
  /* Model Handlers */

  as::Model &GetModel();

  /* GL Initializations */

  GLsizei GetNumMipmapLevels() const;

 private:
  const SceneShader *scene_shader_;

  as::Model skybox_model_;
};
}  // namespace shader
