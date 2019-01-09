#include "aircraft_controller.hpp"

ctrl::AircraftController::AircraftController(
    const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &drift_dir,
    const float speed, const glm::vec3 &drift_dir_adjust_factor,
    const float speed_adjust_factor, const glm::vec3 &max_drift_dir_change,
    const float max_speed_change, const glm::vec3 &drift_dir_change_decay,
    const float speed_change_decay, const glm::vec3 &drift_dir_bounce_force,
    const float speed_bounce_force)
    : pos_(pos),
      dir_(dir),
      drift_dir_(drift_dir),
      speed_(speed),
      drift_dir_change_(glm::vec3(0.0f)),
      speed_change_(0.0f),
      prefer_drift_dir_(drift_dir),
      prefer_speed_(speed),
      drift_dir_adjust_factor_(drift_dir_adjust_factor),
      speed_adjust_factor_(speed_adjust_factor),
      max_drift_dir_change_(max_drift_dir_change),
      max_speed_change_(max_speed_change),
      drift_dir_change_decay_(drift_dir_change_decay),
      speed_change_decay_(speed_change_decay),
      drift_dir_bounce_force_(drift_dir_bounce_force),
      speed_bounce_force_(speed_bounce_force),
      drift_dir_wind_(glm::vec3(0.0f)),
      speed_wind_(0.0f),
      drift_dir_wind_adjust_factor_(glm::vec3(0.0f)),
      speed_wind_adjust_factor_(0.0f),
      max_drift_dir_wind_(glm::vec3(0.0f)),
      max_speed_wind_(0.0f) {
  rand_engine_ = std::mt19937(rand_device_());
  distrib_ = std::uniform_real_distribution<float>(-1.0f, 1.0f);
}

glm::vec3 ctrl::AircraftController::GetPos() const { return pos_; }

glm::vec3 ctrl::AircraftController::GetDir() const { return dir_; }

glm::vec3 ctrl::AircraftController::GetDriftDir() const { return drift_dir_; }

float ctrl::AircraftController::GetSpeed() const { return speed_; }

void ctrl::AircraftController::SetWind(
    const glm::vec3 &drift_dir_wind, const float speed_wind,
    const glm::vec3 &drift_dir_wind_adjust_factor,
    const float speed_wind_adjust_factor, const glm::vec3 &max_drift_dir_wind,
    const float max_speed_wind) {
  drift_dir_wind_ = drift_dir_wind;
  speed_wind_ = speed_wind;
  drift_dir_wind_adjust_factor_ = drift_dir_wind_adjust_factor;
  speed_wind_adjust_factor_ = speed_wind_adjust_factor;
  max_drift_dir_wind_ = max_drift_dir_wind;
  max_speed_wind_ = max_speed_wind;
}

void ctrl::AircraftController::AddDriftDir(const glm::vec3 &add_drift_dir) {
  drift_dir_change_ += drift_dir_adjust_factor_ * add_drift_dir;
  drift_dir_change_ = glm::min(drift_dir_change_, max_drift_dir_change_);
  drift_dir_change_ = glm::max(drift_dir_change_, (-max_drift_dir_change_));
}

void ctrl::AircraftController::AddSpeed(const float add_speed) {
  speed_change_ += speed_adjust_factor_ * add_speed;
  speed_change_ = glm::min(speed_change_, max_speed_change_);
  speed_change_ = glm::max(speed_change_, (-max_speed_change_));
}

void ctrl::AircraftController::Update() {
  // Update wind
  drift_dir_wind_ += GenRand() * drift_dir_wind_adjust_factor_;
  drift_dir_wind_ =
      glm::clamp(drift_dir_wind_, (-max_drift_dir_wind_), max_drift_dir_wind_);
  speed_wind_ += GenRand().x * speed_wind_adjust_factor_;
  speed_wind_ = glm::clamp(speed_wind_, (-max_speed_wind_), max_speed_wind_);

  // Add wind to the changes
  drift_dir_change_ += drift_dir_wind_;
  speed_change_ += speed_wind_;

  // Update values
  drift_dir_ += drift_dir_change_;
  speed_ += speed_change_;

  // Update the direction
  dir_ = glm::mod(dir_ + drift_dir_, 2.0f * glm::pi<float>());

  // Update the position
  const glm::vec4 forward_dir = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
  const glm::vec3 drift_pos = glm::vec3(CalcRotMatrix(dir_) * forward_dir);
  pos_ += speed_ * drift_pos;

  // Decay changes
  drift_dir_change_ *= drift_dir_change_decay_;
  speed_change_ *= speed_change_decay_;

  // Bounce back the values
  drift_dir_ += CalcDriftDirBounceForce() * (-CalcDriftDirDiff());
  speed_ += CalcSpeedBounceForce() * (-CalcSpeedDiff());
}

glm::vec3 ctrl::AircraftController::CalcDriftDirBounceForce() const {
  const glm::vec3 linear_force =
      drift_dir_bounce_force_ * glm::length(CalcDriftDirDiff());
  return glm::exp(linear_force) - 1.0f;
}

float ctrl::AircraftController::CalcSpeedBounceForce() const {
  const float linear_force = speed_bounce_force_ * glm::length(CalcSpeedDiff());
  return glm::exp(linear_force) - 1.0f;
}

glm::vec3 ctrl::AircraftController::CalcDriftDirDiff() const {
  return drift_dir_ - prefer_drift_dir_;
}

float ctrl::AircraftController::CalcSpeedDiff() const {
  return speed_ - prefer_speed_;
}

glm::mat4 ctrl::AircraftController::CalcRotMatrix(
    const glm::vec3 &angles) const {
  // TODO: Merge with other code
  const glm::quat quaternion = glm::quat(angles);
  return glm::mat4_cast(quaternion);
}

glm::vec3 ctrl::AircraftController::GenRand() {
  // TODO: Merge with other code
  return glm::vec3(distrib_(rand_engine_), distrib_(rand_engine_),
                   distrib_(rand_engine_));
}
