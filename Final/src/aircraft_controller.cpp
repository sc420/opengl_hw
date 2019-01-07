#include "aircraft_controller.hpp"

ctrl::AircraftController::AircraftController(
    const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &drift_dir,
    const float speed, const glm::vec3 &drift_dir_adjust_factor,
    const float speed_adjust_factor, const glm::vec3 &drift_dir_bounce_force,
    const float speed_bounce_force)
    : pos_(pos),
      dir_(dir),
      drift_dir_(drift_dir),
      speed_(speed),
      prefer_drift_dir_(drift_dir),
      prefer_speed_(speed),
      drift_dir_adjust_factor_(drift_dir_adjust_factor),
      speed_adjust_factor_(speed_adjust_factor),
      drift_dir_bounce_force_(drift_dir_bounce_force),
      speed_bounce_force_(speed_bounce_force) {}

glm::vec3 ctrl::AircraftController::GetPos() const { return pos_; }

glm::vec3 ctrl::AircraftController::GetDir() const { return dir_; }

glm::vec3 ctrl::AircraftController::GetDriftDir() const { return drift_dir_; }

float ctrl::AircraftController::GetSpeed() const { return speed_; }

void ctrl::AircraftController::AddDriftDir(const glm::vec3 &add_drift_dir) {
  drift_dir_ += drift_dir_adjust_factor_ * add_drift_dir;
}

void ctrl::AircraftController::AddSpeed(const float add_speed) {
  speed_ += speed_adjust_factor_ * add_speed;
}

void ctrl::AircraftController::Update() {
  // Update the direction
  dir_ = dir_ + drift_dir_;
  dir_ = glm::mod(dir_, 2.0f * glm::pi<float>());

  // Update the position
  pos_ += speed_ *
          glm::vec3(CalcRotMatrix(dir_) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));

  // Bounce back the values
  drift_dir_ += CalcDriftDirBounceForce() * (-CalcDriftDirDrift());
  speed_ += CalcSpeedBounceForce() * (-CalcSpeedDrift());
}

glm::vec3 ctrl::AircraftController::CalcDriftDirBounceForce() const {
  const glm::vec3 linear_force =
      drift_dir_bounce_force_ * glm::length(CalcDriftDirDrift());
  return glm::exp(linear_force) - 1.0f;
}

float ctrl::AircraftController::CalcSpeedBounceForce() const {
  const float linear_force =
      speed_bounce_force_ * glm::length(CalcSpeedDrift());
  return glm::exp(linear_force) - 1.0f;
}

glm::vec3 ctrl::AircraftController::CalcDriftDirDrift() const {
  return drift_dir_ - prefer_drift_dir_;
}

float ctrl::AircraftController::CalcSpeedDrift() const {
  return speed_ - prefer_speed_;
}

glm::mat4 ctrl::AircraftController::CalcRotMatrix(
    const glm::vec3 &rot_axes) const {
  // TODO: Merge with other code
  const glm::quat pitch =
      glm::angleAxis(rot_axes.x, glm::vec3(1.0f, 0.0f, 0.0f));
  const glm::quat yaw = glm::angleAxis(rot_axes.y, glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::quat roll =
      glm::angleAxis(rot_axes.z, glm::vec3(0.0f, 0.0f, 1.0f));
  const glm::quat orientation = glm::normalize(pitch * yaw * roll);
  return glm::mat4_cast(orientation);
}
