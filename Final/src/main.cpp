#include "imgui/imgui.h"
#include "imgui/imgui_impl_freeglut.h"
#include "imgui/imgui_impl_opengl3.h"

#include "as/common.hpp"
#include "as/gl/gl_tools.hpp"
#include "as/trans/camera.hpp"

#include "aircraft_controller.hpp"
#include "depth_shader.hpp"
#include "diff_shader.hpp"
#include "fbx_camera_controller.hpp"
#include "fbx_controller.hpp"
#include "postproc_shader.hpp"
#include "scene_shader.hpp"
#include "skybox_shader.hpp"
#include "sound_controller.hpp"

/*******************************************************************************
 * Constants
 ******************************************************************************/

/* FBX */
static const auto kBlackHawkAnimDuration = 0.1f;
static const auto kBlackHawkExplosionAnimDuration = 5.0f;
/* User Interfaces */
static const auto kInitWindowRelativeCenterPos = glm::vec2(0.5f, 0.5f);
static const auto kInitWindowSize = glm::ivec2(720, 450);
static const auto kMinWindowSize = glm::ivec2(720, 450);
static const auto kCameraMovingStep = 0.05f;
static const auto kCameraRotationSensitivity = 0.003f;
static const auto kCameraZoomingStep = 1.0f;
/* Timers */
static const auto kTimerInterval = 10;
/* Camera Shaking */
static const auto kCameraShakingMaxWind = 0.1f;
/* Collision */
static const auto kCollisionDistUpdateInterval = 0.5f;
static const auto kCollisionDist = 0.3f;
static const auto kCollisionWarningDist = 1.0f;
static const auto kCollisionShakingDurationRatio = 0.5f;

/* Debug */
// Shadow
static const auto kSeeFromLight = false;
// Camera
static const auto kUpdateCameraFromAircraftController = true;
// Model editing
static const auto kEditingModelName = "tower";
static const auto kEditingModelScalingStep = 0.01f;
static const auto kEditingModelRotationStep = 0.01f;
static const auto kEditingModelTranslationStep = 0.1f;

/*******************************************************************************
 * Debugging
 ******************************************************************************/

// Model editing
int editing_model_instance_idx = 0;

/*******************************************************************************
 * Camera States
 ******************************************************************************/

// Camera transformations
as::CameraTrans camera_trans(glm::vec3(10.0f, 5.0f, 0.0f),
                             glm::vec3(glm::radians(30.0f), glm::radians(0.0f),
                                       glm::radians(0.0f)));

/*******************************************************************************
 * GL Managers
 ******************************************************************************/

as::GLManagers gl_managers;
as::UiManager ui_manager;

/*******************************************************************************
 * Shaders
 ******************************************************************************/

shader::DepthShader depth_shader;
shader::DiffShader diff_shader;
shader::PostprocShader postproc_shader;
shader::SceneShader scene_shader;
shader::SkyboxShader skybox_shader;

/*******************************************************************************
 * Controllers
 ******************************************************************************/

// Only for the black hawk
const glm::vec3 kInitAircraftPos = glm::vec3(10.0f, 5.0f, 0.0f);

ctrl::FbxCameraController fbx_camera_ctrl(
    // Position, Rotation, Scaling
    1e-2f * glm::vec3(0.0f, -400.0f, -2300.0f),
    glm::vec3(glm::radians(-10.0f), 0.0f, 0.0f), glm::vec3(1.0f),
    // Adjust factor
    1e-2f * glm::vec3(10.0f, 1.0f, 10.0f), glm::vec3(1e-3f), glm::vec3(0.0f),
    // Max change
    1e-2f * glm::vec3(10.0f), glm::vec3(1.0f), glm::vec3(0.0f),
    // Change decay
    glm::vec3(0.9f), glm::vec3(0.9f), glm::vec3(0.0f),
    // Bounce force
    glm::vec3(1e-3f, 1e-3f, 1e-2f), glm::vec3(1e-1f, 1e-2f, 1e-1f),
    glm::vec3(0.0f));
ctrl::AircraftController aircraft_ctrl(
    // Position, Direction, Drift direction, Speed
    kInitAircraftPos, glm::vec3(0.0f), glm::vec3(0.0f), 1e-2f,
    // Adjust factor
    glm::vec3(1e-4f), 1e-4f,
    // Max change
    glm::vec3(1e-2f), 1e-2f,
    // Change decay
    glm::vec3(0.9f), 0.9f,
    // Bounce force
    glm::vec3(1e1f), 1.0f);
ctrl::FbxController fbx_ctrl;
ctrl::FbxController explosion_fbx_ctrl;
ctrl::SoundController sound_ctrl;

/*******************************************************************************
 * Random Number Generators
 ******************************************************************************/

// Camera shaking
std::random_device rand_device;
std::mt19937 rand_engine(rand_device());
std::uniform_real_distribution<float> camera_shaking_distrib(-1e-2f, 1e-2f);
float camera_shaking_wind = 0.0f;

/*******************************************************************************
 * Collision States
 ******************************************************************************/

float cur_collision_dist = std::numeric_limits<float>::max();
float collision_dist_update_elapsed_time = 0.0f;
bool has_collided = false;
float collision_anim_elapsed_time = 0.0f;
bool has_collision_anim_finished = false;

/*******************************************************************************
 * Rendering States
 ******************************************************************************/

bool use_gui = false;
bool use_fbx = false;
bool use_aircraft_ctrl = false;
bool use_normal = false;
bool use_instantiating = false;
bool use_surrounding = false;
bool use_fog = false;
bool mix_fog_with_skybox = false;
bool use_sound = false;
bool last_use_sound = false;
bool use_aircraft_wind = false;
bool use_camera_wind = false;
bool use_hdr = false;
bool use_gamma_correct = false;
bool render_wireframe = false;

