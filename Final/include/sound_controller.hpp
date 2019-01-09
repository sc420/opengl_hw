#pragma once

#include <irrklang/irrKlang.h>

#include "as/common.hpp"

namespace ctrl {
class SoundController {
 public:
  SoundController();

  ~SoundController();

  void Register2DSound(const std::string& sound_name, const std::string& path,
                       const bool play_looped = false,
                       const bool start_paused = false);

  void Register3DSound(const std::string& sound_name, const std::string& path,
                       const glm::vec3& sound_pos = glm::vec3(0.0f),
                       const bool play_looped = false,
                       const bool start_paused = false);

  void SetSoundVolume(const std::string& sound_name, const float volume);

  void SetSoundPaused(const std::string& sound_name, const bool paused);

  void Set3DListenerPosition(const glm::vec3& listener_pos,
                             const glm::vec3& listener_look_at);

  void Set3DSoundPosition(const std::string& sound_name,
                          const glm::vec3& sound_pos);

  bool IsSoundPaused(const std::string& sound_name);

  bool IsSoundFinished(const std::string& sound_name);

 private:
  irrklang::ISoundEngine* engine_;

  std::map<std::string, irrklang::ISound*> name_to_sound_;

  irrklang::ISound* GetSound(const std::string& sound_name) const;

  irrklang::vec3df GlmVecToIrrklangVec(const glm::vec3& v) const;
};
}  // namespace ctrl
