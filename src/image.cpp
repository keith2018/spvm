/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "image.h"
#include <memory.h>
#include "logger.h"
#include "utils.h"

namespace SPVM {

// image coordinates(i,j,k) with sample weight
typedef struct SpvmImageCoord_ {
  SpvmI32 coord;
  SpvmF32 weight;
} SpvmImageCoord;

#define ImageCoordNull {0, 1.f}

/**
 * image format bytes per pixel
 */
SpvmWord getImageFormatBPP(SpvmFormat format) {
  switch (format) {
    case SPVM_FORMAT_R8_UNORM:
      return 1;
    case SPVM_FORMAT_R8G8_UNORM:
      return 2;
    case SPVM_FORMAT_R8G8B8_UNORM:
      return 3;
    case SPVM_FORMAT_R8G8B8A8_UNORM:
      return 4;
    default:
      LOGE("image format not support");
      break;
  }
  return 0;
}

SpvmImage *createImage(SpvmImageInfo *imageInfo) {
  SpvmImage *ret = (SpvmImage *) malloc(sizeof(SpvmImage));
  ret->info = *imageInfo;
  ret->layerCount = imageInfo->arrayLayers;
  ret->layers = (SpvmImageLayer *) malloc(ret->layerCount * sizeof(SpvmImageLayer));

  // init layers
  for (SpvmWord i = 0; i < ret->layerCount; i++) {
    SpvmImageLayer *layer = &ret->layers[i];
    layer->layer = imageInfo->baseArrayLayer + i;
    layer->width = imageInfo->width;
    layer->height = imageInfo->height;
    layer->depth = imageInfo->depth;
    layer->levelCount = imageInfo->mipLevels;
    layer->levels = (SpvmImageLevel *) malloc(layer->levelCount * sizeof(SpvmImageLevel));

    // init levels
    for (SpvmWord j = 0; j < layer->levelCount; j++) {
      SpvmImageLevel *level = &layer->levels[j];
      level->level = imageInfo->baseMipLevel + j;
      level->width = 0;
      level->height = 0;
      level->depth = 0;
      level->dataSize = 0;
      level->data = nullptr;
    }
  }
  return ret;
}

SpvmSampler *createSampler(SpvmSamplerInfo *samplerInfo) {
  SpvmSampler *ret = (SpvmSampler *) malloc(sizeof(SpvmSampler));
  if (samplerInfo) {
    ret->info = *samplerInfo;
  } else {
    // default sampler
    ret->info.magFilter = SpvSamplerFilterModeNearest;
    ret->info.minFilter = SpvSamplerFilterModeNearest;
    ret->info.mipmapMode = SpvSamplerFilterModeNearest;
    ret->info.addressModeU = SpvSamplerAddressingModeClampToEdge;
    ret->info.addressModeV = SpvSamplerAddressingModeClampToEdge;
    ret->info.addressModeW = SpvSamplerAddressingModeClampToEdge;
    ret->info.mipLodBias = 0.f;
    ret->info.compareEnable = false;
    ret->info.compareOp = SPVM_COMPARE_OP_NEVER;
    ret->info.minLod = 0.f;
    ret->info.maxLod = kMaxSamplerLod;
    ret->info.borderColor = SPVM_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    ret->info.unnormalizedCoordinates = false;
  }
  return ret;
}

SpvmSampler *createSamplerConstant(SpvmSamplerAttribute *attribute) {
  SpvmSampler *ret = (SpvmSampler *) malloc(sizeof(SpvmSampler));
  ret->info.magFilter = attribute->filterMode;
  ret->info.minFilter = attribute->filterMode;
  ret->info.mipmapMode = attribute->filterMode;
  ret->info.addressModeU = attribute->addressingMode;
  ret->info.addressModeV = attribute->addressingMode;
  ret->info.addressModeW = attribute->addressingMode;
  ret->info.mipLodBias = 0.f;
  ret->info.compareEnable = false;
  ret->info.compareOp = SPVM_COMPARE_OP_NEVER;
  ret->info.minLod = 0.f;
  ret->info.maxLod = kMaxSamplerLod;
  ret->info.borderColor = SPVM_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  ret->info.unnormalizedCoordinates = !attribute->normalized;
  return ret;
}

SpvmSampledImage *createSampledImage(SpvmImage *image, SpvmSampler *sampler) {
  SpvmSampledImage *ret = (SpvmSampledImage *) malloc(sizeof(SpvmSampledImage));
  ret->image = image;
  ret->sampler = sampler;
  return ret;
}

bool uploadImageData(SpvmImage *image, SpvmByte *dataPtr, SpvmWord dataSize,
                     SpvmWord width, SpvmWord height, SpvmWord depth,
                     SpvmWord mipLevel, SpvmWord arrayLayer) {
  if (mipLevel < image->info.baseMipLevel || mipLevel >= image->info.baseMipLevel + image->info.mipLevels) {
    LOGE("uploadImageData error: mipLevel invalid");
    return false;
  }

  if (arrayLayer < image->info.baseArrayLayer || arrayLayer >= image->info.baseArrayLayer + image->layerCount) {
    LOGE("uploadImageData error: arrayLayer invalid");
    return false;
  }

  SpvmImageLevel *level = &image->layers[arrayLayer].levels[mipLevel];
  level->width = width;
  level->height = height;
  level->depth = depth;
  level->dataSize = dataSize;

  free(level->data);
  level->data = (SpvmByte *) malloc(dataSize);
  memcpy(level->data, dataPtr, dataSize);

  return true;
}

void destroyImage(SpvmImage *image) {
  for (SpvmWord i = 0; i < image->layerCount; i++) {
    SpvmImageLayer *layer = &image->layers[i];
    for (SpvmWord j = 0; j < layer->levelCount; j++) {
      SpvmImageLevel *level = &layer->levels[j];
      if (level->data) {
        free(level->data);
        level->data = nullptr;
      }
    }
    free(layer->levels);
  }
  free(image->layers);
  free(image);
}

void destroySampler(SpvmSampler *sampler) {
  free(sampler);
}

void destroySampledImager(SpvmSampledImage *sampledImage) {
  free(sampledImage);
}

bool checkImageType(SpvmImage *image, SpvmTypeImage *type) {
  if (image->info.dim != type->dim) {
    LOGE("image dim not match");
    return false;
  }

  if (type->format != SpvImageFormatUnknown) {
    LOGE("image format not match");
    return false;
  }

  if (!image->info.mipmaps && image->info.mipLevels > 1) {
    LOGE("image no mipmaps but mipLevels > 1");
    return false;
  }

  if (image->info.samples == 1 && type->ms) {
    LOGE("image samples not match");
    return false;
  }

  if (image->info.arrayed != type->arrayed) {
    LOGE("image arrayed not match");
    return false;
  }

  if (!image->info.arrayed && image->info.arrayLayers > 1) {
    LOGE("image not arrayed but arrayLayers > 1");
    return false;
  }

  return true;
}

SpvmVec4 colorWithRGBAFloat(SpvmF32 r, SpvmF32 g, SpvmF32 b, SpvmF32 a) {
  SpvmVec4 ret;
  ret.elem[0].f32 = r;
  ret.elem[1].f32 = g;
  ret.elem[2].f32 = g;
  ret.elem[3].f32 = a;
  return ret;
}

SpvmVec4 colorWithRGBAInteger(SpvmI32 r, SpvmI32 g, SpvmI32 b, SpvmI32 a) {
  SpvmVec4 ret;
  ret.elem[0].i32 = r;
  ret.elem[1].i32 = g;
  ret.elem[2].i32 = g;
  ret.elem[3].i32 = a;
  return ret;
}

SpvmVec4 getBorderColorRGBA(SpvmBorderColor borderColor) {
  switch (borderColor) {
    case SPVM_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
      return colorWithRGBAFloat(0.f, 0.f, 0.f, 0.f);
    case SPVM_BORDER_COLOR_INT_TRANSPARENT_BLACK:
      return colorWithRGBAInteger(0, 0, 0, 0);
    case SPVM_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
      return colorWithRGBAFloat(0.f, 0.f, 0.f, 1.f);
    case SPVM_BORDER_COLOR_INT_OPAQUE_BLACK:
      return colorWithRGBAInteger(0, 0, 0, 1);
    case SPVM_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
      return colorWithRGBAFloat(1.f, 1.f, 1.f, 1.f);
    case SPVM_BORDER_COLOR_INT_OPAQUE_WHITE:
      return colorWithRGBAInteger(1, 1, 1, 1);
    default:
      break;
  }
  return {};
}

SpvmVec4 readPixelColorRGBA(SpvmFormat format, const SpvmByte *texelPtr) {
  SpvmVec4 texel;
  switch (format) {
    case SPVM_FORMAT_R8_UNORM: {
      texel.elem[0].f32 = (SpvmF32) texelPtr[0] / 255.f;
      texel.elem[1].f32 = 0.f;
      texel.elem[2].f32 = 0.f;
      texel.elem[3].f32 = 1.f;
      break;
    }
    case SPVM_FORMAT_R8G8_UNORM: {
      texel.elem[0].f32 = (SpvmF32) texelPtr[0] / 255.f;
      texel.elem[1].f32 = (SpvmF32) texelPtr[1] / 255.f;
      texel.elem[2].f32 = 0.f;
      texel.elem[3].f32 = 1.f;
      break;
    }
    case SPVM_FORMAT_R8G8B8_UNORM: {
      texel.elem[0].f32 = (SpvmF32) texelPtr[0] / 255.f;
      texel.elem[1].f32 = (SpvmF32) texelPtr[1] / 255.f;
      texel.elem[2].f32 = (SpvmF32) texelPtr[2] / 255.f;
      texel.elem[3].f32 = 1.f;
      break;
    }
    case SPVM_FORMAT_R8G8B8A8_UNORM: {
      texel.elem[0].f32 = (SpvmF32) texelPtr[0] / 255.f;
      texel.elem[1].f32 = (SpvmF32) texelPtr[1] / 255.f;
      texel.elem[2].f32 = (SpvmF32) texelPtr[2] / 255.f;
      texel.elem[3].f32 = (SpvmF32) texelPtr[3] / 255.f;
      break;
    }
    default:
      LOGE("image format not support");
      break;
  }

  return texel;
}

void writePixelColorRGBA(SpvmFormat format, SpvmByte *dstTexelPtr, SpvmVec4 srcTexel) {
  switch (format) {
    case SPVM_FORMAT_R8_UNORM: {
      dstTexelPtr[0] = (uint8_t) (srcTexel.elem[0].f32 * 255);
      break;
    }
    case SPVM_FORMAT_R8G8_UNORM: {
      dstTexelPtr[0] = (uint8_t) (srcTexel.elem[0].f32 * 255);
      dstTexelPtr[1] = (uint8_t) (srcTexel.elem[1].f32 * 255);
      break;
    }
    case SPVM_FORMAT_R8G8B8_UNORM: {
      dstTexelPtr[0] = (uint8_t) (srcTexel.elem[0].f32 * 255);
      dstTexelPtr[1] = (uint8_t) (srcTexel.elem[1].f32 * 255);
      dstTexelPtr[2] = (uint8_t) (srcTexel.elem[2].f32 * 255);
      break;
    }
    case SPVM_FORMAT_R8G8B8A8_UNORM: {
      dstTexelPtr[0] = (uint8_t) (srcTexel.elem[0].f32 * 255);
      dstTexelPtr[1] = (uint8_t) (srcTexel.elem[1].f32 * 255);
      dstTexelPtr[2] = (uint8_t) (srcTexel.elem[2].f32 * 255);
      dstTexelPtr[3] = (uint8_t) (srcTexel.elem[3].f32 * 255);
      break;
    }
    default:
      LOGE("image format not support");
      break;
  }
}

SpvmI32 getAddressMirror(SpvmI32 idx) {
  return idx >= 0 ? idx : -(1 + idx);
}

SpvmI32 getWrappingAddress(SpvmI32 idx, SpvmI32 size, SpvSamplerAddressingMode wrappingMode) {
  switch (wrappingMode) {
    case SpvSamplerAddressingModeClampToEdge:
      idx = sClamp(idx, 0, size - 1);
      break;
    case SpvSamplerAddressingModeClamp:
      idx = sClamp(idx, -1, size);
      break;
    case SpvSamplerAddressingModeRepeat:
      idx %= size;
      break;
    case SpvSamplerAddressingModeRepeatMirrored:
      idx = (size - 1) - getAddressMirror((idx % (2 * size)) - size);
      break;
    default:
      break;
  }

  return idx;
}

inline bool checkCoordinateValid(SpvmI32 i, SpvmI32 j, SpvmI32 k, SpvmImageLevel *imageLevel) {
  if (i < 0 || i >= imageLevel->width
      || j < 0 || j >= imageLevel->height
      || k < 0 || k >= imageLevel->depth) {
    return false;
  }
  return true;
}

SpvmWord getImageDimCoordinatesCount(SpvDim dim) {
  switch (dim) {
    case SpvDim1D:
      return 1;
    case SpvDim2D:
      return 2;
    case SpvDim3D:
    case SpvDimCube:
      return 3;
    default:
      LOGE("getImageDimCoordinatesCount, image dim not support");
      break;
  }
  return 0;
}

SpvmVec4 getCoordinatesFromValue(SpvmValue *coordinate, SpvmImageInfo *imageInfo, bool arrayed, bool proj) {
  SpvmVec4 uvwa{0, 0, 0, 0}; // if projection, a -> q
  if (coordinate->memberCnt == 0) {
    uvwa.elem[0].f32 = coordinate->value.f32;
  } else {
    SpvmWord coordCnt = getImageDimCoordinatesCount(imageInfo->dim);
    // u,v,w
    for (SpvmWord i = 0; i < coordCnt && i < coordinate->memberCnt; i++) {
      uvwa.elem[i].f32 = coordinate->value.members[i]->value.f32;
    }
    // q
    if (proj && coordinate->memberCnt > coordCnt) {
      SpvmF32 q = coordinate->value.members[coordCnt]->value.f32;
      uvwa.elem[3].f32 = q;
      for (SpvmWord i = 0; i < coordCnt; i++) {
        uvwa.elem[i].f32 /= q;
      }
    }
    // a
    if (arrayed && coordinate->memberCnt > coordCnt) {
      uvwa.elem[3].f32 = coordinate->value.members[coordCnt]->value.f32;
    }
  }

  return uvwa;
}

SpvmVecElement getDepthCompareResult(SpvmVecElement depth, SpvmVecElement dRef, SpvmCompareOp op) {
  bool pass = false;
  switch (op) {
    case SPVM_COMPARE_OP_NEVER:
      pass = false;
      break;
    case SPVM_COMPARE_OP_LESS:
      pass = depth.f32 < dRef.f32;
      break;
    case SPVM_COMPARE_OP_EQUAL:
      pass = fEqual(depth.f32, dRef.f32);
      break;
    case SPVM_COMPARE_OP_LESS_OR_EQUAL:
      pass = depth.f32 <= dRef.f32;
      break;
    case SPVM_COMPARE_OP_GREATER:
      pass = depth.f32 > dRef.f32;
      break;
    case SPVM_COMPARE_OP_NOT_EQUAL:
      pass = !fEqual(depth.f32, dRef.f32);
      break;
    case SPVM_COMPARE_OP_GREATER_OR_EQUAL:
      pass = depth.f32 >= dRef.f32;
      break;
    case SPVM_COMPARE_OP_ALWAYS:
      pass = true;
      break;
  }

  SpvmVecElement ret;
  ret.f32 = pass ? 1.f : 0.f;
  return ret;
}

SpvmByte *pixelDataPtr(SpvmImageLevel *imageLevel, SpvmI32 i, SpvmI32 j, SpvmI32 k, SpvmFormat imageFormat) {
  SpvmWord ptrOffset = k * (imageLevel->width * imageLevel->height)
      + j * imageLevel->width
      + i;
  return imageLevel->data + ptrOffset * getImageFormatBPP(imageFormat);
}

SpvmVec4 sampleImageLevel(SpvmImageLevel *imageLevel, SpvmImageCoord iIdx, SpvmImageCoord jIdx, SpvmImageCoord kIdx,
                          SpvmSamplerInfo *samplerInfo, SpvmFormat imageFormat) {
  if (samplerInfo) {
    iIdx.coord = getWrappingAddress(iIdx.coord, (SpvmI32) imageLevel->width, samplerInfo->addressModeU);
    jIdx.coord = getWrappingAddress(jIdx.coord, (SpvmI32) imageLevel->height, samplerInfo->addressModeV);
    kIdx.coord = getWrappingAddress(kIdx.coord, (SpvmI32) imageLevel->depth, samplerInfo->addressModeW);
  }

  SpvmVec4 texel;
  if (!checkCoordinateValid(iIdx.coord, jIdx.coord, kIdx.coord, imageLevel)) {
    texel = getBorderColorRGBA(samplerInfo ? samplerInfo->borderColor : (SpvmBorderColor) 0);
  } else {
    SpvmByte *texelPtr = pixelDataPtr(imageLevel, iIdx.coord, jIdx.coord, kIdx.coord, imageFormat);
    texel = readPixelColorRGBA(imageFormat, texelPtr);
  }

  texel = vec4FMul(texel, iIdx.weight * jIdx.weight * kIdx.weight);
  return texel;
}

SpvmVec4 sampleImageNearest(SpvmImageLevel *imageLevel,
                            SpvmVec4 uvwa,
                            SpvmImageInfo *imageInfo,
                            SpvmSamplerInfo *samplerInfo) {
  SpvmImageCoord iIdx{(SpvmI32) floor(uvwa.elem[0].f32), 1.f};
  SpvmImageCoord jIdx{(SpvmI32) floor(uvwa.elem[1].f32), 1.f};
  SpvmImageCoord kIdx{(SpvmI32) floor(uvwa.elem[2].f32), 1.f};

  return sampleImageLevel(imageLevel, iIdx, jIdx, kIdx, samplerInfo, imageInfo->format);
}

SpvmVec4 sampleImageLinear(SpvmImageLevel *imageLevel,
                           SpvmVec4 uvwa,
                           SpvmImageInfo *imageInfo,
                           SpvmSamplerInfo *samplerInfo) {
  SpvmImageCoord iIdx0{(SpvmI32) floor(uvwa.elem[0].f32 - 0.5f), 1.f};
  SpvmImageCoord jIdx0{(SpvmI32) floor(uvwa.elem[1].f32 - 0.5f), 1.f};
  SpvmImageCoord kIdx0{(SpvmI32) floor(uvwa.elem[2].f32 - 0.5f), 1.f};

  SpvmImageCoord iIdx1 = {iIdx0.coord + 1, fract(uvwa.elem[0].f32 - 0.5f)};
  SpvmImageCoord jIdx1 = {jIdx0.coord + 1, fract(uvwa.elem[1].f32 - 0.5f)};
  SpvmImageCoord kIdx1 = {kIdx0.coord + 1, fract(uvwa.elem[2].f32 - 0.5f)};

  iIdx0.weight = 1.f - iIdx1.weight;
  jIdx0.weight = 1.f - jIdx1.weight;
  kIdx0.weight = 1.f - kIdx1.weight;

#define ADD_IMAGE_SAMPLE_TEXEL(i, j, k) \
  retTexel = vec4FAdd(retTexel, sampleImageLevel(imageLevel, i, j, k, samplerInfo, imageInfo->format))

  SpvmVec4 retTexel{0, 0, 0, 0};
  switch (imageInfo->dim) {
    case SpvDim1D: {
      ADD_IMAGE_SAMPLE_TEXEL(iIdx0, ImageCoordNull, ImageCoordNull);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx1, ImageCoordNull, ImageCoordNull);
      break;
    }
    case SpvDim2D: {
      ADD_IMAGE_SAMPLE_TEXEL(iIdx0, jIdx0, ImageCoordNull);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx0, jIdx1, ImageCoordNull);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx1, jIdx0, ImageCoordNull);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx1, jIdx1, ImageCoordNull);
      break;
    }
    case SpvDim3D: {
      ADD_IMAGE_SAMPLE_TEXEL(iIdx0, jIdx0, kIdx0);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx0, jIdx0, kIdx1);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx0, jIdx1, kIdx0);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx0, jIdx1, kIdx1);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx1, jIdx0, kIdx0);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx1, jIdx0, kIdx1);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx1, jIdx1, kIdx0);
      ADD_IMAGE_SAMPLE_TEXEL(iIdx1, jIdx1, kIdx1);
      break;
    }
    default:
      LOGE("sample image error: dim not support");
      break;
  }

  return retTexel;
}

