#include "fbx_camera_controller.hpp"

ctrl::FbxCameraController::FbxCameraController(
    const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scaling,
    const glm::vec3 &pos_adjust_factor, const glm::vec3 &rot_adjust_factor,
    const glm::vec3 &scaling_adjust_factor, const glm::vec3 &max_pos_change,
    const glm::vec3 &max_rot_change, const glm::vec3 &max_scaling_change,
    const glm::vec3 &pos_change_decay, const glm::vec3 &rot_change_decay,
    const glm::vec3 &scaling_change_decay, const glm::vec3 &pos_bounce_force,
    const glm::vec3 &rot_bounce_force, const glm::vec3 &scaling_bounce_force)
    : pos_(pos),
      rot_(rot),
      scaling_(scaling),
      pos_change_(glm::vec3(0.0f)),
      rot_change_(glm::vec3(0.0f)),
      scaling_change_(glm::vec3(0.0f)),
      prefer_pos_(pos),
      prefer_rot_(rot),
      prefer_scaling_(scaling),
      pos_adjust_factor_(pos_adjust_factor),
      rot_adjust_factor_(rot_adjust_factor),
      scaling_adjust_factor_(scaling_adjust_factor),
      max_pos_change_(max_pos_change),
      max_rot_change_(max_rot_change),
      max_scaling_change_(max_scaling_change),
      pos_change_decay_(pos_change_decay),
      rot_change_decay_(rot_change_decay),
      scaling_change_decay_(scaling_change_decay),
      pos_bounce_force_(pos_bounce_force),
      rot_bounce_force_(rot_bounce_force),
      scaling_bounce_force_(scaling_bounce_force) {}

glm::vec3 ctrl::FbxCameraController::GetPos() const { return pos_; }

glm::vec3 ctrl::FbxCameraController::GetRot() const { return rot_; }

glm::vec3 ctrl::FbxCameraController::GetScaling() const { return scaling_; }

glm::vec3 ctrl::FbxCameraController::GetRotDir() const {
  const glm::mat4 rot_matrix = CalcRotMatrix((-rot_));
  const glm::vec4 axis = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
  return rot_matrix * axis;
}

glm::vec3 ctrl::FbxCameraController::GetRotUp() const {
  const glm::mat4 rot_matrix = CalcRotMatrix((-rot_));
  const glm::vec4 axis = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
  return rot_matrix * axis;
}

void ctrl::FbxCameraController::SetPreferPos(const glm::vec3 &prefer_pos) {
  prefer_pos_ = prefer_pos;
}

void ctrl::FbxCameraController::SetPreferRot(const glm::vec3 &prefer_rot) {
  prefer_rot_ = prefer_rot;
}

void ctrl::FbxCameraController::SetPreferScaling(
    const glm::vec3 &prefer_scaling) {
  prefer_scaling_ = prefer_scaling;
}

void ctrl::FbxCameraController::AddPos(const glm::vec3 &add_pos) {
  pos_change_ += pos_adjust_factor_ * add_pos;
  pos_change_ = glm::min(pos_change_, max_pos_change_);
  pos_change_ = glm::max(pos_change_, (-max_pos_change_));
}

void ctrl::FbxCameraController::AddRot(const glm::vec3 &add_rot) {
  rot_change_ += rot_adjust_factor_ * add_rot;
  rot_change_ = glm::min(rot_change_, max_rot_change_);
  rot_change_ = glm::max(rot_change_, (-max_rot_change_));
}

void ctrl::FbxCameraController::AddScaling(const glm::vec3 &add_scaling) {
  scaling_change_ += scaling_adjust_factor_ * add_scaling;
  scaling_change_ = glm::min(scaling_change_, max_scaling_change_);
  scaling_change_ = glm::max(scaling_change_, (-max_scaling_change_));
}

void ctrl::FbxCameraController::Update() {
  // Update values
  pos_ += pos_change_;
  rot_ += rot_change_;
  scaling_ += scaling_change_;
  // Decay changes
  pos_change_ *= pos_change_decay_;
  rot_change_ *= rot_change_decay_;
  scaling_change_ *= scaling_change_decay_;
  // Bounce back the values
  pos_ += CalcPosBounceForce() * (-CalcPosDiff());
  rot_ += CalcRotBounceForce() * (-CalcRotDiff());
  scaling_ += CalcScalingBounceForce() * (-CalcScalingDiff());
}

glm::vec3 ctrl::FbxCameraController::CalcPosBounceForce() const {
  const glm::vec3 linear_force = pos_bounce_force_ * glm::length(CalcPosDiff());
  return glm::exp(linear_force) - 1.0f;
}

glm::vec3 ctrl::FbxCameraController::CalcRotBounceForce() const {
  const glm::vec3 linear_force = rot_bounce_force_ * glm::length(CalcRotDiff());
  return glm::exp(linear_force) - 1.0f;
}

glm::vec3 ctrl::FbxCameraController::CalcScalingBounceForce() const {
  const glm::vec3 linear_force =
      scaling_bounce_force_ * glm::length(CalcScalingDiff());
  return glm::exp(linear_force) - 1.0f;
}

glm::vec3 ctrl::FbxCameraController::CalcPosDiff() const {
  return pos_ - prefer_pos_;
}

glm::vec3 ctrl::FbxCameraController::CalcRotDiff() const {
  return rot_ - prefer_rot_;
}

glm::vec3 ctrl::FbxCameraController::CalcScalingDiff() const {
  return scaling_ - prefer_scaling_;
}

glm::mat4 ctrl::FbxCameraController::CalcRotMatrix(
    const glm::vec3 &angles) const {
  // TODO: Merge with other code
  const glm::quat quaternion = glm::quat(angles);
  return glm::mat4_cast(quaternion);
}