/*******************************************************************************
 * User Interface States
 ******************************************************************************/

// Modes
enum class Modes { comparison, navigation };

// Current mode
Modes cur_mode = Modes::navigation;

/*******************************************************************************
 * Timers
 ******************************************************************************/

float last_elapsed_time = 0.0f;

/*******************************************************************************
 * Menus
 ******************************************************************************/

enum MainMenuItems { kMainMidLevelSep, kMainExit };
enum GuiMenuItems { kGuiOn, kGuiOff };
enum TimerMenuItems { kTimerStart, kTimerStop };

/*******************************************************************************
 * GL Initializations
 ******************************************************************************/

void InitGLUT(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
  as::SetGLWindowInitRelativeCenterPos(kInitWindowRelativeCenterPos,
                                       kInitWindowSize);
  as::SetGLWindowInitSize(kInitWindowSize);
  glutCreateWindow("Final");
}

/*******************************************************************************
 * GL Context Configuration
 ******************************************************************************/

void ConfigGLSettings() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClearDepth(1.0f);
  glClearStencil(0);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE_ARB);
  // glEnable(GL_CULL_FACE);
  // glEnable(GL_BLEND);

  // Set blending function
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Enable stencil test
  glEnable(GL_STENCIL_TEST);
  // Set default stencil action
  glStencilFunc(GL_EQUAL, 0, 0xFF);
  // Set stencil test actions
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void InitUiManager() {
  // Get the UI manager
  ui_manager = gl_managers.GetUiManager();
  // Set initial window size
  ui_manager.SaveWindowSize(kInitWindowSize);
  // Start the clock
  ui_manager.StartClock();
}

void InitShaders() {
  // Register managers
  depth_shader.RegisterGLManagers(gl_managers);
  diff_shader.RegisterGLManagers(gl_managers);
  postproc_shader.RegisterGLManagers(gl_managers);
  scene_shader.RegisterGLManagers(gl_managers);
  skybox_shader.RegisterGLManagers(gl_managers);
  // Register shaders
  depth_shader.RegisterSceneShader(scene_shader);
  scene_shader.RegisterDepthShader(depth_shader);
  scene_shader.RegisterSkyboxShader(skybox_shader);
  skybox_shader.RegisterSceneShader(scene_shader);
  // Initialize shaders
  depth_shader.Init();
  diff_shader.Init();
  postproc_shader.Init();
  scene_shader.Init();
  skybox_shader.Init();
  // Reuse skybox texture
  scene_shader.ReuseSkyboxTexture();
  // Bind textures
  scene_shader.BindTextures();
}

void ConfigGL() {
  as::EnableCatchingGLError();
  ConfigGLSettings();
  InitUiManager();
  InitShaders();
}

/*******************************************************************************
 * Controller Initializations
 ******************************************************************************/

void StartInitialSound() {
  if (use_sound) {
    sound_ctrl.Register3DSound("helicopter_hovering",
                               "assets/sound/helicopter-hovering-01.wav",
                               glm::vec3(0.0f, 0.0f, 1.0f), true);
    sound_ctrl.Register2DSound("cockpit_interior",
                               "assets/sound/cockpit_interior.mp3", true);
    sound_ctrl.SetSoundVolume("cockpit_interior", 0.5f);
  }
}

/*******************************************************************************
 * Controller Updaters
 ******************************************************************************/

void UpdateFbxCameraController() {
  if (use_aircraft_wind) {
    fbx_camera_ctrl.SetWind(
        // Position wind, Rotation wind
        glm::vec3(0.0f), glm::vec3(0.0f),
        // Adjust factors
        glm::vec3(1e-2f), glm::vec3(1e-4f),
        // Max wind
        glm::vec3(1e-3f), glm::vec3(1e-5f));
  } else {
    fbx_camera_ctrl.SetWind(
        // Position wind, Rotation wind
        glm::vec3(0.0f), glm::vec3(0.0f),
        // Adjust factors
        glm::vec3(0.0f), glm::vec3(0.0f),
        // Max wind
        glm::vec3(0.0f), glm::vec3(0.0f));
  }
}

void UpdateAircraftController() {
  if (use_aircraft_wind) {
    aircraft_ctrl.SetWind(
        // Drift direction wind, speed wind
        glm::vec3(0.0f), 0.0f,
        // Adjust factors
        glm::vec3(0.0f, 1e-8f, 0.0f), 1e-7f,
        // Max wind
        glm::vec3(1e-6f), 1e-5f);
  } else {
    aircraft_ctrl.SetWind(
        // Drift direction wind, speed wind
        glm::vec3(0.0f), 0.0f,
        // Adjust factors
        glm::vec3(0.0f), 0.0f,
        // Max wind
        glm::vec3(0.0f), 0.0f);
  }
}

/*******************************************************************************
 * FBX Handlers
 ******************************************************************************/

void InitFbx() {
  fbx_ctrl.Initialize(
      "assets/models/blackhawk/blackhawk-helicopter-animation.fbx",
      kInitWindowSize);
  explosion_fbx_ctrl.Initialize(
      "assets/models/blackhawk_explosion/blackhawk-helicopter-explosion.fbx",
      kInitWindowSize);
}

/*******************************************************************************
 * Collision Handlers
 ******************************************************************************/

float GetCollisionDist() {
  return scene_shader.GetMinDistanceToModel(aircraft_ctrl.GetPos(), "ground");
}