SpvmVec4 sampleImageFiltered(SpvmImageLevel *imageLevel,
                             SpvmVec4 uvwa,
                             SpvSamplerFilterMode filterMode,
                             SpvmImageInfo *imageInfo,
                             SpvmSamplerInfo *samplerInfo) {
  SpvmVec4 retTexel{0, 0, 0, 0};
  switch (filterMode) {
    case SpvSamplerFilterModeNearest: {
      retTexel = sampleImageNearest(imageLevel, uvwa, imageInfo, samplerInfo);
      break;
    }
    case SpvSamplerFilterModeLinear: {
      retTexel = sampleImageLinear(imageLevel, uvwa, imageInfo, samplerInfo);
      break;
    }
    default:
      break;
  }
  return retTexel;
}

SpvmVec4 sampleImageLayerLod(SpvmImageLayer *imageLayer,
                             SpvmWord level,
                             SpvmVec4 uvwa,
                             SpvSamplerFilterMode filterMode,
                             SpvmImageOperands *operands,
                             SpvmImageInfo *imageInfo,
                             SpvmSamplerInfo *samplerInfo) {
  SpvmImageLevel *imageLevel = &imageLayer->levels[level - imageInfo->baseMipLevel];

  if (imageLevel->width <= 0 || imageLevel->height <= 0 || imageLevel->data == nullptr) {
    LOGE("image level not uploaded with pixel data");
    return {0, 0, 0, 0};
  }

  // (s,t,r) -> (u,v,w)
  if (!samplerInfo->unnormalizedCoordinates) {
    uvwa.elem[0].f32 *= (SpvmF32) imageLevel->width;
    uvwa.elem[1].f32 *= (SpvmF32) imageLevel->height;
    uvwa.elem[2].f32 *= (SpvmF32) imageLevel->depth;
  }

  // add offset
  if (operands && operands->offset) {
    SpvmVec4 offset = readFromValue(operands->offset);
    for (SpvmWord i = 0; i < 4; i++) {
      uvwa.elem[i].f32 += (SpvmF32) offset.elem[i].i32;
    }
  }

  return sampleImageFiltered(imageLevel, uvwa, filterMode, imageInfo, samplerInfo);
}

