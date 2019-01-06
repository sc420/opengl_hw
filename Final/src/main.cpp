#include "imgui/imgui.h"
#include "imgui/imgui_impl_freeglut.h"
#include "imgui/imgui_impl_opengl3.h"

#include "as/common.hpp"
#include "as/gl/gl_tools.hpp"
#include "as/trans/camera.hpp"

#include "depth_shader.hpp"
#include "diff_shader.hpp"
#include "fbx_controller.hpp"
#include "postproc_shader.hpp"
#include "scene_shader.hpp"
#include "skybox_shader.hpp"

/*******************************************************************************
 * Constants
 ******************************************************************************/

/* User Interfaces */
static const auto kInitWindowRelativeCenterPos = glm::vec2(0.5f, 0.5f);
static const auto kInitWindowSize = glm::ivec2(720, 450);
static const auto kMinWindowSize = glm::ivec2(720, 450);
static const auto kCameraMovingStep = 0.05f;
static const auto kCameraRotationSensitivity = 0.003f;
static const auto kCameraZoomingStep = 1.0f;
/* Timers */
static const auto kTimerInterval = 10;

/*******************************************************************************
 * Camera States
 ******************************************************************************/

// Camera transformations
as::CameraTrans camera_trans(glm::vec3(10.0f, 5.0f, 0.0f),
                             glm::vec3(glm::radians(30.0f), glm::radians(0.0f),
                                       glm::radians(0.0f)));

// as::CameraTrans camera_trans(glm::vec3(-20.0, 30.0f, 30.0f),
//                             glm::vec3(glm::radians(30.0f),
//                             glm::radians(30.0f),
//                                       glm::radians(0.0f)));

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

ctrl::FbxController fbx_ctrl;

/*******************************************************************************
 * User Interface States
 ******************************************************************************/

// Modes
enum class Modes { comparison, navigation };

// Current mode
Modes cur_mode = Modes::navigation;

/*******************************************************************************
 * Menus
 ******************************************************************************/

enum MainMenuItems {
  kMainMidLevelSep,
  kMainImgAbs,
  kMainLaplacian,
  kMainSharpness,
  kMainPixelation,
  kMainAdvancedSep,
  kMainBloomEffect,
  kMainMagnifier,
  kMainCool,
  kMainSpecial,
  kMainExit
};
enum ModeMenuItems { kModeComparison, kModeNavigation };
enum NormalHeightMenuItems { kNormalHeightOn, kNormalHeightOff };
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
  glutCreateWindow("Assignment 4");
}

/*******************************************************************************
 * GL Context Configuration
 ******************************************************************************/

void ConfigGLSettings() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(1.0f);
  glClearStencil(0);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE_ARB);

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
 * FBX Handlers
 ******************************************************************************/

