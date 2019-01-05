#pragma once

#include "as/common.hpp"

namespace as {
class CameraTrans {
 public:
  CameraTrans();

  CameraTrans(const glm::vec3 &init_pos, const glm::vec3 &init_angles);

  glm::mat4 GetTrans() const;

  void AddEyeWorldSpace(const glm::vec3 &add_pos);

  void AddEye(const glm::vec3 &add_dir);

  void AddAngles(const glm::vec3 &add_angles);

  void SetTrans(const glm::vec3 &pos, const glm::vec3 &angles);

  void SetEye(const glm::vec3 &pos);

  void SetAngles(const glm::vec3 &angles);

  glm::vec3 GetEye() const;

  glm::vec3 GetAngles() const;

  void SetInitTrans(const glm::vec3 &init_pos, const glm::vec3 &init_angles);

  void ResetTrans();

 private:
  glm::vec3 init_pos_;
  glm::vec3 init_angles_;
  glm::vec3 eye_;
  // (x, y, z) = (pitch, yaw, roll)
  glm::vec3 angles_;

  glm::mat4 GetRotationMatrix() const;
};
}  // namespace as