void sampleMipmapLevel(SpvmImageLevel *dstLevel, SpvmImageLevel *srcLevel,
                       SpvmImageInfo *imageInfo, SpvSamplerFilterMode filterMode) {
  SpvmVec4 uvwa;
  SpvmF32 ratioW = (SpvmF32) srcLevel->width / (SpvmF32) dstLevel->width;
  SpvmF32 ratioH = (SpvmF32) srcLevel->height / (SpvmF32) dstLevel->height;
  SpvmF32 ratioD = (SpvmF32) srcLevel->depth / (SpvmF32) dstLevel->depth;

  for (SpvmI32 dd = 0; dd < dstLevel->depth; dd++) {
    for (SpvmI32 dh = 0; dh < dstLevel->height; dh++) {
      for (SpvmI32 dw = 0; dw < dstLevel->width; dw++) {
        uvwa.elem[0].f32 = (SpvmF32) dw * ratioW;
        uvwa.elem[1].f32 = (SpvmF32) dh * ratioH;
        uvwa.elem[2].f32 = (SpvmF32) dd * ratioD;
        SpvmVec4 srcTexel = sampleImageFiltered(srcLevel, uvwa, filterMode, imageInfo, nullptr);
        SpvmByte *dstTexelPtr = pixelDataPtr(dstLevel, dw, dh, dd, imageInfo->format);
        writePixelColorRGBA(imageInfo->format, dstTexelPtr, srcTexel);
      }
    }
  }
}