void InitFbx() {
  fbx_ctrl.Initialize(
      "assets/models/blackhawk/blackhawk-helicopter-animation.fbx",
      kInitWindowSize);

  fbx_ctrl.Draw();
  fbx_ctrl.PostDraw();
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
  // Cleanup
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

  /* Debug Widget */

  ImGui::Begin("Debug");

  ImGui::SetNextTreeNodeOpen(true);
  if (ImGui::CollapsingHeader("Camera")) {
    ImGui::Text("Position: (%.1f, %.1f, %.1f)", camera_pos.x, camera_pos.y,
                camera_pos.z);
    ImGui::Text("Angles (Degree): (%.1f, %.1f, %.1f)", camera_angles.x,
                camera_angles.y, camera_angles.z);
  }

  ImGui::SetNextTreeNodeOpen(true);
  if (ImGui::CollapsingHeader("Performance")) {
    ImGui::Text("FPS: %.1f", io.Framerate);
  }

  ImGui::End();
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
 * GL States Updaters
 ******************************************************************************/

void UpdateGlobalTrans() {
  dto::GlobalTrans global_trans;
  const glm::mat4 identity(1.0f);
  const float aspect_ratio = ui_manager.GetWindowAspectRatio();
  global_trans.proj =
      glm::perspective(glm::radians(80.0f), aspect_ratio, 1e-3f, 1e3f);
  // global_trans.proj = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 1e-3f, 1e3f);
  global_trans.view = camera_trans.GetTrans();
  global_trans.model = identity;

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

  // Draw the scene depth on depth framebuffer
  depth_shader.UseDepthFramebuffer();
  as::ClearDepthBuffer();
  depth_shader.Draw(window_size);

  // Draw the scene on postproc framebuffer
  postproc_shader.UseScreenFramebuffer();
  as::ClearColorBuffer();
  as::ClearDepthBuffer();
  scene_shader.Draw();
  skybox_shader.Draw();

  // Draw post-processing effects on default framebuffer
  scene_shader.UseDefaultFramebuffer();
  as::ClearDepthBuffer();
  postproc_shader.Draw();

  // Draw fbx on default framebuffer
  scene_shader.UseDefaultFramebuffer();
  fbx_ctrl.Draw();

  // Draw ImGui on default framebuffer
  scene_shader.UseDefaultFramebuffer();
  DrawImGui();

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
  postproc_shader.UpdateScreenTextures(width, height);
  // Update screen depth renderbuffers
  postproc_shader.UpdateScreenRenderbuffers(width, height);
  // Update differential rendering stencil renderbuffers
  diff_shader.UpdateObjDiffRenderbuffer(width, height);
  // Update differential rendering framebuffer textures
  diff_shader.UpdateDiffFramebufferTextures(width, height);
  // Update FbxSdk
  fbx_ctrl.OnReshape(width, height);
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
    } break;
    case 27: {  // Escape
      glutLeaveMainLoop();
    } break;
  }
  // Mark key state down
  ui_manager.MarkKeyDown(key);
  // Update FbxSdk
  fbx_ctrl.OnKeyboard(key);
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
  // Update FbxSdk
  fbx_ctrl.OnMouse(button, state, x, y);
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
                                 glm::vec3(diff.y, diff.x, 0.0f));
        } break;
        default: { throw new std::runtime_error("Unknown mode"); }
      }
    }
    // Update mouse position
    postproc_shader.UpdateMousePos(mouse_pos);
    // Save mouse position
    ui_manager.MarkMouseMotion(mouse_pos);
  }
  // Update FbxSdk
  fbx_ctrl.OnMotion(x, y);
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

  // Update camera transformation
  if (ui_manager.IsKeyDown('w')) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(0.0f, 0.0f, -1.0f));
    UpdateGlobalTrans();
    UpdateLighting();
  }
  if (ui_manager.IsKeyDown('s')) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(0.0f, 0.0f, 1.0f));
    UpdateGlobalTrans();
    UpdateLighting();
  }
  if (ui_manager.IsKeyDown('a')) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(-1.0f, 0.0f, 0.0f));
    UpdateGlobalTrans();
    UpdateLighting();
  }
  if (ui_manager.IsKeyDown('d')) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(1.0f, 0.0f, 0.0f));
    UpdateGlobalTrans();
    UpdateLighting();
  }
  if (ui_manager.IsKeyDown('z')) {
    camera_trans.AddEyeWorldSpace(kCameraMovingStep *
                                  glm::vec3(0.0f, 1.0f, 0.0f));
    UpdateGlobalTrans();
    UpdateLighting();
  }
  if (ui_manager.IsKeyDown('x')) {
    camera_trans.AddEyeWorldSpace(kCameraMovingStep *
                                  glm::vec3(0.0f, -1.0f, 0.0f));
    UpdateGlobalTrans();
    UpdateLighting();
  }

  // Update model transformation
  if (ui_manager.IsKeyDown('q')) {
    scene_shader.UpdateSceneModelTrans(glm::radians(1.0f));
  }
  if (ui_manager.IsKeyDown('e')) {
    scene_shader.UpdateSceneModelTrans(glm::radians(-1.0f));
  }

  // Update time
  postproc_shader.UpdateTime(elapsed_time);

  // Update FBX
  fbx_ctrl.SetTime(elapsed_time / 5.0f);

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
    case MainMenuItems::kMainImgAbs: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectImgAbs);
    } break;
    case MainMenuItems::kMainLaplacian: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectLaplacian);
    } break;
    case MainMenuItems::kMainSharpness: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectSharpness);
    } break;
    case MainMenuItems::kMainPixelation: {
      postproc_shader.UpdateEffectIdx(
          shader::PostprocShader::kEffectPixelation);
    } break;
    case MainMenuItems::kMainAdvancedSep: {
    } break;
    case MainMenuItems::kMainBloomEffect: {
      postproc_shader.UpdateEffectIdx(
          shader::PostprocShader::kEffectBloomEffect);
    } break;
    case MainMenuItems::kMainMagnifier: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectMagnifier);
    } break;
    case MainMenuItems::kMainCool: {
    } break;
    case MainMenuItems::kMainSpecial: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectSpecial);
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

