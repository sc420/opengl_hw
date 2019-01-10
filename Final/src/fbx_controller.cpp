#include "fbx_controller.hpp"

ctrl::FbxController::FbxController() { InitializeMemoryAllocator(); }

ctrl::FbxController::~FbxController() { delete gSceneContext; }

void ctrl::FbxController::Initialize(const std::string& fbx_path,
                                     const glm::ivec2& window_size) {
  const bool lSupportVBO = true;
  gSceneContext = new SceneContext(fbx_path.c_str(), window_size.x,
                                   window_size.y, lSupportVBO);

  Draw();
  ConfigSceneContext();
}

void ctrl::FbxController::Draw() { gSceneContext->OnDisplay(); }

void ctrl::FbxController::GetCameraTransform(glm::vec3& eye, glm::vec3& center,
                                             glm::vec3& up, float& roll) {
  fbxsdk::FbxDouble3 fbx_position;
  fbxsdk::FbxDouble3 fbx_interest_position;
  fbxsdk::FbxDouble3 fbx_up_vector;
  double fbx_roll;
  gSceneContext->GetCameraTransform(fbx_position, fbx_interest_position,
                                    fbx_up_vector, fbx_roll);
  eye = glm::vec3(fbx_position[0], fbx_position[1], fbx_position[2]);
  center = glm::vec3(fbx_interest_position[0], fbx_interest_position[1],
                     fbx_interest_position[2]);
  up = glm::vec3(fbx_up_vector[0], fbx_up_vector[1], fbx_up_vector[2]);
  roll = static_cast<float>(fbx_roll);
}

void ctrl::FbxController::GetModelTransform(glm::vec3& translation,
                                            glm::vec3& rotation,
                                            glm::vec3& scaling) {
  fbxsdk::FbxDouble3 fbx_translation;
  fbxsdk::FbxDouble3 fbx_rotation;
  fbxsdk::FbxDouble3 fbx_scaling;
  gSceneContext->GetModelTransform(fbx_translation, fbx_rotation, fbx_scaling);
  translation =
      glm::vec3(fbx_translation[0], fbx_translation[1], fbx_translation[2]);
  rotation = glm::vec3(fbx_rotation[0], fbx_rotation[1], fbx_rotation[2]);
  rotation = glm::radians(rotation);
  scaling = glm::vec3(fbx_scaling[0], fbx_scaling[1], fbx_scaling[2]);
}

void ctrl::FbxController::SetTime(const double ratio) {
  // Ask to display the current frame only if necessary.
  if (gSceneContext->GetStatus() == SceneContext::MUST_BE_REFRESHED) {
    glutPostRedisplay();
  }

  gSceneContext->SetTime(ratio);
}

void ctrl::FbxController::SetCameraTransform(const glm::vec3& eye,
                                             const glm::vec3& angles,
                                             const glm::vec3& up,
                                             const float& roll) {
  const glm::quat quaternion = glm::quat(angles);
  const glm::vec4 dir =
      glm::mat4_cast(quaternion) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

  const glm::vec3 center = eye + glm::vec3(dir);
  fbxsdk::FbxDouble3 fbx_position(eye[0], eye[1], eye[2]);
  fbxsdk::FbxDouble3 fbx_interest_position(center[0], center[1], center[2]);
  fbxsdk::FbxDouble3 fbx_up_vector(up[0], up[1], up[2]);
  const double fbx_roll = roll;

  gSceneContext->SetCameraTransform(fbx_position, fbx_interest_position,
                                    fbx_up_vector, fbx_roll);
}

void ctrl::FbxController::SetModelTransform(const glm::vec3& translation,
                                            const glm::vec3& rotation,
                                            const glm::vec3& scaling) {
  const glm::vec3 rotation_degress = glm::degrees(rotation);
  fbxsdk::FbxDouble3 fbx_translation(translation[0], translation[1],
                                     translation[2]);
  // The order is around XYZ axes
  fbxsdk::FbxDouble3 fbx_rotation(rotation_degress[0], rotation_degress[1],
                                  rotation_degress[2]);
  fbxsdk::FbxDouble3 fbx_scaling(scaling[0], scaling[1], scaling[2]);
  gSceneContext->SetModelTransform(fbx_translation, fbx_rotation, fbx_scaling);
}

void ctrl::FbxController::OnReshape(const int width, const int height) {
  gSceneContext->OnReshape(width, height);
  gSceneContext->SetCameraAspect(
      fbxsdk::FbxCamera::EAspectRatioMode::eWindowSize, width, height);
}

void ctrl::FbxController::OnKeyboard(const unsigned char key) {
  gSceneContext->OnKeyboard(key);
}

void ctrl::FbxController::OnMouse(const int button, const int state,
                                  const int x, const int y) {
  gSceneContext->OnMouse(button, state, x, y);
}

void ctrl::FbxController::OnMotion(const int x, const int y) {
  gSceneContext->OnMouseMotion(x, y);
}

bool ctrl::FbxController::init_mem_allocator_ = false;

void ctrl::FbxController::InitializeMemoryAllocator() {
  // Check whether the memory allocator has been used
  if (init_mem_allocator_) {
    return;
  }

  // Use a custom memory allocator
  FbxSetMallocHandler(MyMemoryAllocator::MyMalloc);
  FbxSetReallocHandler(MyMemoryAllocator::MyRealloc);
  FbxSetFreeHandler(MyMemoryAllocator::MyFree);
  FbxSetCallocHandler(MyMemoryAllocator::MyCalloc);

  init_mem_allocator_ = true;
}

void ctrl::FbxController::ConfigSceneContext() {
  // Import the scene if it's ready to load.
  if (gSceneContext->GetStatus() != SceneContext::MUST_BE_LOADED) {
    return;
  }

  // This function is only called in the first display callback
  // to make sure that the application window is opened and a
  // status message is displayed before.
  gSceneContext->LoadFile();

  // Set the current animation stack and set the start, stop and current
  // time.
  gSceneContext->SetCurrentAnimStack(0);
  // Set the current pose
  gSceneContext->SetCurrentPoseIndex(0);
  // Set the pesepective camera
  gSceneContext->SetCurrentCamera(FBXSDK_CAMERA_PERSPECTIVE);
  // Set the zoom mode
  gSceneContext->SetZoomMode(SceneContext::ZOOM_POSITION);
}
