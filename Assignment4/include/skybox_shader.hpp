#pragma once

#include "scene_shader.hpp"
#include "shader.hpp"

namespace shader {
class SceneShader;

class SkyboxShader : public Shader {
 public:
  /* Shader registrations */

  void RegisterSceneShader(const SceneShader &scene_shader);

  /* Model handlers */

  void LoadModel();

  /* GL initialization methods */

  void Init() override;

  void InitVertexArrays();

  void InitUniformBlocks();

  void InitTextures();

  /* GL drawing methods */

  void Draw() override;

  /* Name management */

  std::string GetId() const override;

  std::string GetTextureName() const;

 protected:
  /* Model handlers */

  virtual as::Model &GetModel() override;

  /* GL initialization methods */

  GLsizei GetNumMipmapLevels() const;

 private:
  std::shared_ptr<SceneShader> scene_shader_;
  as::Model skybox_model_;
};
}  // namespace shader