void GLUTModeMenuCallback(const int id) {
  switch (id) {
    case ModeMenuItems::kModeComparison: {
      cur_mode = Modes::comparison;
    } break;
    case ModeMenuItems::kModeNavigation: {
      cur_mode = Modes::navigation;
    } break;
    default: {
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
    }
  }
}

void GLUTNormalHeightMenuCallback(const int id) {
  switch (id) {
    case NormalHeightMenuItems::kNormalHeightOn: {
      scene_shader.ToggleNormalHeight(true);
    } break;
    case NormalHeightMenuItems::kNormalHeightOff: {
      scene_shader.ToggleNormalHeight(false);
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
  const int mode_submenu_hdlr = glutCreateMenu(GLUTModeMenuCallback);
  const int normal_height_submenu_hdlr =
      glutCreateMenu(GLUTNormalHeightMenuCallback);
  const int timer_submenu_hdlr = glutCreateMenu(GLUTTimerMenuCallback);

  /* Main Menu */
  glutSetMenu(main_menu_hdlr);
  glutAddSubMenu("Mode", mode_submenu_hdlr);
  glutAddSubMenu("Normal&Height Mapping", normal_height_submenu_hdlr);
  glutAddSubMenu("Timer", timer_submenu_hdlr);
  glutAddMenuEntry("(Mid-level)", MainMenuItems::kMainMidLevelSep);
  glutAddMenuEntry("1. Image Abstraction", MainMenuItems::kMainImgAbs);
  glutAddMenuEntry("2. Laplacian Filter", MainMenuItems::kMainLaplacian);
  glutAddMenuEntry("3. Sharpness Filter", MainMenuItems::kMainSharpness);
  glutAddMenuEntry("4. Pixelation", MainMenuItems::kMainPixelation);
  glutAddMenuEntry("(Advanced)", MainMenuItems::kMainAdvancedSep);
  glutAddMenuEntry("1. Blooem Effect", MainMenuItems::kMainBloomEffect);
  glutAddMenuEntry("2. Magnifier", MainMenuItems::kMainMagnifier);
  glutAddMenuEntry("(Cool)", MainMenuItems::kMainCool);
  glutAddMenuEntry("1. Special Effect", MainMenuItems::kMainSpecial);
  glutAddMenuEntry("Exit", MainMenuItems::kMainExit);

  /* Mode Submenu */
  glutSetMenu(mode_submenu_hdlr);
  glutAddMenuEntry("Comparison", ModeMenuItems::kModeComparison);
  glutAddMenuEntry("Navigation", ModeMenuItems::kModeNavigation);

  /* Normal&Height Submenu */
  glutSetMenu(normal_height_submenu_hdlr);
  glutAddMenuEntry("On", NormalHeightMenuItems::kNormalHeightOn);
  glutAddMenuEntry("Off", NormalHeightMenuItems::kNormalHeightOff);

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
    InitGLUT(argc, argv);
    as::InitGLEW();
    ConfigGL();
    InitFbx();
    InitImGui();
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