void generateMipmaps(SpvmImage *image, SpvSamplerFilterMode filterMode) {
  for (SpvmU32 layer = 0; layer < image->layerCount; layer++) {
    SpvmImageLayer *imageLayer = &image->layers[layer];
    SpvmImageLevel *srcLevel = &imageLayer->levels[0];   // based on first level
    if (srcLevel->dataSize <= 0 || srcLevel->data == nullptr) {
      LOGE("generateMipmaps error: levels[0] not upload with image data, layer index: %d", layer);
      continue;
    }
    for (SpvmU32 levelIdx = 1; levelIdx < imageLayer->levelCount; levelIdx++) {
      SpvmImageLevel *dstLevel = &imageLayer->levels[levelIdx];
      dstLevel->width = uMax(imageLayer->width >> dstLevel->level, 1);
      dstLevel->height = uMax(imageLayer->height >> dstLevel->level, 1);
      dstLevel->depth = uMax(imageLayer->depth >> dstLevel->level, 1);
      dstLevel->dataSize = dstLevel->width * dstLevel->height * dstLevel->depth * getImageFormatBPP(image->info.format);
      dstLevel->data = (SpvmByte *) malloc(dstLevel->dataSize);

      sampleMipmapLevel(dstLevel, srcLevel, &image->info, filterMode);
      srcLevel = dstLevel;
    }
  }
}

