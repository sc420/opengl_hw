#include "as/trans/camera.hpp"

as::CameraTrans::CameraTrans()
    : CameraTrans(glm::vec3(0.0f), glm::vec3(0.0f)) {}

as::CameraTrans::CameraTrans(const glm::vec3 &init_pos,
                             const glm::vec3 &init_angles)
    : init_pos_(init_pos),
      init_angles_(init_angles),
      eye_(init_pos),
      angles_(init_angles) {}

glm::mat4 as::CameraTrans::GetTrans() const {
  const glm::mat4 identity(1.0f);
  // Calculate translation
  const glm::mat4 translate = glm::translate(identity, -1.0f * eye_);
  // Calculate rotation
  const glm::mat4 rotate = GetRotationMatrix();
  // Return transformation matrix
  return rotate * translate;
}

void as::CameraTrans::AddEyeWorldSpace(const glm::vec3 &add_pos) {
  eye_ += add_pos;
}

void as::CameraTrans::AddEye(const glm::vec3 &add_dir) {
  const glm::mat4 rotate = GetRotationMatrix();
  const glm::vec4 rotated_add_dir =
      glm::transpose(rotate) * glm::vec4(add_dir, 1.0f);
  eye_ += glm::vec3(rotated_add_dir);
}

void as::CameraTrans::AddAngles(const glm::vec3 &add_angles) {
  angles_ += add_angles;
  // Keep the range in [0, 2*pi)
  const float two_pi = 2.0f * glm::pi<float>();
  angles_ = glm::mod(angles_, two_pi);
}

void as::CameraTrans::SetTrans(const glm::vec3 &pos, const glm::vec3 &angles) {
  eye_ = pos;
  angles_ = angles_;
}

void as::CameraTrans::SetEye(const glm::vec3 &pos) { eye_ = pos; }

void as::CameraTrans::SetAngles(const glm::vec3 &angles) { angles_ = angles; }

glm::vec3 as::CameraTrans::GetEye() const { return eye_; }

glm::vec3 as::CameraTrans::GetAngles() const { return angles_; }

void as::CameraTrans::SetInitTrans(const glm::vec3 &init_pos,
                                   const glm::vec3 &init_angles) {
  init_pos_ = init_pos;
  init_angles_ = init_angles;
}

void as::CameraTrans::ResetTrans() {
  eye_ = init_pos_;
  angles_ = init_angles_;
}

glm::mat4 as::CameraTrans::GetRotationMatrix() const {
  const glm::quat pitch =
      glm::angleAxis(angles_.x, glm::vec3(1.0f, 0.0f, 0.0f));
  const glm::quat yaw = glm::angleAxis(angles_.y, glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::quat roll = glm::angleAxis(angles_.z, glm::vec3(0.0f, 0.0f, 1.0f));
  const glm::quat orientation = glm::normalize(pitch * yaw * roll);
  return glm::mat4_cast(orientation);
}
