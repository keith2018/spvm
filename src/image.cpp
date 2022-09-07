/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "image.h"
#include "logger.h"
#include "utils.h"

namespace SPVM {

// image coordinates(i,j,k) with sample weight
typedef struct SpvmImageCoord_ {
  SpvmI32 coord;
  SpvmF32 weight;
} SpvmImageCoord;

#define ImageCoordNull {0, 1.f}

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
    ret->info.maxLod = 0.f;
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
  ret->info.maxLod = 0.f;
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

  if (image->info.samples == 1 && type->ms) {
    LOGE("image samples not match");
    return false;
  }

  if (image->info.arrayLayers == 1 && type->arrayed) {
    LOGE("image arrayLayers not match");
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

SpvmWord getImagePixelBytesSize(SpvmFormat format) {
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

SpvmVec4 getImageColorRGBA(SpvmFormat format, const SpvmByte *texelPtr) {
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
    if (arrayed && coordinate->memberCnt > coordCnt) {
      SpvmF32 q = coordinate->value.members[coordCnt]->value.f32;
      uvwa.elem[3].f32 = q;
      for (SpvmWord i = 0; i < coordCnt; i++) {
        uvwa.elem[i].f32 /= q;
      }
    }
    // a
    if (proj && coordinate->memberCnt > coordCnt) {
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

SpvmVec4 sampleImageLevel(SpvmImageLevel *imageLevel, SpvmImageCoord iIdx, SpvmImageCoord jIdx, SpvmImageCoord kIdx,
                          SpvmSamplerInfo *samplerInfo, SpvmFormat imageFormat) {
  iIdx.coord = getWrappingAddress(iIdx.coord, (SpvmI32) imageLevel->width, samplerInfo->addressModeU);
  jIdx.coord = getWrappingAddress(jIdx.coord, (SpvmI32) imageLevel->height, samplerInfo->addressModeV);
  kIdx.coord = getWrappingAddress(kIdx.coord, (SpvmI32) imageLevel->depth, samplerInfo->addressModeW);

  SpvmVec4 texel;
  if (!checkCoordinateValid(iIdx.coord, jIdx.coord, kIdx.coord, imageLevel)) {
    texel = getBorderColorRGBA(samplerInfo->borderColor);
  } else {
    SpvmWord ptrOffset = kIdx.coord * (imageLevel->width * imageLevel->height)
        + jIdx.coord * imageLevel->width
        + iIdx.coord;
    SpvmByte *texelPtr = imageLevel->data + ptrOffset * getImagePixelBytesSize(imageFormat);
    texel = getImageColorRGBA(imageFormat, texelPtr);
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
    case SpvDimCube: {
      // TODO
      break;
    }
    default:
      break;
  }

  return retTexel;
}

SpvmVec4 sampleImageFiltered(SpvmImageLevel *imageLevel,
                             SpvmVec4 uvwa,
                             SpvmF32 lodLambda,
                             SpvmImageInfo *imageInfo,
                             SpvmSamplerInfo *samplerInfo) {
  SpvmVec4 retTexel{0, 0, 0, 0};
  SpvSamplerFilterMode filterMode = (lodLambda <= 0) ? samplerInfo->magFilter : samplerInfo->minFilter;
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

SpvmWord getLodLevel(SpvmValue *sampledImage, SpvmImageOperands *operands) {
  // TODO
  return 0;
}

void sampleImage(SpvmValue *retValue,
                 SpvmValue *sampledImage,
                 SpvmValue *coordinate,
                 SpvmImageOperands *operands,
                 SpvmValue *depthCompValue,
                 bool proj) {
  SpvmImage *image = (SpvmImage *) sampledImage->value.sampledImage->image->opaque;
  SpvmSampler *sampler = (SpvmSampler *) sampledImage->value.sampledImage->sampler->opaque;
  if (!image || !sampler) {
    return;
  }

  bool arrayed = sampledImage->value.sampledImage->image->type->arrayed;
  bool depthCompared = (depthCompValue && sampler->info.compareEnable);

  // get (u,v,w,a) / (s,t,r,a)
  SpvmVec4 uvwa = getCoordinatesFromValue(coordinate, &image->info, arrayed, proj);
  SpvmVecElement dRef;
  if (depthCompared) {
    dRef.f32 = depthCompValue->value.f32;
  }

  // layer
  SpvmI32 layerIdx = (SpvmI32) image->info.baseArrayLayer
      + sClamp((SpvmI32) roundEven(uvwa.elem[3].f32), 0, (SpvmI32) (image->info.arrayLayers - 1));

  // level
  SpvmWord level = getLodLevel(sampledImage, operands);
  SpvmImageLevel *imageLevel = &image->layers[layerIdx].levels[level];

  // (s,t,r) -> (u,v,w)
  if (!sampler->info.unnormalizedCoordinates) {
    uvwa.elem[0].f32 *= (SpvmF32) imageLevel->width;
    uvwa.elem[1].f32 *= (SpvmF32) imageLevel->height;
    uvwa.elem[2].f32 *= (SpvmF32) imageLevel->depth;
    if (depthCompared) {
      dRef.f32 /= uvwa.elem[3].f32;  // Dref = Dref / q;
    }
  }

  // TODO offset

  // filter
  SpvmF32 lodLambda = 0.f;
  SpvmVec4 retTexel = sampleImageFiltered(imageLevel, uvwa, lodLambda, &image->info, &sampler->info);

  // depth compare
  if (depthCompared) {
    retTexel.elem[0] = getDepthCompareResult(retTexel.elem[0], dRef, sampler->info.compareOp);
  }

  // write texel value to retValue
  if (retValue->memberCnt == 0) {
    retValue->value.f32 = retTexel.elem[0].f32;
  } else {
    for (SpvmWord i = 0; i < retValue->memberCnt && i < 4; i++) {
      retValue->value.members[i]->value.f32 = retTexel.elem[i].f32;
    }
  }
}

}