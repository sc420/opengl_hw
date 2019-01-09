#include "sound_controller.hpp"

ctrl::SoundController::SoundController() {
  engine_ = irrklang::createIrrKlangDevice();
  if (!engine_) {
    throw std::runtime_error("Could not start up irrklang engine");
  }
}

ctrl::SoundController::~SoundController() { engine_->drop(); }

void ctrl::SoundController::Register2DSound(const std::string& sound_name,
                                            const std::string& path,
                                            const bool play_looped,
                                            const bool start_paused) {
  irrklang::ISound* sound =
      engine_->play2D(path.c_str(), play_looped, start_paused, true);
  name_to_sound_[sound_name] = sound;
}

void ctrl::SoundController::Register3DSound(const std::string& sound_name,
                                            const std::string& path,
                                            const glm::vec3& sound_pos,
                                            const bool play_looped,
                                            const bool start_paused) {
  irrklang::ISound* sound =
      engine_->play3D(path.c_str(), GlmVecToIrrklangVec(sound_pos), play_looped,
                      start_paused, true);
  name_to_sound_[sound_name] = sound;
}

void ctrl::SoundController::SetSoundVolume(const std::string& sound_name,
                                           const float volume) {
  irrklang::ISound* sound = GetSound(sound_name);
  sound->setVolume(volume);
}

void ctrl::SoundController::SetSoundPaused(const std::string& sound_name,
                                           const bool paused) {
  irrklang::ISound* sound = GetSound(sound_name);
  sound->setIsPaused(paused);
}

void ctrl::SoundController::Set3DListenerPosition(
    const glm::vec3& listener_pos, const glm::vec3& listener_look_at) {
  engine_->setListenerPosition(GlmVecToIrrklangVec(listener_pos),
                               GlmVecToIrrklangVec(listener_look_at));
}

void ctrl::SoundController::Set3DSoundPosition(const std::string& sound_name,
                                               const glm::vec3& sound_pos) {
  irrklang::ISound* sound = GetSound(sound_name);
  sound->setPosition(GlmVecToIrrklangVec(sound_pos));
}

bool ctrl::SoundController::IsSoundPaused(const std::string& sound_name) {
  irrklang::ISound* sound = GetSound(sound_name);
  return sound->getIsPaused();
}

bool ctrl::SoundController::IsSoundFinished(const std::string& sound_name) {
  irrklang::ISound* sound = GetSound(sound_name);
  return sound->isFinished();
}

irrklang::ISound* ctrl::SoundController::GetSound(
    const std::string& sound_name) const {
  if (name_to_sound_.count(sound_name) == 0) {
    throw std::runtime_error("Could not find sound name '" + sound_name + "'");
  } else {
    return name_to_sound_.at(sound_name);
  }
}

irrklang::vec3df ctrl::SoundController::GlmVecToIrrklangVec(
    const glm::vec3& v) const {
  return irrklang::vec3df(v.x, v.y, v.z);
}