void generateMipmaps(SpvmSampledImage *sampledImage) {
  generateMipmaps(sampledImage->image, sampledImage->sampler->info.mipmapMode);
}

// calculate ρ_max
SpvmF32 getDerivativeRhoMax(void *ctx, SpvmWord coordinateId, SpvmImageOperands *operands, SpvmImageInfo *imageInfo) {
  SpvmVec4 mx{0, 0, 0, 0};
  SpvmVec4 my{0, 0, 0, 0};
  if (operands && operands->dx && operands->dy) {
    mx = readFromValue(operands->dx);
    my = readFromValue(operands->dy);
  } else {
    mx = getDPdx(ctx, coordinateId);
    my = getDPdy(ctx, coordinateId);
  }

  SpvmVec4 sizeInfo = {(SpvmI32) imageInfo->width, (SpvmI32) imageInfo->height, (SpvmI32) imageInfo->depth, 0};
  for (SpvmWord i = 0; i < 4; i++) {
    mx.elem[i].f32 *= (SpvmF32) sizeInfo.elem[i].i32;
    my.elem[i].f32 *= (SpvmF32) sizeInfo.elem[i].i32;
  }

  SpvmF32 rhoX = sqrt(vec4FDot(mx, mx));
  SpvmF32 rhoY = sqrt(vec4FDot(my, my));
  return fMax(rhoX, rhoY);
}

