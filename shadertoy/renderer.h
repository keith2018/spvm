/*
 * spvm-shadertoy
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <string>
#include "module.h"
#include "runtime.h"
#include "image.h"
#include "timer.h"

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
  bool create(void *window, int width, int height, const char *shaderPath);
  void drawFrame();
  void updateSize(int width, int height);

  inline Buffer2D *getFrame() {
    return colorBuffer_;
  }

 private:
  void createBuffer(int width, int height);
  void destroyBuffer();

 private:
  std::vector<uint32_t> spvBytes_;
  Buffer2D *colorBuffer_;
  SPVM::SpvmModule module_;
  SPVM::Runtime runtime_;

  UniformInput uniformInput_;
  Timer timer_;
  float lastFrameTime_;
  int frameIdx_;

  SpvmImage *iChannel0Image_;
  SpvmSampler *iChannel0Sampler_;
  SpvmSampledImage *iChannel0SampledImage_;
};

}
}
