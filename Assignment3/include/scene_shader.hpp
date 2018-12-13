#pragma once

#include "as/gl/shader_app.hpp"

namespace shader {
class SceneShader : public app::ShaderApp {
 public:
  struct GlobalMvp {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

  struct ModelTrans {
    glm::mat4 trans;
  };

  /* GL initialization methods */

  void Init() override;

  void InitUniformBlocks();

  /* GL drawing methods */

  void Draw() override;

  /* State updating methods */

  void UpdateGlobalMvp(const GlobalMvp &global_mvp);

  void UpdateModelTrans(const ModelTrans &model_trans);

  /* Name management */

  std::string GetId() const override;

 protected:
  std::string GetGlobalMvpBufferName() const;

  std::string GetModelTransBufferName() const;

  std::string GetGlobalMvpUniformBlockName() const;

  std::string GetModelTransUniformBlockName() const;

 private:
  GlobalMvp global_mvp_;

  ModelTrans model_trans_;
};
}  // namespace shader