SpvmF32 getLodLambda(void *ctx,
                     SpvmImage *image,
                     SpvmSampler *sampler,
                     SpvmWord coordinateId,
                     SpvmImageOperands *operands) {
  SpvmF32 lambdaBase = 0.f;
  if (operands && operands->lod) {
    lambdaBase = operands->lod->value.f32;
  } else {
    lambdaBase = log2f(getDerivativeRhoMax(ctx, coordinateId, operands, &image->info));   // isotropic, η = 1
  }
  SpvmF32 samplerBias = sampler->info.mipLodBias;
  SpvmF32 shaderOpBias = (operands && operands->bias) ? operands->bias->value.f32 : 0.f;
  SpvmF32 samplerLodMin = sampler->info.minLod;
  SpvmF32 shaderOpLodMin = (operands && operands->minLod) ? operands->minLod->value.f32 : 0.f;
  SpvmF32 lodMin = fMax(samplerLodMin, shaderOpLodMin);
  SpvmF32 lodMax = sampler->info.maxLod;

  lambdaBase += fClamp(samplerBias + shaderOpBias, -kMaxSamplerLodBias, kMaxSamplerLodBias);
  return fClamp(lambdaBase, lodMin, lodMax);
}

SpvmF32 getComputeAccessedLod(SpvmImage *image, SpvmSampler *sampler, SpvmF32 lambda) {
  if (!image->info.mipmaps) {
    return 0.f;
  }

  SpvmF32 d = (SpvmF32) image->info.baseMipLevel + fClamp(lambda, 0, (SpvmF32) image->info.mipLevels - 1);
  if (sampler->info.mipmapMode == SpvSamplerFilterModeNearest) {
    return ceil(d + 0.5f) - 1.0f;
  }
  return d;
}

SpvmVec4 getImageSizeLod(SpvmImage *image, SpvmI32 lod) {
  SpvmVec4 ret{0, 0, 0, 0};
  SpvmU32 idx = 0;
  SpvmImageInfo *info = &image->info;
  if (lod < info->baseMipLevel || lod >= (info->baseMipLevel + info->mipLevels)) {
    LOGE("getImageSizeLod invalid lod");
    return ret;
  }
  SpvmImageLevel *imageLevel = &image->layers[0].levels[lod - info->baseMipLevel];
  switch (info->dim) {
    case SpvDim1D:
      ret.elem[idx++].u32 = imageLevel->width;
      break;
    case SpvDim2D:
    case SpvDimCube:
      ret.elem[idx++].u32 = imageLevel->width;
      ret.elem[idx++].u32 = imageLevel->height;
      break;
    case SpvDim3D:
      ret.elem[idx++].u32 = imageLevel->width;
      ret.elem[idx++].u32 = imageLevel->height;
      ret.elem[idx++].u32 = imageLevel->depth;
      break;
    default:
      LOGE("getImageSizeLod dim not support");
      break;
  }
  if (info->arrayed) {
    ret.elem[idx++].u32 = info->arrayLayers;
  }

  return ret;
}

