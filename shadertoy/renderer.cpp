/*
 * spvm-shadertoy
 * @author 	: keith@robot9.me
 *
 */

#include "renderer.h"
#include "decoder.h"
#include "compiler.h"
#include "utils.h"
#include "logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define HEAP_SIZE 512 * 1024
#define PIXEL_CONVERT(val) 255 * fClamp(val, 0.f, 1.f)

const char *IMAGE_0_PATH = "assets/images/awesomeface.png";

namespace SPVM {
namespace ShaderToy {

Renderer::Renderer() : colorBuffer_(nullptr) {}

Renderer::~Renderer() {
  destroy();
}

bool Renderer::create(void *window, int width, int height) {
  colorBuffer_ = new Buffer2D();
  colorBuffer_->data = nullptr;
  createBuffer(width, height);

  // init uniform
  uniformInput_.iResolution.x = (float) colorBuffer_->width;
  uniformInput_.iResolution.y = (float) colorBuffer_->height;
  uniformInput_.iResolution.z = 0;

  int iw = 0, ih = 0, n = 0;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *pixelBytes = stbi_load(IMAGE_0_PATH, &iw, &ih, &n, STBI_default);
  if (pixelBytes != nullptr && n > 0) {
    // image
    SPVM::SpvmImageInfo imageInfo;
    imageInfo.dim = SpvDim2D;
    imageInfo.format = SPVM_FORMAT_UNDEFINED;
    imageInfo.width = iw;
    imageInfo.height = ih;
    imageInfo.depth = 1;
    imageInfo.mipmaps = true;
    imageInfo.baseMipLevel = 0;
    imageInfo.mipLevels = 5;
    imageInfo.arrayed = false;
    imageInfo.baseArrayLayer = 0;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = 1;
    switch (n) {
      case 1:
        imageInfo.format = SPVM_FORMAT_R8_UNORM;
        break;
      case 2:
        imageInfo.format = SPVM_FORMAT_R8G8_UNORM;
        break;
      case 3:
        imageInfo.format = SPVM_FORMAT_R8G8B8_UNORM;
        break;
      case 4:
        imageInfo.format = SPVM_FORMAT_R8G8B8A8_UNORM;
        break;
      default:
        break;
    }

    iChannel0Image_ = SPVM::createImage(&imageInfo);
    SPVM::uploadImageData(iChannel0Image_, pixelBytes, iw * ih * n, iw, ih, 1);
    stbi_image_free(pixelBytes);

    // mipmaps
    generateMipmaps(iChannel0Image_, SpvSamplerFilterModeLinear);

    // sampler
    iChannel0Sampler_ = SPVM::createSampler();
    iChannel0Sampler_->info.minFilter = SpvSamplerFilterModeLinear;
    iChannel0Sampler_->info.magFilter = SpvSamplerFilterModeLinear;
    iChannel0Sampler_->info.mipmapMode = SpvSamplerFilterModeLinear;

    // sampledImage
    iChannel0SampledImage_ = SPVM::createSampledImage(iChannel0Image_, iChannel0Sampler_);
  }

  // load shader
  settings_.loadShaderCallback_ = [&](std::string shaderPath) {
    reloadShader(shaderPath.c_str());
  };
  shaderWrapperStr_ = readFileString(settings_.shaderWrapperPath_.c_str());
  if (shaderWrapperStr_.empty()) {
    LOGE("error read shader wrapper file: %s", settings_.shaderWrapperPath_.c_str());
    return false;
  }

  return reloadShader(settings_.getShaderPath().c_str());
}

bool Renderer::reloadShader(const char *shaderPath) {
  LOGI("start load shader: %s", shaderPath);
  std::string shaderStr = readFileString(shaderPath);
  if (shaderStr.empty()) {
    LOGE("error read shader file: %s", shaderPath);
    return false;
  }

  shaderStr = shaderWrapperStr_ + shaderStr;
  spvBytes_ = Compiler::compileShaderFragment(shaderStr.c_str());
  if (spvBytes_.empty()) {
    LOGE("error compile shader file to spir-v");
    return false;
  }

  // decode spv file
  if (module_) {
    delete module_;
  }
  module_ = new SpvmModule();
  bool success = SPVM::Decoder::decodeBytes((const SpvmByte *) (spvBytes_.data()),
                                            spvBytes_.size() * sizeof(uint32_t),
                                            module_);
  if (!success) {
    LOGE("decode spv file failed");
    return false;
  }

  // init runtime
  runtimes_.clear();
  runtimes_.resize(threadPool_.getThreadCnt());
  for (size_t i = 0; i < runtimes_.size(); i++) {
    Runtime &rt = runtimes_[i];
    success = rt.initWithModule(module_, HEAP_SIZE);
    if (!success) {
      LOGE("init spvm runtime failed");
      return false;
    }

    if (iChannel0SampledImage_) {
      rt.writeUniformBinding(iChannel0SampledImage_, 0, 1, 0);
    }
  }

  lastFrameTime_ = 0.f;
  frameIdx_ = 0;
  timer_.start();

  LOGI("load shader success.");
  return true;
}

void Renderer::drawFrame() {
  // update uniform
  uniformInput_.iTime = (float) timer_.elapse() / 1000.f;
  uniformInput_.iTimeDelta = uniformInput_.iTime - lastFrameTime_;
  lastFrameTime_ = uniformInput_.iTime;
  frameIdx_++;
  uniformInput_.iFrame = frameIdx_;
  for (size_t i = 0; i < runtimes_.size(); i++) {
    runtimes_[i].writeUniformBinding(&uniformInput_, 0, 0);
  }

  // fragment shading
  float blockSize = (float) rasterBlockSize_;
  int blockCntY = (int) (((float) colorBuffer_->height + blockSize - 1.f) / blockSize);
  int blockCntX = (int) (((float) colorBuffer_->width + blockSize - 1.f) / blockSize);

  for (int blockY = 0; blockY < blockCntY; blockY++) {
    for (int blockX = 0; blockX < blockCntX; blockX++) {
      threadPool_.pushTask([&, blockX, blockY](int threadId) {
        Runtime &rt = runtimes_[threadId];
        float fragCoord[2];
        float fragColor[4];

        int blockStartX = blockX * (int) blockSize;
        int blockStartY = blockY * (int) blockSize;

        for (size_t y = blockStartY; y <= blockStartY + (int) blockSize && y < colorBuffer_->height; y++) {
          uint8_t *rowPtr = &colorBuffer_->data[y * colorBuffer_->width * 4];
          for (size_t x = blockStartX; x <= blockStartX + (int) blockSize && x < colorBuffer_->width; x++) {
            fragCoord[0] = (float) x;
            fragCoord[1] = (float) y;
            rt.writeInput(fragCoord, 0);

            rt.execEntryPoint();

            rt.readOutput(fragColor, 0);
            uint8_t *pixel = &rowPtr[x * 4];
            pixel[0] = PIXEL_CONVERT(fragColor[0]);
            pixel[1] = PIXEL_CONVERT(fragColor[1]);
            pixel[2] = PIXEL_CONVERT(fragColor[2]);
            pixel[3] = PIXEL_CONVERT(fragColor[3]);
          }
        }
      });
    }
  }

  threadPool_.waitTasksFinish();
}

void Renderer::updateSize(int width, int height) {
  if (colorBuffer_) {
    createBuffer(width, height);
    uniformInput_.iResolution.x = (float) colorBuffer_->width;
    uniformInput_.iResolution.y = (float) colorBuffer_->height;
  }
}

void Renderer::createBuffer(int width, int height) {
  colorBuffer_->width = width;
  colorBuffer_->height = height;
  if (colorBuffer_->data) {
    delete[] colorBuffer_->data;
  }
  colorBuffer_->data = new uint8_t[width * height * 4];
}

void Renderer::destroyBuffer() {
  if (colorBuffer_) {
    delete[] colorBuffer_->data;
    delete colorBuffer_;
    colorBuffer_ = nullptr;
  }
}

void Renderer::destroy() {
  destroyBuffer();
  if (module_) {
    delete module_;
    module_ = nullptr;
  }
}

std::string Renderer::readFileString(const char *path) {
  std::string retStr;
  FILE *file = fopen(path, "rb");
  if (!file) {
    LOGE("error open file");
    return retStr;
  }

  fseek(file, 0, SEEK_END);
  size_t fileLength = ftell(file);
  if (fileLength <= 0) {
    LOGE("error get file size");
    fclose(file);
    return retStr;
  }

  char *buffer = new char[fileLength + 1];
  rewind(file);
  fread(buffer, 1, fileLength, file);
  buffer[fileLength] = '\0';
  retStr = buffer;
  fclose(file);
  delete[] buffer;

  return retStr;
}

}
}