/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include "spvm.h"

namespace SPVM {

#define kMaxSamplerLodBias 64.f
#define kMaxSamplerLod 64.f

typedef enum SpvmFormat_ {
  SPVM_FORMAT_UNDEFINED = 0,
  SPVM_FORMAT_R8_UNORM = 9,
  SPVM_FORMAT_R8G8_UNORM = 16,
  SPVM_FORMAT_R8G8B8_UNORM = 23,
  SPVM_FORMAT_R8G8B8A8_UNORM = 37,
} SpvmFormat;

typedef enum SpvmBorderColor_ {
  SPVM_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK = 0,
  SPVM_BORDER_COLOR_INT_TRANSPARENT_BLACK = 1,
  SPVM_BORDER_COLOR_FLOAT_OPAQUE_BLACK = 2,
  SPVM_BORDER_COLOR_INT_OPAQUE_BLACK = 3,
  SPVM_BORDER_COLOR_FLOAT_OPAQUE_WHITE = 4,
  SPVM_BORDER_COLOR_INT_OPAQUE_WHITE = 5,
} SpvmBorderColor;

typedef enum SpvmCompareOp_ {
  SPVM_COMPARE_OP_NEVER = 0,
  SPVM_COMPARE_OP_LESS = 1,
  SPVM_COMPARE_OP_EQUAL = 2,
  SPVM_COMPARE_OP_LESS_OR_EQUAL = 3,
  SPVM_COMPARE_OP_GREATER = 4,
  SPVM_COMPARE_OP_NOT_EQUAL = 5,
  SPVM_COMPARE_OP_GREATER_OR_EQUAL = 6,
  SPVM_COMPARE_OP_ALWAYS = 7,
} SpvmCompareOp;

typedef struct SpvmImageInfo_ {
  SpvDim dim;
  SpvmFormat format;
  SpvmWord width;
  SpvmWord height;
  SpvmWord depth;
  SpvmBool mipmaps;
  SpvmWord baseMipLevel;
  SpvmWord mipLevels;
  SpvmBool arrayed;
  SpvmWord baseArrayLayer;
  SpvmWord arrayLayers;
  SpvmWord samples;
} SpvmImageInfo;

typedef struct SpvmSamplerInfo_ {
  SpvSamplerFilterMode magFilter;
  SpvSamplerFilterMode minFilter;
  SpvSamplerFilterMode mipmapMode;
  SpvSamplerAddressingMode addressModeU;
  SpvSamplerAddressingMode addressModeV;
  SpvSamplerAddressingMode addressModeW;
  SpvmF32 mipLodBias;
  SpvmBool compareEnable;
  SpvmCompareOp compareOp;
  SpvmF32 minLod;
  SpvmF32 maxLod;
  SpvmBorderColor borderColor;
  SpvmBool unnormalizedCoordinates;
} SpvmSamplerInfo;

typedef struct SpvmSampler_ {
  SpvmSamplerInfo info;
} SpvmSampler;

typedef struct SpvmImageLevel_ {
  SpvmWord level;
  SpvmWord width;
  SpvmWord height;
  SpvmWord depth;
  SpvmWord dataSize;
  SpvmByte *data;
} SpvmImageLevel;

typedef struct SpvmImageLayer_ {
  SpvmWord layer;
  SpvmWord width;
  SpvmWord height;
  SpvmWord depth;
  SpvmWord levelCount;
  SpvmImageLevel *levels;
} SpvmImageLayer;

typedef struct SpvmImage_ {
  SpvmImageInfo info;
  SpvmWord layerCount;
  SpvmImageLayer *layers;
} SpvmImage;

typedef struct SpvmSampledImage_ {
  SpvmImage *image;
  SpvmSampler *sampler;
} SpvmSampledImage;

SpvmImage *createImage(SpvmImageInfo *imageInfo);
SpvmSampler *createSampler(SpvmSamplerInfo *samplerInfo = nullptr);
SpvmSampler *createSamplerConstant(SpvmSamplerAttribute *attribute);
SpvmSampledImage *createSampledImage(SpvmImage *image, SpvmSampler *sampler);

bool uploadImageData(SpvmImage *image, SpvmByte *dataPtr, SpvmWord dataSize,
                     SpvmWord width, SpvmWord height, SpvmWord depth = 1,
                     SpvmWord mipLevel = 0, SpvmWord arrayLayer = 0);

void generateMipmaps(SpvmImage *image, SpvSamplerFilterMode filterMode);
void generateMipmaps(SpvmSampledImage *sampledImage);

void destroyImage(SpvmImage *image);
void destroySampler(SpvmSampler *sampler);
void destroySampledImager(SpvmSampledImage *sampledImage);

bool checkImageType(SpvmImage *image, SpvmTypeImage *type);

void sampleImage(void *ctx,
                 SpvmValue *retValue,
                 SpvmValue *sampledImageValue,
                 SpvmValue *coordinateValue,
                 SpvmWord coordinateId,
                 SpvmImageOperands *operands,
                 SpvmValue *depthCompValue = nullptr,
                 bool proj = false);

void fetchImage(void *ctx,
                SpvmValue *retValue,
                SpvmValue *imageValue,
                SpvmValue *coordinateValue,
                SpvmWord coordinateId,
                SpvmImageOperands *operands);

void queryImageFormat(void *ctx, SpvmValue *retValue, SpvmValue *imageValue);
void queryImageOrder(void *ctx, SpvmValue *retValue, SpvmValue *imageValue);
void queryImageSizeLod(void *ctx, SpvmValue *retValue, SpvmValue *imageValue, SpvmValue *lodValue);
void queryImageSize(void *ctx, SpvmValue *retValue, SpvmValue *imageValue);
void queryImageLod(void *ctx,
                   SpvmValue *retValue,
                   SpvmValue *sampledImageValue,
                   SpvmValue *coordinateValue,
                   SpvmWord coordinateId);
void queryImageLevels(void *ctx, SpvmValue *retValue, SpvmValue *imageValue);
void queryImageSamples(void *ctx, SpvmValue *retValue, SpvmValue *imageValue);

}