SpvmVec4 getImageSize(SpvmImage *image) {
  SpvmVec4 ret{0, 0, 0, 0};
  SpvmU32 idx = 0;
  switch (image->info.dim) {
    case SpvDimBuffer:
      ret.elem[idx++].u32 = image->info.width;
      break;
    case SpvDim2D:
    case SpvDimRect:
      ret.elem[idx++].u32 = image->info.width;
      ret.elem[idx++].u32 = image->info.height;
      break;
    case SpvDim3D:
      ret.elem[idx++].u32 = image->info.width;
      ret.elem[idx++].u32 = image->info.height;
      ret.elem[idx++].u32 = image->info.depth;
      break;
    default:
      LOGE("getImageSize dim not support");
      break;
  }
  if (image->info.arrayed) {
    ret.elem[idx++].u32 = image->info.arrayLayers;
  }

  return ret;
}

void sampleImage(void *ctx,
                 SpvmValue *retValue,
                 SpvmValue *sampledImageValue,
                 SpvmValue *coordinateValue,
                 SpvmWord coordinateId,
                 SpvmImageOperands *operands,
                 SpvmValue *depthCompValue,
                 bool proj) {
  SpvmImage *image = (SpvmImage *) sampledImageValue->value.sampledImage->image->opaque;
  SpvmSampler *sampler = (SpvmSampler *) sampledImageValue->value.sampledImage->sampler->opaque;
  if (!image || !sampler) {
    LOGE("sampleImage error: image or sampler is null");
    return;
  }

  bool depthCompared = (depthCompValue && sampler->info.compareEnable);

  // get (u,v,w,a) / (s,t,r,a)
  SpvmVec4 uvwa = getCoordinatesFromValue(coordinateValue, &image->info, image->info.arrayed, proj);
  SpvmVecElement dRef;
  if (depthCompared) {
    dRef.f32 = depthCompValue->value.f32;
    if (!sampler->info.unnormalizedCoordinates) {
      dRef.f32 /= uvwa.elem[3].f32;  // Dref = Dref / q;
    }
  }

  // layer
  SpvmI32 layerIdx = (SpvmI32) image->info.baseArrayLayer;
  if (sampler->info.unnormalizedCoordinates) {
    layerIdx = sClamp((SpvmI32) roundEven(uvwa.elem[3].f32), (SpvmI32) image->info.baseArrayLayer,
                      (SpvmI32) (image->info.baseArrayLayer + image->info.arrayLayers - 1));
  }
  SpvmImageLayer *imageLayer = &image->layers[layerIdx - (SpvmI32) image->info.baseArrayLayer];
  SpvmVec4 retTexel;

  // lod & level
  SpvmF32 lodLambda = getLodLambda(ctx, image, sampler, coordinateId, operands);
  SpvmF32 lodLevel = getComputeAccessedLod(image, sampler, lodLambda);
  SpvSamplerFilterMode filterMode = (lodLambda <= 0) ? sampler->info.magFilter : sampler->info.minFilter;

  if (sampler->info.mipmapMode == SpvSamplerFilterModeNearest) {
    SpvmWord level = (SpvmWord) lodLevel;
    retTexel = sampleImageLayerLod(imageLayer, level, uvwa, filterMode, operands, &image->info, &sampler->info);
  } else {
    SpvmF32 levelHi = floor(lodLevel);
    SpvmF32 levelLo = fMin(levelHi + 1, image->info.baseMipLevel + image->info.mipLevels - 1);
    SpvmF32 delta = lodLevel - levelHi;
    SpvmVec4 texelHi = sampleImageLayerLod(imageLayer, (SpvmWord) levelHi, uvwa, filterMode, operands,
                                           &image->info, &sampler->info);
    SpvmVec4 texelLo = sampleImageLayerLod(imageLayer, (SpvmWord) levelLo, uvwa, filterMode, operands,
                                           &image->info, &sampler->info);
    retTexel = vec4FMul(texelHi, 1.f - delta);
    retTexel = vec4FAdd(retTexel, vec4FMul(texelLo, delta));
  }

  // depth compare
  if (depthCompared) {
    retTexel.elem[0] = getDepthCompareResult(retTexel.elem[0], dRef, sampler->info.compareOp);
  }

  // write texel value to retValue
  writeToValue(retValue, retTexel);
}

void fetchImage(void *ctx,
                SpvmValue *retValue,
                SpvmValue *imageValue,
                SpvmValue *coordinateValue,
                SpvmWord coordinateId,
                SpvmImageOperands *operands) {
  SpvmImage *image = (SpvmImage *) imageValue->value.image->opaque;
  if (!image) {
    LOGE("fetchImage error: image is null");
    return;
  }
  // uvwa
  SpvmVec4 uvwa = getCoordinatesFromValue(coordinateValue, &image->info, image->info.arrayed, false);
  // layer
  SpvmI32 layerIdx = sClamp((SpvmI32) uvwa.elem[3].i32, (SpvmI32) image->info.baseArrayLayer,
                            (SpvmI32) (image->info.baseArrayLayer + image->info.arrayLayers - 1));
  SpvmImageLayer *imageLayer = &image->layers[layerIdx];

  // level
  SpvmI32 level = image->info.baseMipLevel;
  if (operands && operands->lod) {
    level = sClamp(operands->lod->value.i32, image->info.baseMipLevel,
                   image->info.baseMipLevel + image->info.mipLevels - 1);
  }
  SpvmImageLevel *imageLevel = &imageLayer->levels[level - image->info.baseMipLevel];
  if (imageLevel->width <= 0 || imageLevel->height <= 0 || imageLevel->data == nullptr) {
    LOGE("image level not uploaded with pixel data");
    return;
  }

  // add offset
  if (operands && operands->offset) {
    SpvmVec4 offset = readFromValue(operands->offset);
    for (SpvmWord i = 0; i < 4; i++) {
      uvwa.elem[i].i32 += offset.elem[i].i32;
    }
  }

  SpvmByte *texelPtr = pixelDataPtr(imageLevel, uvwa.elem[0].i32, uvwa.elem[1].i32, uvwa.elem[2].i32,
                                    image->info.format);
  SpvmVec4 retTexel = readPixelColorRGBA(image->info.format, texelPtr);

  // write texel value to retValue
  writeToValue(retValue, retTexel);
}