void StartCollision() {
  if (!has_collided) {
    has_collision_anim_finished = false;
    collision_anim_elapsed_time = 0.0f;
    has_collided = true;

    // Stop the aircraft from flying
    aircraft_ctrl.SetPreferSpeed(0.0f);

    if (use_sound) {
      // Stop all previously played sound
      sound_ctrl.SetSoundStop("helicopter_hovering");
      sound_ctrl.SetSoundStop("cockpit_interior");
      sound_ctrl.SetSoundStop("pull_up_alarm");
      sound_ctrl.SetSoundStop("altitude_warning");

      // Play the explosion sound
      sound_ctrl.Register2DSound("big_explosion",
                                 "assets/sound/big_explosion.wav");

      // Play the sad sound
      sound_ctrl.Register2DSound("sad_violin", "assets/sound/sad_violin.mp3");
      sound_ctrl.SetSoundVolume("sad_violin", 0.0f);
    }
  }
}

void ResetCollision() {
  has_collided = false;
  collision_anim_elapsed_time = 0.0f;
  has_collision_anim_finished = false;

  // Restore flying state
  aircraft_ctrl.SetPos(kInitAircraftPos);
  aircraft_ctrl.SetDir(glm::vec3(0.0f));
  aircraft_ctrl.SetPreferSpeed(1e-2f);

  if (use_sound) {
    // Stop previously played sound
    sound_ctrl.StopAllSound();
  }

  // Resume initial sound
  StartInitialSound();
}

/*******************************************************************************
 * GUI Handlers
 ******************************************************************************/

void InitImGui() {
  // Setup Dear ImGui context
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  // Enable Keyboard/Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // Setup Platform/Renderer bindings
  ImGui_ImplFreeGLUT_Init();
  // NOTE: GLUT can only have one binding function, we don't bind it again
  // ImGui_ImplFreeGLUT_InstallFuncs();
  ImGui_ImplOpenGL3_Init();
}

void DestoryImGui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplFreeGLUT_Shutdown();
  ImGui::DestroyContext();
}

void UpdateImGui() {
  // Get ImGui IO
  const ImGuiIO &io = ImGui::GetIO();

  // Get camera states
  const glm::vec3 camera_pos = camera_trans.GetEye();
  const glm::vec3 camera_angles = glm::degrees(camera_trans.GetAngles());

  static bool has_opened = false;

  /* Debug Widget */

  {
    ImGui::Begin("Debug");

    if (!has_opened) ImGui::SetNextTreeNodeOpen(true);
    if (ImGui::CollapsingHeader("Performance")) {
      ImGui::Text("FPS: %.1f", io.Framerate);
    }

    if (!has_opened) ImGui::SetNextTreeNodeOpen(true);
    if (ImGui::CollapsingHeader("Camera")) {
      ImGui::Text("Position: (%.1f, %.1f, %.1f)", camera_pos.x, camera_pos.y,
                  camera_pos.z);
      ImGui::Text("Angles (Degree): (%.2f, %.2f, %.2f)", camera_angles.x,
                  camera_angles.y, camera_angles.z);
    }

    if (!has_opened) ImGui::SetNextTreeNodeOpen(true);
    if (ImGui::CollapsingHeader("FBX Model")) {
      glm::vec3 translation;
      glm::vec3 rotation;
      glm::vec3 scaling;

      fbx_ctrl.GetModelTransform(translation, rotation, scaling);

      ImGui::Text("Translation: (%.1f, %.1f, %.1f)", translation[0],
                  translation[1], translation[2]);
      ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", rotation[0], rotation[1],
                  rotation[2]);
      ImGui::Text("Scaling: (%.3f, %.3f, %.3f)", scaling[0], scaling[1],
                  scaling[2]);
    }

    if (!has_opened) ImGui::SetNextTreeNodeOpen(true);
    ImGui::SetNextTreeNodeOpen(true);
    if (ImGui::CollapsingHeader("Aircraft Controller")) {
      const glm::vec3 pos = aircraft_ctrl.GetPos();
      const glm::vec3 dir = aircraft_ctrl.GetDir();
      const glm::vec3 drift_dir = aircraft_ctrl.GetDriftDir();
      const float speed = aircraft_ctrl.GetSpeed();

      ImGui::Text("Pos: (%.1f, %.1f, %.1f)", pos[0], pos[1], pos[2]);
      ImGui::Text("Dir: (%.2f, %.2f, %.2f)", dir[0], dir[1], dir[2]);
      ImGui::Text("Drift Dir: (%.2f, %.2f, %.2f)", drift_dir[0], drift_dir[1],
                  drift_dir[2]);
      ImGui::Text("Speed: %.3f", speed);
    }

    if (!has_opened) ImGui::SetNextTreeNodeOpen(true);
    if (ImGui::CollapsingHeader("Collision")) {
      ImGui::Text("Distance: %.1f", cur_collision_dist);
    }

    if (!has_opened) ImGui::SetNextTreeNodeOpen(true);
    ImGui::SetNextTreeNodeOpen(true);
    if (ImGui::CollapsingHeader("Model Editing")) {
      const dto::SceneModel &scene_model =
          scene_shader.GetSceneModel(kEditingModelName);
      const glm::vec3 trans =
          scene_model.GetInstancingTranslation(editing_model_instance_idx);
      const glm::vec3 rot =
          scene_model.GetInstancingRotation(editing_model_instance_idx);
      const glm::vec3 scaling =
          scene_model.GetInstancingScaling(editing_model_instance_idx);

      ImGui::Text("Model: %s[%d]", kEditingModelName,
                  editing_model_instance_idx);
      ImGui::Text("Trans: (%.2f, %.2f, %.2f)", trans[0], trans[1], trans[2]);
      ImGui::Text("Rotation: (%.3f, %.3f, %.3f)", rot[0], rot[1], rot[2]);
      ImGui::Text("Scaling: (%.5f, %.5f, %.5f)", scaling[0], scaling[1],
                  scaling[2]);
    }

    ImGui::End();
  }

  /* State Control Widget */

  {
    ImGui::Begin("State Control");

    if (!has_opened) ImGui::SetNextTreeNodeOpen(true);
    if (ImGui::CollapsingHeader("Demo")) {
      ImGui::Checkbox("FBX", &use_fbx);
      ImGui::Checkbox("Flying", &use_aircraft_ctrl);
      ImGui::Checkbox("Normal", &use_normal);
      ImGui::Checkbox("Instantiating", &use_instantiating);
      ImGui::Checkbox("Surrounding", &use_surrounding);
      ImGui::Checkbox("Fog", &use_fog);
      ImGui::Checkbox("Mix Fog with Skybox", &mix_fog_with_skybox);
      ImGui::Checkbox("Sound", &use_sound);
      ImGui::Checkbox("Camera Wind", &use_camera_wind);
      ImGui::Checkbox("Aircraft Wind", &use_aircraft_wind);
      ImGui::Checkbox("HDR", &use_hdr);
      ImGui::Checkbox("Gamma Correction", &use_gamma_correct);
    }

    if (!has_opened) ImGui::SetNextTreeNodeOpen(true);
    if (ImGui::CollapsingHeader("Debug")) {
      ImGui::Checkbox("Wireframe", &render_wireframe);
    }

    // Update controllers
    UpdateFbxCameraController();
    UpdateAircraftController();

    ImGui::End();
  }

  has_opened = true;
}

