#include "fbx_controller.hpp"

ctrl::FbxController::FbxController() { InitializeMemoryAllocator(); }

ctrl::FbxController::~FbxController() { delete gSceneContext; }

void ctrl::FbxController::Initialize(const std::string& fbx_path,
                                     const glm::ivec2& window_size) {
  const bool lSupportVBO = true;
  gSceneContext = new SceneContext(
      "assets/models/blackhawk/blackhawk-helicopter-animation.fbx",
      window_size.x, window_size.y, lSupportVBO);

  Draw();
  ConfigSceneContext();
}

void ctrl::FbxController::Draw() { gSceneContext->OnDisplay(); }

void ctrl::FbxController::GetCameraTransform(glm::vec3& position,
                                             glm::vec3& up_vector,
                                             float& roll) {
  fbxsdk::FbxDouble3 fbx_position;
  fbxsdk::FbxDouble3 fbx_up_vector;
  double fbx_roll;
  gSceneContext->GetCameraTransform(fbx_position, fbx_up_vector, fbx_roll);
  position = glm::vec3(fbx_position[0], fbx_position[1], fbx_position[2]);
  up_vector = glm::vec3(fbx_up_vector[0], fbx_up_vector[1], fbx_up_vector[2]);
  roll = static_cast<float>(fbx_roll);
}

void ctrl::FbxController::SetTime(const double ratio) {
  // Ask to display the current frame only if necessary.
  if (gSceneContext->GetStatus() == SceneContext::MUST_BE_REFRESHED) {
    glutPostRedisplay();
  }

  gSceneContext->SetTime(ratio);
}

void ctrl::FbxController::SetModelTransform(const glm::vec3& position,
                                            const glm::vec3& up_vector,
                                            const float& roll) {
  // TODO: Try using  glm::value_ptr()
  const glm::vec3 reverse_position = (-position);
  fbxsdk::FbxDouble3 fbx_position(reverse_position[0], reverse_position[1],
                                  reverse_position[2]);
  fbxsdk::FbxDouble3 fbx_up_vector(up_vector[0], up_vector[1], up_vector[2]);
  const double fbx_roll = roll;
  gSceneContext->SetCameraTransform(fbx_position, fbx_up_vector, fbx_roll);
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
