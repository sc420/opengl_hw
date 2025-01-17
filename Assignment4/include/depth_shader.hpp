#pragma once

#include "as/trans/camera.hpp"

#include "scene_shader.hpp"
#include "shader.hpp"
#include "trans_dto.hpp"

namespace shader {
class SceneShader;

class DepthShader : public Shader {
 public:
  /* Shader Registrations */

  void RegisterSceneShader(SceneShader &scene_shader);

  /* GL Initializations */

  void Init() override;

  void InitFramebuffers();

  void InitUniformBlocks();

  void InitDepthTexture();

  /* GL Drawing Methods */

  void Draw(const glm::ivec2 &window_size);

  void DrawModelWithoutTextures(const as::Model &model,
                                const std::string &group_name);

  dto::GlobalTrans GetLightTrans() const;

  void UseDepthFramebuffer();

  /* Name Management */

  std::string GetId() const override;

  std::string GetDepthTextureName() const;

  std::string GetDepthTextureUnitName() const;

 protected:
  /* Name Management */

  std::string GetDepthFramebufferName() const;

 private:
  /* Constants */

  static const glm::ivec2 kDepthMapSize;

  /* Shaders */

  SceneShader *scene_shader_;

  /* GL States */

  dto::GlobalTrans global_trans_;
  dto::ModelTrans model_trans_;

  /* State Updaters */

  void UpdateGlobalTrans(const dto::GlobalTrans &global_trans);

  void UpdateQuadModelTrans();

  void UpdateSceneModelTrans();
};
}  // namespace shader