void DrawImGui() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplFreeGLUT_NewFrame();

  // Update ImGui
  UpdateImGui();

  // Draw ImGui on default framebuffer
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/*******************************************************************************
 * Random Number Generators
 ******************************************************************************/

glm::vec3 GenRand(std::uniform_real_distribution<float> &distrib) {
  return glm::vec3(distrib(rand_engine), distrib(rand_engine),
                   distrib(rand_engine));
}

/*******************************************************************************
 * Camera Shaking
 ******************************************************************************/

glm::vec3 GetCameraShaked(const glm::vec3 &v) {
  if (use_camera_wind) {
    return v + camera_shaking_wind * GenRand(camera_shaking_distrib);
  } else {
    return v;
  }
}

void UpdateCameraShakingWind() {
  camera_shaking_wind += GenRand(camera_shaking_distrib).x;
  camera_shaking_wind = glm::clamp(
      camera_shaking_wind, (-kCameraShakingMaxWind), kCameraShakingMaxWind);
}

/*******************************************************************************
 * GL States Updaters
 ******************************************************************************/

dto::GlobalTrans GetCameraTrans() {
  dto::GlobalTrans global_trans;
  const glm::mat4 identity(1.0f);
  const float aspect_ratio = ui_manager.GetWindowAspectRatio();
  global_trans.proj =
      glm::perspective(glm::radians(50.0f), aspect_ratio, 1e-3f, 1e3f);
  global_trans.view = camera_trans.GetTrans();
  global_trans.model = identity;
  return global_trans;
}

void UpdateGlobalTrans() {
  dto::GlobalTrans global_trans = GetCameraTrans();

  // DEBUG: See from light source
  if (kSeeFromLight) {
    global_trans.proj = scene_shader.GetLightProjection();
  }

  scene_shader.UpdateGlobalTrans(global_trans);
}

void UpdateLighting() {
  const glm::vec3 &eye = camera_trans.GetEye();
  scene_shader.UpdateViewPos(eye);
}

void UpdatePostprocInputs() {
  postproc_shader.UpdateEnabled(cur_mode == Modes::comparison &&
                                ui_manager.IsMouseDown(GLUT_LEFT_BUTTON));
}

void UpdateStates() {
  UpdateGlobalTrans();
  UpdateLighting();
  UpdatePostprocInputs();
}

/*******************************************************************************
 * GLUT Callbacks / Display
 ******************************************************************************/

void GLUTDisplayCallback() {
  const glm::ivec2 window_size = ui_manager.GetWindowSize();

  // Update states
  UpdateStates();

  // Draw the scene depth from light source on depth framebuffer
  depth_shader.UseDepthFramebuffer();
  depth_shader.DrawFromLight(window_size);

  // Update wireframe rendering
  if (render_wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Draw the scene on postproc framebuffer "draw original"
  postproc_shader.UsePostprocFramebuffer(
      shader::PostprocShader::PostprocFramebufferTypes::kDrawOriginal, 0);
  postproc_shader.UseDefaultPostprocTextures();
  as::ClearColorBuffer();
  as::ClearDepthBuffer();

  skybox_shader.Draw();
  scene_shader.Draw();

  if (use_fbx) {
    if (!has_collided) {
      fbx_ctrl.Draw();
    } else {
      if (!has_collision_anim_finished) {
        explosion_fbx_ctrl.Draw();
      }
    }
  }

  // Restore polygon mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  if (use_hdr) {
    // Draw bloom effects on postproc framebuffers "draw scaling", "blur
    // scaling"
    // and "combining"
    postproc_shader.DrawBloom(window_size);
  }

  // Draw post-processing effects on default framebuffer
  scene_shader.UseDefaultFramebuffer();
  as::ClearColorBuffer();
  as::ClearDepthBuffer();
  postproc_shader.DrawPostprocEffects();

  // Draw ImGui on default framebuffer
  scene_shader.UseDefaultFramebuffer();
  if (use_gui) {
    DrawImGui();
  }

  // Swap double buffers
  glutSwapBuffers();
}

