#pragma once

#include "scene_shader.hpp"

namespace shader {
class SkyboxShader : public SceneShader {
 public:
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

 protected:
  /* Model handlers */

  virtual as::Model &GetModel() override;

 private:
  as::Model skybox_model_;
};
}  // namespace shader
