#pragma once

#include "as/common.hpp"

namespace as {
class CameraTrans {
 public:
  CameraTrans(const glm::vec3 &init_pos, const glm::vec3 &init_angle);

  glm::mat4 GetTrans() const;

  void AddEyeWorldSpace(const glm::vec3 &add_pos);

  void AddEye(const glm::vec3 &add_dir);

  void AddAngle(const glm::vec3 &add_angle);

  glm::vec3 GetEye() const;

  glm::vec3 GetAngle() const;

  void SetInitTrans(const glm::vec3 &init_pos, const glm::vec3 &init_angle);

  void ResetTrans();

 private:
  glm::vec3 init_pos_;
  glm::vec3 init_angle_;
  glm::vec3 eye_;
  // (x, y, z) = (pitch, yaw, roll)
  glm::vec3 angle_;

  glm::mat4 GetRotationMatrix() const;
};
}  // namespace as