void GLUTReshapeCallback(const int width, const int height) {
  const glm::ivec2 window_size = glm::vec2(width, height);
  // Limit the window size
  if (as::LimitGLWindowSize(window_size, kMinWindowSize)) {
    // If the window size has been limited, ignore the new size
    return;
  }
  // Save window size
  ui_manager.SaveWindowSize(window_size);
  // Set the viewport
  glViewport(0, 0, width, height);
  // Update screen textures
  postproc_shader.UpdatePostprocTextures(width, height);
  // Update screen depth renderbuffers
  postproc_shader.UpdatePostprocRenderbuffer(width, height);
  // Update differential rendering stencil renderbuffers
  diff_shader.UpdateObjDiffRenderbuffer(width, height);
  // Update differential rendering framebuffer textures
  diff_shader.UpdateDiffFramebufferTextures(width, height);
  // Update FBX
  fbx_ctrl.OnReshape(width, height);
  explosion_fbx_ctrl.OnReshape(width, height);
  // Update ImGui
  ImGui_ImplFreeGLUT_ReshapeFunc(width, height);
}

/*******************************************************************************
 * GLUT Callbacks / User Interface
 ******************************************************************************/

void GLUTKeyboardCallback(const unsigned char key, const int x, const int y) {
  switch (key) {
    case 'i': {
      // Switch between framebuffers
      diff_shader.ToggleDisplayMode();
    } break;
    case 'r': {
      // Reset camera transformation
      camera_trans.ResetTrans();
      UpdateGlobalTrans();
      UpdateLighting();
      // Reset collision state
      ResetCollision();
    } break;
    case 27: {  // Escape
      glutLeaveMainLoop();
    } break;
  }
  // Mark key state down
  ui_manager.MarkKeyDown(key);
  // Update FBX
  fbx_ctrl.OnKeyboard(key);
  explosion_fbx_ctrl.OnKeyboard(key);
  // Update ImGui
  ImGui_ImplFreeGLUT_KeyboardFunc(key, x, y);
}

void GLUTKeyboardUpCallback(const unsigned char key, const int x, const int y) {
  // Mark key state up
  ui_manager.MarkKeyUp(key);
  // Update ImGui
  ImGui_ImplFreeGLUT_KeyboardUpFunc(key, x, y);
}

void GLUTSpecialCallback(const int key, const int x, const int y) {
  switch (key) {
    case GLUT_KEY_LEFT: {
      const int num_instancing =
          scene_shader.GetSceneModel(kEditingModelName).GetNumInstancing();
      editing_model_instance_idx =
          (editing_model_instance_idx + num_instancing - 1) % num_instancing;
    } break;
    case GLUT_KEY_RIGHT: {
      const int num_instancing =
          scene_shader.GetSceneModel(kEditingModelName).GetNumInstancing();
      editing_model_instance_idx =
          (editing_model_instance_idx + 1) % num_instancing;
    } break;
  }

  // Update ImGui
  ImGui_ImplFreeGLUT_SpecialFunc(key, x, y);
}

void GLUTSpecialUpCallback(const int key, const int x, const int y) {
  // Update ImGui
  ImGui_ImplFreeGLUT_SpecialUpFunc(key, x, y);
}

void GLUTMouseCallback(const int button, const int state, const int x,
                       const int y) {
  const glm::vec2 mouse_pos = glm::vec2(x, y);
  // Check which mouse button is clicked
  switch (state) {
    case GLUT_DOWN: {
      ui_manager.MarkMouseDown(button, mouse_pos);
    } break;
    case GLUT_UP: {
      ui_manager.MarkMouseUp(button, mouse_pos);
    } break;
  }
  // Update mouse position
  postproc_shader.UpdateMousePos(mouse_pos);
  // Update FBX
  fbx_ctrl.OnMouse(button, state, x, y);
  explosion_fbx_ctrl.OnMouse(button, state, x, y);
  // Update ImGui
  ImGui_ImplFreeGLUT_MouseFunc(button, state, x, y);
}

void GLUTMouseWheelCallback(const int button, const int dir, const int x,
                            const int y) {
  if (dir > 0) {
    camera_trans.AddEye(kCameraZoomingStep * glm::vec3(0.0f, 0.0f, -1.0f));
  } else {
    camera_trans.AddEye(kCameraZoomingStep * glm::vec3(0.0f, 0.0f, 1.0f));
  }
  // Update ImGui
  ImGui_ImplFreeGLUT_MouseWheelFunc(button, dir, x, y);
}

void GLUTMotionCallback(const int x, const int y) {
  const glm::ivec2 mouse_pos = glm::vec2(x, y);
  // Get ImGui IO
  ImGuiIO &io = ImGui::GetIO();
  // Check whether the ImGui widget isn't active
  if (!io.WantCaptureMouse) {
    // Check whether the left mouse button is down
    if (ui_manager.IsMouseDown(GLUT_LEFT_BUTTON)) {
      switch (cur_mode) {
        case Modes::comparison: {
        } break;
        case Modes::navigation: {
          const glm::ivec2 diff = mouse_pos - ui_manager.GetMousePos();
          camera_trans.AddAngles(kCameraRotationSensitivity *
                                 glm::vec3((-diff.y), (-diff.x), 0.0f));
        } break;
        default: { throw new std::runtime_error("Unknown mode"); }
      }
    }
    // Update mouse position
    postproc_shader.UpdateMousePos(mouse_pos);
    // Save mouse position
    ui_manager.MarkMouseMotion(mouse_pos);
  }
  // Update FBX
  fbx_ctrl.OnMotion(x, y);
  explosion_fbx_ctrl.OnMotion(x, y);
  // Update ImGui
  ImGui_ImplFreeGLUT_MotionFunc(x, y);
}

void GLUTPassiveMotionCallback(const int x, const int y) {
  // Update ImGui
  ImGui_ImplFreeGLUT_MotionFunc(x, y);
}

void GLUTCloseCallback() { ui_manager.MarkWindowClosed(); }

/*******************************************************************************
 * GLUT Callbacks / Timer
 ******************************************************************************/