void queryImageFormat(void *ctx, SpvmValue *retValue, SpvmValue *imageValue) {
  SpvmImage *image = (SpvmImage *) imageValue->value.image->opaque;
  if (!image) {
    LOGE("queryImageFormat error: image is null");
    return;
  }

  SpvImageChannelDataType ret = SpvImageChannelDataTypeMax;
  switch (image->info.format) {
    case SPVM_FORMAT_R8_UNORM:
    case SPVM_FORMAT_R8G8_UNORM:
    case SPVM_FORMAT_R8G8B8_UNORM:
    case SPVM_FORMAT_R8G8B8A8_UNORM:
      ret = SpvImageChannelDataTypeFloat;
      break;
    default:
      LOGE("queryImageFormat format not support");
      break;
  }
  retValue->value.i32 = ret;
}

void queryImageOrder(void *ctx, SpvmValue *retValue, SpvmValue *imageValue) {
  SpvmImage *image = (SpvmImage *) imageValue->value.image->opaque;
  if (!image) {
    LOGE("queryImageOrder error: image is null");
    return;
  }

  SpvImageChannelOrder ret = SpvImageChannelOrderMax;
  switch (image->info.format) {
    case SPVM_FORMAT_R8_UNORM:
      ret = SpvImageChannelOrderR;
      break;
    case SPVM_FORMAT_R8G8_UNORM:
      ret = SpvImageChannelOrderRG;
      break;
    case SPVM_FORMAT_R8G8B8_UNORM:
      ret = SpvImageChannelOrderRGB;
      break;
    case SPVM_FORMAT_R8G8B8A8_UNORM:
      ret = SpvImageChannelOrderRGBA;
      break;
    default:
      LOGE("queryImageOrder format not support");
      break;
  }
  retValue->value.i32 = ret;
}

void queryImageSizeLod(void *ctx, SpvmValue *retValue, SpvmValue *imageValue, SpvmValue *lodValue) {
  SpvmImage *image = (SpvmImage *) imageValue->value.image->opaque;
  if (!image) {
    LOGE("queryImageSizeLod error: image is null");
    return;
  }
  SpvmVec4 sizeInfo = getImageSizeLod(image, lodValue->value.i32);
  writeToValue(retValue, sizeInfo);
}

void queryImageSize(void *ctx, SpvmValue *retValue, SpvmValue *imageValue) {
  SpvmImage *image = (SpvmImage *) imageValue->value.image->opaque;
  if (!image) {
    LOGE("queryImageSize error: image is null");
    return;
  }
  SpvmVec4 sizeInfo = getImageSize(image);
  writeToValue(retValue, sizeInfo);
}

void queryImageLod(void *ctx,
                   SpvmValue *retValue,
                   SpvmValue *sampledImageValue,
                   SpvmValue *coordinateValue,
                   SpvmWord coordinateId) {
  SpvmImage *image = (SpvmImage *) sampledImageValue->value.sampledImage->image->opaque;
  SpvmSampler *sampler = (SpvmSampler *) sampledImageValue->value.sampledImage->sampler->opaque;
  if (!image || !sampler) {
    LOGE("queryImageLod error: image or sampler is null");
    return;
  }
  SpvmF32 lodLambda = getLodLambda(ctx, image, sampler, coordinateId, nullptr);
  SpvmF32 lodLevel = getComputeAccessedLod(image, sampler, lodLambda);
  retValue->value.members[0]->value.f32 = lodLambda;
  retValue->value.members[1]->value.f32 = lodLevel;
}

void queryImageLevels(void *ctx, SpvmValue *retValue, SpvmValue *imageValue) {
  SpvmImage *image = (SpvmImage *) imageValue->value.image->opaque;
  if (!image) {
    LOGE("queryImageLevels error: image is null");
    return;
  }
  retValue->value.u32 = image->info.mipLevels;
}

void queryImageSamples(void *ctx, SpvmValue *retValue, SpvmValue *imageValue) {
  SpvmImage *image = (SpvmImage *) imageValue->value.image->opaque;
  if (!image) {
    LOGE("queryImageSamples error: image is null");
    return;
  }
  retValue->value.u32 = image->info.samples;
}

}