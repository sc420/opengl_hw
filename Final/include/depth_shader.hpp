#pragma once

#include "as/trans/camera.hpp"

#include "scene_model_dto.hpp"
#include "scene_shader.hpp"
#include "shader.hpp"
#include "trans_dto.hpp"

namespace shader {
class SceneShader;

class DepthShader : public Shader {
 public:
  enum class DepthTextureTypes {
    kFromLight,
    kFromCamera,
  };

  /* Shader Registrations */

  void RegisterSceneShader(SceneShader &scene_shader);

  /* GL Initializations */

  void Init();

  void InitFramebuffers();

  void InitUniformBlocks();

  void InitDepthTexture(const DepthTextureTypes depth_tex_type);

  /* GL Drawing Methods */

  void DrawFromLight(const glm::ivec2 &window_size);

  void DrawFromCamera(const glm::ivec2 &window_size,
                      const dto::GlobalTrans &camera_trans);

  dto::GlobalTrans GetLightTrans() const;

  void UseDepthFramebuffer();

  void UseDepthTexture(const DepthTextureTypes depth_tex_type);

  /* Name Management */

  std::string GetId() const override;

  std::string GetDepthTextureName(const DepthTextureTypes depth_tex_type) const;

  std::string GetDepthTextureUnitName(
      const DepthTextureTypes depth_tex_type) const;

  std::string DepthTextureTypeToName(
      const DepthTextureTypes depth_tex_type) const;

 protected:
  /* Name Management */

  std::string GetDepthFramebufferName() const;

  std::string GetGlobalTransBufferName() const;

  std::string GetModelTransBufferName() const;

  std::string GetGlobalTransUniformBlockName() const;

  std::string GetModelTransUniformBlockName() const;

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

  void UpdateModelTrans(const dto::SceneModel &scene_model);

  /* GL Drawing Methods */

  void DrawModelWithoutTextures(const dto::SceneModel &scene_model);
};
}  // namespace shader