void GLUTTimerCallback(const int val) {
  // Check whether the window has closed
  if (ui_manager.IsWindowClosed()) {
    return;
  }

  // Get the elapsed time
  const float elapsed_time =
      static_cast<float>(ui_manager.CalcElapsedSeconds());

  // Calculate elapsed time difference
  const float elapsed_time_diff = elapsed_time - last_elapsed_time;

  // Update last elapsed time
  last_elapsed_time = elapsed_time;

  // Update black hawk transformation
  if (ui_manager.IsKeyDown('w')) {
    fbx_camera_ctrl.AddPos(glm::vec3(0.0f, 0.0f, -1.0f));
    fbx_camera_ctrl.AddRot(glm::vec3(-1.0f, 0.0f, 0.0f));
    aircraft_ctrl.AddSpeed(1.0f);
  }
  if (ui_manager.IsKeyDown('s')) {
    fbx_camera_ctrl.AddPos(glm::vec3(0.0f, 0.0f, 1.0f));
    fbx_camera_ctrl.AddRot(glm::vec3(1.0f, 0.0f, 0.0f));
    aircraft_ctrl.AddSpeed(-1e-1f);
  }
  if (ui_manager.IsKeyDown('a')) {
    fbx_camera_ctrl.AddPos(glm::vec3(-1.0f, 0.0f, 0.0f));
    fbx_camera_ctrl.AddRot(glm::vec3(0.0f, 0.0f, 1.0f));
    aircraft_ctrl.AddDriftDir(glm::vec3(0.0f, 1.0f, 0.0f));
    aircraft_ctrl.AddSpeed(-1e-1f);
  }
  if (ui_manager.IsKeyDown('d')) {
    fbx_camera_ctrl.AddPos(glm::vec3(1.0f, 0.0f, 0.0f));
    fbx_camera_ctrl.AddRot(glm::vec3(0.0f, 0.0f, -1.0f));
    aircraft_ctrl.AddDriftDir(glm::vec3(0.0f, -1.0f, 0.0f));
    aircraft_ctrl.AddSpeed(-1e-1f);
  }
  if (ui_manager.IsKeyDown('z')) {
    fbx_camera_ctrl.AddPos(glm::vec3(0.0f, 1.0f, 0.0f));
    aircraft_ctrl.AddDriftDir(glm::vec3(1.0f, 0.0f, 0.0f));
    aircraft_ctrl.AddSpeed(-5e-2f);
  }
  if (ui_manager.IsKeyDown('x')) {
    fbx_camera_ctrl.AddPos(glm::vec3(0.0f, -1.0f, 0.0f));
    aircraft_ctrl.AddDriftDir(glm::vec3(-1.0f, 0.0f, 0.0f));
    aircraft_ctrl.AddSpeed(-5e-2f);
  }

  // Camera transformation for debugging
  if (ui_manager.IsKeyDown('t')) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(0.0f, 0.0f, -1.0f));
  }
  if (ui_manager.IsKeyDown('g')) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(0.0f, 0.0f, 1.0f));
  }
  if (ui_manager.IsKeyDown('f')) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(-1.0f, 0.0f, 0.0f));
  }
  if (ui_manager.IsKeyDown('h')) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(1.0f, 0.0f, 0.0f));
  }
  if (ui_manager.IsKeyDown('c')) {
    camera_trans.AddEyeWorldSpace(kCameraMovingStep *
                                  glm::vec3(0.0f, 1.0f, 0.0f));
  }
  if (ui_manager.IsKeyDown('v')) {
    camera_trans.AddEyeWorldSpace(kCameraMovingStep *
                                  glm::vec3(0.0f, -1.0f, 0.0f));
  }

  // Model transformation for debugging
  {
    dto::SceneModel &scene_model =
        scene_shader.GetSceneModel(kEditingModelName);

    // Scaling
    if (ui_manager.IsKeyDown('[')) {
      scene_model.SetInstancingScaling(
          editing_model_instance_idx,
          scene_model.GetInstancingScaling(editing_model_instance_idx) *
              glm::vec3(1.0f - kEditingModelScalingStep));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown(']')) {
      scene_model.SetInstancingScaling(
          editing_model_instance_idx,
          scene_model.GetInstancingScaling(editing_model_instance_idx) *
              glm::vec3(1.0f + kEditingModelScalingStep));
      scene_shader.UpdateSceneModel(scene_model);
    }
    // Rotation
    if (ui_manager.IsKeyDown('1')) {
      scene_model.SetInstancingRotation(
          editing_model_instance_idx,
          scene_model.GetInstancingRotation(editing_model_instance_idx) +
              kEditingModelRotationStep * glm::vec3(-1.0f, 0.0f, 0.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('2')) {
      scene_model.SetInstancingRotation(
          editing_model_instance_idx,
          scene_model.GetInstancingRotation(editing_model_instance_idx) +
              kEditingModelRotationStep * glm::vec3(1.0f, 0.0f, 0.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('3')) {
      scene_model.SetInstancingRotation(
          editing_model_instance_idx,
          scene_model.GetInstancingRotation(editing_model_instance_idx) +
              kEditingModelRotationStep * glm::vec3(0.0f, -1.0f, 0.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('4')) {
      scene_model.SetInstancingRotation(
          editing_model_instance_idx,
          scene_model.GetInstancingRotation(editing_model_instance_idx) +
              kEditingModelRotationStep * glm::vec3(0.0f, 1.0f, 0.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('5')) {
      scene_model.SetInstancingRotation(
          editing_model_instance_idx,
          scene_model.GetInstancingRotation(editing_model_instance_idx) +
              kEditingModelRotationStep * glm::vec3(0.0f, 0.0f, -1.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('6')) {
      scene_model.SetInstancingRotation(
          editing_model_instance_idx,
          scene_model.GetInstancingRotation(editing_model_instance_idx) +
              kEditingModelRotationStep * glm::vec3(0.0f, 0.0f, 1.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    // Translation
    if (ui_manager.IsKeyDown('i')) {
      editing_model_instance_idx,
          scene_model.SetInstancingTranslation(
              editing_model_instance_idx,
              scene_model.GetInstancingTranslation(editing_model_instance_idx) +
                  kEditingModelTranslationStep * glm::vec3(0.0f, 0.0f, -1.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('k')) {
      scene_model.SetInstancingTranslation(
          editing_model_instance_idx,
          scene_model.GetInstancingTranslation(editing_model_instance_idx) +
              kEditingModelTranslationStep * glm::vec3(0.0f, 0.0f, 1.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('j')) {
      scene_model.SetInstancingTranslation(
          editing_model_instance_idx,
          scene_model.GetInstancingTranslation(editing_model_instance_idx) +
              kEditingModelTranslationStep * glm::vec3(-1.0f, 0.0f, 0.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('l')) {
      scene_model.SetInstancingTranslation(
          editing_model_instance_idx,
          scene_model.GetInstancingTranslation(editing_model_instance_idx) +
              kEditingModelTranslationStep * glm::vec3(1.0f, 0.0f, 0.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('b')) {
      scene_model.SetInstancingTranslation(
          editing_model_instance_idx,
          scene_model.GetInstancingTranslation(editing_model_instance_idx) +
              kEditingModelTranslationStep * glm::vec3(0.0f, 1.0f, 0.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
    if (ui_manager.IsKeyDown('n')) {
      scene_model.SetInstancingTranslation(
          editing_model_instance_idx,
          scene_model.GetInstancingTranslation(editing_model_instance_idx) +
              kEditingModelTranslationStep * glm::vec3(0.0f, -1.0f, 0.0f));
      scene_shader.UpdateSceneModel(scene_model);
    }
  }

  // Update model transformation
  if (ui_manager.IsKeyDown('q')) {
    scene_shader.UpdateSceneModelTrans(glm::radians(1.0f));
  }
  if (ui_manager.IsKeyDown('e')) {
    scene_shader.UpdateSceneModelTrans(glm::radians(-1.0f));
  }

  if (use_aircraft_ctrl) {
    // Update camera eye and angles from aircraft controller
    if (kUpdateCameraFromAircraftController) {
      const glm::vec3 aircraft_pos = aircraft_ctrl.GetPos();
      const glm::vec3 aircraft_dir = aircraft_ctrl.GetDir();

      camera_trans.SetEye(GetCameraShaked(aircraft_pos));
      camera_trans.SetAngles(GetCameraShaked(aircraft_dir));
    }
  }

  // Update camera transformation
  UpdateGlobalTrans();
  UpdateLighting();

  // Update time
  postproc_shader.UpdateTime(elapsed_time);

  // Update fbx camera controller
  fbx_camera_ctrl.Update();

  if (use_aircraft_ctrl) {
    // Update aircraft controller
    aircraft_ctrl.Update();
  }

  /* Collision */

  // Check collision and update
  collision_dist_update_elapsed_time += elapsed_time_diff;
  if (collision_dist_update_elapsed_time > kCollisionDistUpdateInterval) {
    cur_collision_dist = GetCollisionDist();

    if (use_sound) {
      if (!has_collided && cur_collision_dist < kCollisionWarningDist) {
        if (sound_ctrl.IsSoundFinished("pull_up_alarm")) {
          sound_ctrl.Register2DSound("pull_up_alarm",
                                     "assets/sound/boeing-pull-up-alarm.wav");
        }
        if (sound_ctrl.IsSoundFinished("altitude_warning")) {
          sound_ctrl.Register2DSound(
              "altitude_warning",
              "assets/sound/helicopter-altitude-warning-sound.wav", true);
          sound_ctrl.SetSoundVolume("altitude_warning", 0.2f);
        }
      } else {
        sound_ctrl.SetSoundStop("altitude_warning");
      }
    }

    if (cur_collision_dist < kCollisionDist) {
      StartCollision();
    }
    collision_dist_update_elapsed_time = 0.0f;
  }

  if (has_collided) {
    // Update collision animation elapsed time
    collision_anim_elapsed_time += elapsed_time_diff;

    if (use_sound) {
      // Update sad sound volume
      sound_ctrl.SetSoundVolume(
          "sad_violin",
          glm::clamp(0.2f * collision_anim_elapsed_time, 0.0f, 1.0f));
    }
  }

  // Check whether the explosion animation is finished
  if (collision_anim_elapsed_time > kBlackHawkExplosionAnimDuration) {
    has_collision_anim_finished = true;
  }

  // Update collision post-processing effect
  if (has_collided) {
    postproc_shader.UpdateEffectAmount(collision_anim_elapsed_time /
                                       kBlackHawkExplosionAnimDuration);
    if (collision_anim_elapsed_time <
        kCollisionShakingDurationRatio * kBlackHawkExplosionAnimDuration) {
      postproc_shader.UpdateUseShakingEffect(true);
    } else {
      postproc_shader.UpdateUseBlurringEffect(true);
    }
  } else {
    postproc_shader.UpdateUseShakingEffect(false);
    postproc_shader.UpdateUseBlurringEffect(false);
  }

  /* FBX controllers */

  // Update FBX controller
  fbx_ctrl.SetTime(elapsed_time / kBlackHawkAnimDuration);
  fbx_ctrl.SetCameraTransform(glm::vec3(0.0f), glm::vec3(0.0f), 0.0f);
  fbx_ctrl.SetModelTransform(fbx_camera_ctrl.GetPos(), fbx_camera_ctrl.GetRot(),
                             fbx_camera_ctrl.GetScaling());

  explosion_fbx_ctrl.SetTime(collision_anim_elapsed_time /
                             kBlackHawkExplosionAnimDuration);
  explosion_fbx_ctrl.SetCameraTransform(glm::vec3(0.0f), glm::vec3(0.0f), 0.0f);
  explosion_fbx_ctrl.SetModelTransform(
      fbx_camera_ctrl.GetPos() + glm::vec3(20.0f, -20.0f, -140.0f),
      fbx_camera_ctrl.GetRot() +
          glm::radians(glm::vec3(-105.0f, -90.0f, 200.0f)),
      fbx_camera_ctrl.GetScaling());

  /* Sound controller */

  // Check whether to start or stop the sound
  if (use_sound != last_use_sound) {
    if (use_sound) {
      if (!has_collided) {
        StartInitialSound();
      }
    } else {
      sound_ctrl.StopAllSound();
    }

    last_use_sound = use_sound;
  }

  // Update sound controller
  if (use_sound) {
    sound_ctrl.Set3DSoundPosition(
        "helicopter_hovering",
        glm::pow(glm::vec3(0.2f * fbx_camera_ctrl.GetPos().x, 0.0f, 0.0f),
                 glm::vec3(2.0f)));
  }

  // Update normal state
  scene_shader.ToggleNormalHeight(use_normal);

  // Update instantiating state
  scene_shader.ToggleInstantiating(use_instantiating);

  // Update surrounding visibility
  scene_shader.GetSceneModel("surround").SetVisible(use_surrounding);

  // Update fog state
  scene_shader.ToggleFog(use_fog);
  scene_shader.ToggleMixFogWithSkybox(mix_fog_with_skybox);

  // Update gamma correction state
  postproc_shader.UpdateUseGammaCorrect(use_gamma_correct);

  // Update camera shaking wind
  UpdateCameraShakingWind();

  // Mark the current window as needing to be redisplayed
  glutPostRedisplay();

  // Register the timer callback again
  glutTimerFunc(kTimerInterval, GLUTTimerCallback, val);
}

/*******************************************************************************
 * GLUT Callbacks / Menus
 ******************************************************************************/

void GLUTMainMenuCallback(const int id) {
  switch (id) {
    case MainMenuItems::kMainMidLevelSep: {
    } break;
    case MainMenuItems::kMainExit: {
      glutLeaveMainLoop();
    } break;
    default: {
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
    }
  }
}

void GLUTGuiMenuCallback(const int id) {
  switch (id) {
    case GuiMenuItems::kGuiOn: {
      use_gui = true;
    } break;
    case GuiMenuItems::kGuiOff: {
      use_gui = false;
    } break;
    default: {
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
    }
  }
}

void GLUTTimerMenuCallback(const int id) {
  switch (id) {
    case TimerMenuItems::kTimerStart: {
      ui_manager.StartClock();
    } break;
    case TimerMenuItems::kTimerStop: {
      ui_manager.StopClock();
    } break;
    default: {
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
    }
  }
}

/*******************************************************************************
 * GLUT Handlers
 ******************************************************************************/

void RegisterGLUTCallbacks() {
  /* Display */
  glutDisplayFunc(GLUTDisplayCallback);
  glutReshapeFunc(GLUTReshapeCallback);
  /* User Interface */
  glutKeyboardFunc(GLUTKeyboardCallback);
  glutKeyboardUpFunc(GLUTKeyboardUpCallback);
  glutSpecialFunc(GLUTSpecialCallback);
  glutSpecialUpFunc(GLUTSpecialUpCallback);
  glutMouseFunc(GLUTMouseCallback);
  glutMouseWheelFunc(GLUTMouseWheelCallback);
  glutMotionFunc(GLUTMotionCallback);
  glutPassiveMotionFunc(GLUTPassiveMotionCallback);
  glutCloseFunc(GLUTCloseCallback);
  /* Timer */
  glutTimerFunc(kTimerInterval, GLUTTimerCallback, 0);
}

void CreateGLUTMenus() {
  const int main_menu_hdlr = glutCreateMenu(GLUTMainMenuCallback);
  const int gui_submenu_hdlr = glutCreateMenu(GLUTGuiMenuCallback);
  const int timer_submenu_hdlr = glutCreateMenu(GLUTTimerMenuCallback);

  /* Main Menu */
  glutSetMenu(main_menu_hdlr);
  glutAddSubMenu("GUI", gui_submenu_hdlr);
  glutAddSubMenu("Timer", timer_submenu_hdlr);
  glutAddMenuEntry("Exit", MainMenuItems::kMainExit);

  /* GUI Submenu */
  glutSetMenu(gui_submenu_hdlr);
  glutAddMenuEntry("On", GuiMenuItems::kGuiOn);
  glutAddMenuEntry("Off", GuiMenuItems::kGuiOff);

  /* Timer Submenu */
  glutSetMenu(timer_submenu_hdlr);
  glutAddMenuEntry("Start", TimerMenuItems::kTimerStart);
  glutAddMenuEntry("Stop", TimerMenuItems::kTimerStop);

  glutSetMenu(main_menu_hdlr);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void EnterGLUTLoop() { glutMainLoop(); }

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

int main(int argc, char *argv[]) {
  try {
    // DEBUG: See from light source
    if (kSeeFromLight) {
      camera_trans.SetEye(scene_shader.GetLightPos());
      camera_trans.SetAngles(scene_shader.GetLightAngles());
    }

    InitGLUT(argc, argv);
    as::InitGLEW();
    ConfigGL();
    InitFbx();
    InitImGui();
    StartInitialSound();
    RegisterGLUTCallbacks();
    CreateGLUTMenus();
    EnterGLUTLoop();
    DestoryImGui();
  } catch (const std::exception &ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
    throw;
  }
  return 0;
}
