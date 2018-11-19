#include "as/trans/camera.hpp"

as::CameraTrans::CameraTrans(const glm::vec3 &init_pos,
                             const glm::vec3 &init_angle)
    : init_pos_(init_pos),
      init_angle_(init_angle),
      eye_(init_pos),
      angle_(init_angle) {}

glm::mat4 as::CameraTrans::GetTrans() const {
  const glm::mat4 identity(1.0f);
  // Calculate rotation
  const glm::mat4 rotate = GetRotateMatrix();
  // Calculate translation
  const glm::mat4 translate = glm::translate(identity, -1.0f * eye_);
  // Return transformation matrix
  return rotate * translate;
}

void as::CameraTrans::AddEyeAxisAligned(const glm::vec3 &add_pos) {
  eye_ += add_pos;
}

void as::CameraTrans::AddEye(const glm::vec3 &add_dir) {
  const glm::mat4 rotate = GetRotateMatrix();
  const glm::vec4 rotated_add_dir =
      glm::transpose(rotate) * glm::vec4(add_dir, 1.0f);
  eye_ += glm::vec3(rotated_add_dir);
}

void as::CameraTrans::AddAngle(const glm::vec3 &add_angle) {
  angle_ += add_angle;
}

glm::vec3 as::CameraTrans::GetEye() const { return eye_; }

glm::vec3 as::CameraTrans::GetAngle() const { return angle_; }

void as::CameraTrans::SetInitTrans(const glm::vec3 &init_pos,
                                   const glm::vec3 &init_angle) {
  init_pos_ = init_pos;
  init_angle_ = init_angle;
}

void as::CameraTrans::ResetTrans() {
  eye_ = init_pos_;
  angle_ = init_angle_;
}

glm::mat4 as::CameraTrans::GetRotateMatrix() const {
  const glm::quat pitch = glm::angleAxis(angle_.x, glm::vec3(1.0f, 0.0f, 0.0f));
  const glm::quat yaw = glm::angleAxis(angle_.y, glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::quat roll = glm::angleAxis(angle_.z, glm::vec3(0.0f, 0.0f, 1.0f));
  const glm::quat orientation = glm::normalize(pitch * yaw * roll);
  return glm::mat4_cast(orientation);
}
