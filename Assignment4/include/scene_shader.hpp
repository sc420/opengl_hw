#pragma once

#include "shader.hpp"

namespace shader {
class SceneShader : public Shader {
 public:
  struct GlobalTrans {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

  struct ModelTrans {
    glm::mat4 trans;
  };

  struct Lighting {
    glm::vec4 light_color;
    glm::vec4 light_pos;
    glm::vec4 light_intensity;
    glm::vec4 view_pos;
  };

  SceneShader();

  /* Model handlers */

  void LoadModel();

  /* GL initialization methods */

  void Init() override;

  void InitVertexArrays();

  void InitUniformBlocks();

  void InitTextures();

  void InitLighting();

  /* GL drawing methods */

  void Draw() override;

  /* State updating methods */

  void UpdateGlobalTrans(const GlobalTrans &global_trans);

  void UpdateModelTrans(const ModelTrans &model_trans);

  void UpdateViewPos(const glm::vec3 &view_pos);

  /* Name management */

  std::string GetId() const override;

 protected:
  /* Model handlers */

  virtual as::Model &GetModel() override;

  /* GL initialization methods */

  GLsizei GetNumMipmapLevels() const;

  /* Name management */

  std::string GetGlobalTransBufferName() const;

  std::string GetModelTransBufferName() const;

  std::string GetLightingBufferName() const;

  std::string GetGlobalTransUniformBlockName() const;

  std::string GetModelTransUniformBlockName() const;

  std::string GetLightingUniformBlockName() const;

 private:
  as::Model scene_model_;

  GlobalTrans global_trans_;

  ModelTrans model_trans_;

  Lighting lighting_;
};
}  // namespace shader
