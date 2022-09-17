/*
 * spvm-shadertoy
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <string>
#include "module.h"
#include "quad.h"
#include "image.h"
#include "timer.h"
#include "settings.h"
#include "threadpool.h"

namespace SPVM {
namespace ShaderToy {

typedef struct Buffer2D_ {
  size_t width;
  size_t height;
  uint8_t *data;
} Buffer2D;

typedef struct vec3_ {
  float x;
  float y;
  float z;
} vec3;

typedef struct vec4_ {
  float x;
  float y;
  float z;
  float w;
} vec4;

typedef struct UniformInput_ {
  vec3 iResolution;             // viewport resolution (in pixels)
  float iTime;                  // shader playback time (in seconds)
  float iTimeDelta;             // render time (in seconds)
  int iFrame;                   // shader playback frame
  vec4 iMouse;                  // mouse pixel coords. xy: current (if MLB down), zw: click
  vec4 iDate;                   // (year, month, day, time in seconds)
  float iSampleRate;            // sound sample rate (i.e., 44100)
} UniformInput;

class Renderer {
 public:
  Renderer();
  ~Renderer();
  bool create(void *window, int width, int height);
  bool reloadShader(const char *shaderPath);
  void drawFrame();
  void updateSize(int width, int height);
  void destroy();

  inline Buffer2D *getFrame() {
    return colorBuffer_;
  }

  inline Settings &getSettings() {
    return settings_;
  }

 private:
  void createBuffer(int width, int height);
  void destroyBuffer();

  static std::string readFileString(const char *path);
  inline void execBlockShading(int threadId, int blockX, int blockY, int blockSize);
  static inline void pixelColorCvt(uint8_t *rowPtr, size_t x, size_t width, float fragColor[4]);

 private:
  Settings settings_;
  int rasterBlockSize_ = 32;
  ThreadPool threadPool_;

  SpvmModule *module_ = nullptr;
  std::vector<RuntimeQuadContext> runtimes_;

  std::string shaderWrapperStr_;
  std::vector<uint32_t> spvBytes_;
  Buffer2D *colorBuffer_ = nullptr;

  UniformInput uniformInput_;
  Timer timer_;
  float lastFrameTime_ = 0.f;
  int frameIdx_ = 0;

  SpvmImage *iChannel0Image_ = nullptr;
  SpvmSampler *iChannel0Sampler_ = nullptr;
  SpvmSampledImage *iChannel0SampledImage_ = nullptr;
};

}
}
