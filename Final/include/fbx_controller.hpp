#pragma once

#include "fbxsdk_impl/SceneContext.h"

#include "as/common.hpp"

namespace ctrl {
class FbxController {
 public:
  class MyMemoryAllocator {
   public:
    static void *MyMalloc(size_t pSize) {
      char *p = (char *)malloc(pSize + FBXSDK_MEMORY_ALIGNMENT);
      memset(p, '#', FBXSDK_MEMORY_ALIGNMENT);
      return p + FBXSDK_MEMORY_ALIGNMENT;
    }

    static void *MyCalloc(size_t pCount, size_t pSize) {
      char *p = (char *)calloc(pCount, pSize + FBXSDK_MEMORY_ALIGNMENT);
      memset(p, '#', FBXSDK_MEMORY_ALIGNMENT);
      return p + FBXSDK_MEMORY_ALIGNMENT;
    }

    static void *MyRealloc(void *pData, size_t pSize) {
      if (pData) {
        FBX_ASSERT(*((char *)pData - 1) == '#');
        if (*((char *)pData - 1) == '#') {
          char *p = (char *)realloc((char *)pData - FBXSDK_MEMORY_ALIGNMENT,
                                    pSize + FBXSDK_MEMORY_ALIGNMENT);
          memset(p, '#', FBXSDK_MEMORY_ALIGNMENT);
          return p + FBXSDK_MEMORY_ALIGNMENT;
        } else {  // Mismatch
          char *p =
              (char *)realloc((char *)pData, pSize + FBXSDK_MEMORY_ALIGNMENT);
          memset(p, '#', FBXSDK_MEMORY_ALIGNMENT);
          return p + FBXSDK_MEMORY_ALIGNMENT;
        }
      } else {
        char *p = (char *)realloc(NULL, pSize + FBXSDK_MEMORY_ALIGNMENT);
        memset(p, '#', FBXSDK_MEMORY_ALIGNMENT);
        return p + FBXSDK_MEMORY_ALIGNMENT;
      }
    }

    static void MyFree(void *pData) {
      if (pData == NULL) return;
      FBX_ASSERT(*((char *)pData - 1) == '#');
      if (*((char *)pData - 1) == '#') {
        free((char *)pData - FBXSDK_MEMORY_ALIGNMENT);
      } else {  // Mismatch
        free(pData);
      }
    }
  };

  FbxController();

  ~FbxController();

  void Initialize(const std::string &fbx_path, const glm::ivec2 &window_size);

  void Draw();

  void GetCameraTransform(glm::vec3 &eye, glm::vec3 &center, glm::vec3 &up,
                          float &roll);

  void GetModelTransform(glm::vec3 &translation, glm::vec3 &rotation,
                         glm::vec3 &scaling);

  void SetTime(const double ratio);

  void SetCameraTransform(const glm::vec3 &eye, const glm::vec3 &angles,
                          const float &roll);

  void SetModelTransform(const glm::vec3 &translation,
                         const glm::vec3 &rotation, const glm::vec3 &scaling);

  void OnReshape(const int width, const int height);

  void OnKeyboard(const unsigned char key);

  void OnMouse(const int button, const int state, const int x, const int y);

  void OnMotion(const int x, const int y);

 private:
  /* FbxSdk Implementations */
  SceneContext *gSceneContext;

  /* Initialization Status */
  static bool init_mem_allocator_;

  void InitializeMemoryAllocator();

  void ConfigSceneContext();
};
}  // namespace ctrl
