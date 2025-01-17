#include "as/common.hpp"
#include "as/gl/gl_tools.hpp"
#include "as/trans/camera.hpp"

#include "depth_shader.hpp"
#include "diff_shader.hpp"
#include "postproc_shader.hpp"
#include "scene_shader.hpp"
#include "skybox_shader.hpp"

namespace fs = std::experimental::filesystem;

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
as::CameraTrans camera_trans(glm::vec3(0.0f, -5.0f, -8.0f),
                             glm::vec3(glm::radians(30.0f),
                                       glm::radians(-80.0f),
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
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
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
 * GL States Updaters
 ******************************************************************************/

void UpdateGlobalTrans() {
  dto::GlobalTrans global_trans;
  const glm::mat4 identity(1.0f);
  const float aspect_ratio = ui_manager.GetWindowAspectRatio();
  global_trans.proj =
      glm::perspective(glm::radians(80.0f), aspect_ratio, 1e-3f, 1e3f);
  // global_trans.proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1e-3f, 1e3f);
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
  as::ClearColorBuffer();
  as::ClearDepthBuffer();
  depth_shader.Draw(window_size);

  // Use obj framebuffer
  diff_shader.UseDiffFramebuffer(shader::DiffShader::DiffTypes::kObj);
  // Enable writing to stencil buffer to ensure clearing
  glStencilMask(0xFF);
  // Clear all buffers
  as::ClearColorBuffer();
  as::ClearStencilBuffer();
  as::ClearDepthBuffer();
  // Draw stencil with 1
  glStencilFunc(GL_ALWAYS, 1, 0xFF);
  // Enable writing to stencil buffer
  glStencilMask(0xFF);
  // Draw the scene
  scene_shader.DrawScene();

  // Use obj framebuffer
  diff_shader.UseDiffFramebuffer(shader::DiffShader::DiffTypes::kObj);
  // Enable writing to stencil buffer to ensure clearing
  glStencilMask(0xFF);
  // Clear color and depth buffers
  as::ClearColorBuffer();
  as::ClearDepthBuffer();
  // Draw fragments if their stencil values are not 1
  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  // Disable writing to stencil buffer
  glStencilMask(0x00);
  // Draw the quad
  scene_shader.DrawQuad(true);

  // Use no_obj framebuffer
  diff_shader.UseDiffFramebuffer(shader::DiffShader::DiffTypes::kNoObj);
  // Enable writing to stencil buffer to ensure clearing
  glStencilMask(0xFF);
  // Clear color and depth buffers
  as::ClearColorBuffer();
  as::ClearDepthBuffer();
  // Enable stencil test
  glEnable(GL_STENCIL_TEST);
  // Draw fragments if their stencil values are not 1
  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  // Disable writing to stencil buffer
  glStencilMask(0x00);
  // Draw the quad without shadow
  scene_shader.DrawQuad(false);

  // Use bg framebuffer
  diff_shader.UseDiffFramebuffer(shader::DiffShader::DiffTypes::kBg);
  // Enable writing to stencil buffer to ensure clearing
  glStencilMask(0xFF);
  // Always pass stencil to ensure clearing
  glStencilFunc(GL_ALWAYS, 1, 0xFF);
  // Clear all buffers
  as::ClearColorBuffer();
  as::ClearStencilBuffer();
  as::ClearDepthBuffer();
  // Draw the scene
  scene_shader.DrawScene();
  // Draw the skybox
  skybox_shader.Draw();

  // Draw the differential rendering result on postproc framebuffer
  postproc_shader.UseScreenFramebuffer();
  as::ClearColorBuffer();
  as::ClearDepthBuffer();
  diff_shader.Draw();

  // Draw post-processing effects on default framebuffer
  scene_shader.UseDefaultFramebuffer();
  as::ClearDepthBuffer();
  postproc_shader.Draw();

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
}

void GLUTKeyboardUpCallback(const unsigned char key, const int x, const int y) {
  // Mark key state up
  ui_manager.MarkKeyUp(key);
}

void GLUTSpecialCallback(const int key, const int x, const int y) {}

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
}

void GLUTMouseWheelCallback(const int button, const int dir, const int x,
                            const int y) {
  if (dir > 0) {
    camera_trans.AddEye(kCameraZoomingStep * glm::vec3(0.0f, 0.0f, -1.0f));
  } else {
    camera_trans.AddEye(kCameraZoomingStep * glm::vec3(0.0f, 0.0f, 1.0f));
  }
}

void GLUTMotionCallback(const int x, const int y) {
  const glm::ivec2 mouse_pos = glm::vec2(x, y);
  // Check whether the left mouse button is down
  if (ui_manager.IsMouseDown(GLUT_LEFT_BUTTON)) {
    switch (cur_mode) {
      case Modes::comparison: {
      } break;
      case Modes::navigation: {
        const glm::ivec2 diff = mouse_pos - ui_manager.GetMousePos();
        camera_trans.AddAngle(kCameraRotationSensitivity *
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

void GLUTCloseCallback() { ui_manager.MarkWindowClosed(); }

/*******************************************************************************
 * GLUT Callbacks / Timer
 ******************************************************************************/

void GLUTTimerCallback(const int val) {
  // Check whether the window has closed
  if (ui_manager.IsWindowClosed()) {
    return;
  }

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

  // Mark the current window as needing to be redisplayed
  glutPostRedisplay();

  // Update time
  const float elapsed_time =
      static_cast<float>(ui_manager.CalcElapsedSeconds());
  postproc_shader.UpdateTime(elapsed_time);

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
  glutMouseFunc(GLUTMouseCallback);
  glutMouseWheelFunc(GLUTMouseWheelCallback);
  glutMotionFunc(GLUTMotionCallback);
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
    RegisterGLUTCallbacks();
    CreateGLUTMenus();
    EnterGLUTLoop();
  } catch (const std::exception &ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
    throw;
  }
  return 0;
}
