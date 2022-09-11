/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "test.h"

typedef struct ImageContext_ {
  SPVM::SpvmImage *spvmImage;
  SPVM::SpvmSampler *spvmSampler;
  SPVM::SpvmSampledImage *spvmSampledImage;

  void create2d(int32_t width, int32_t height, int32_t mipLevels = 1) {
    SPVM::SpvmImageInfo imageInfo;
    imageInfo.dim = SpvDim2D;
    imageInfo.format = SPVM::SPVM_FORMAT_R8G8B8A8_UNORM;
    imageInfo.width = width;
    imageInfo.height = height;
    imageInfo.depth = 1;
    imageInfo.mipmaps = mipLevels > 1;
    imageInfo.baseMipLevel = 0;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayed = false;
    imageInfo.baseArrayLayer = 0;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = 1;

    spvmImage = SPVM::createImage(&imageInfo);
    spvmSampler = SPVM::createSampler();
    spvmSampledImage = SPVM::createSampledImage(spvmImage, spvmSampler);
  }

  void destroy() {
    SPVM::destroyImage(spvmImage);
    SPVM::destroySampler(spvmSampler);
    SPVM::destroySampledImager(spvmSampledImage);
  }
} ImageContext;

static uint8_t pixelBytes2D[] = {255, 0, 0, 255, 0, 255, 0, 255,
                                 0, 0, 255, 255, 0, 0, 0, 255,
                                 255, 0, 0, 255, 255, 0, 0, 255,
                                 255, 0, 0, 255, 255, 0, 0, 255,
                                 255, 0, 0, 255, 255, 0, 0, 255,
                                 255, 0, 0, 255, 255, 0, 0, 255,
                                 255, 0, 0, 255, 255, 0, 0, 255,
                                 255, 0, 0, 255, 255, 0, 0, 255,};

static uint8_t pixelBytes2D_mip[] = {128, 0, 0, 128, 0, 128, 0, 128,
                                     0, 0, 128, 255, 0, 0, 0, 128,};

TEST(TEST_SPV_EXEC, image_2d_query_frag) {
  TestContext ctx;
  ASSERT_TRUE(ctx.decode("assets/image.frag.spv"));
  ASSERT_TRUE(ctx.init());

  ImageContext imageCtx;
  imageCtx.create2d(4, 4, 2);
  imageCtx.spvmSampler->info.unnormalizedCoordinates = true;
  imageCtx.spvmSampler->info.mipmapMode = SpvSamplerFilterModeLinear;
  SPVM::uploadImageData(imageCtx.spvmImage, pixelBytes2D, sizeof(pixelBytes2D), 4, 4, 1, 0);
  SPVM::uploadImageData(imageCtx.spvmImage, pixelBytes2D_mip, sizeof(pixelBytes2D_mip), 2, 2, 1, 1);
  ctx.runtime.writeUniformBinding(imageCtx.spvmSampledImage, 0, 0, 0);

  SPVM::SpvmWord outQueryLodLoc = ctx.runtime.getLocationByName("outQueryLod");
  SPVM::SpvmWord outTexSizeLoc = ctx.runtime.getLocationByName("outTexSize");
  SPVM::SpvmWord outTexLevelLoc = ctx.runtime.getLocationByName("outTexLevel");

  // textureQueryLod
  float fragTexCoord[2]{0.f, 0.f};
  float outQueryLod[2];
  ctx.runtime.writeInput(fragTexCoord, 0);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outQueryLod, outQueryLodLoc);
  ASSERT_VEC2_EQ(outQueryLod, 0.f, 0.f);

  // textureSize
  int32_t inLod[1]{0};
  int32_t outTexSize[2];
  ctx.runtime.writeInput(inLod, 1);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outTexSize, outTexSizeLoc);
  ASSERT_VEC2_EQ(outTexSize, 4, 4);

  inLod[0] = 1;
  ctx.runtime.writeInput(inLod, 1);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outTexSize, outTexSizeLoc);
  ASSERT_VEC2_EQ(outTexSize, 2, 2);

  // textureQueryLevels
  int32_t outTexLevel[1];
  ctx.runtime.readOutput(&outTexLevel, outTexLevelLoc);
  ASSERT_EQ(outTexLevel[0], 2);

  imageCtx.destroy();
}

TEST(TEST_SPV_EXEC, image_2d_fetch_frag) {
  TestContext ctx;
  ASSERT_TRUE(ctx.decode("assets/image.frag.spv"));
  ASSERT_TRUE(ctx.init());

  ImageContext imageCtx;
  imageCtx.create2d(4, 4, 2);
  imageCtx.spvmSampler->info.unnormalizedCoordinates = true;
  imageCtx.spvmSampler->info.mipmapMode = SpvSamplerFilterModeLinear;
  SPVM::uploadImageData(imageCtx.spvmImage, pixelBytes2D, sizeof(pixelBytes2D), 4, 4, 1, 0);
  SPVM::uploadImageData(imageCtx.spvmImage, pixelBytes2D_mip, sizeof(pixelBytes2D_mip), 2, 2, 1, 1);
  ctx.runtime.writeUniformBinding(imageCtx.spvmSampledImage, 0, 0, 0);

  SPVM::SpvmWord outColorFetchLoc = ctx.runtime.getLocationByName("outColorFetch");
  SPVM::SpvmWord outColorFetchOffsetLoc = ctx.runtime.getLocationByName("outColorFetchOffset");

  int32_t inLod[1]{1};
  int32_t fragTexCoord[2]{0, 0};

  // texelFetch
  ctx.runtime.writeInput(fragTexCoord, 0);
  ctx.runtime.writeInput(inLod, 1);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  float outColorFetch[4];
  ctx.runtime.readOutput(&outColorFetch, outColorFetchLoc);
  ASSERT_VEC4_EQ(outColorFetch, 128.f / 255.f, 0.f, 0.f, 128.f / 255.f);

  // texelFetchOffset
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  float outColorFetchOffset[4];
  ctx.runtime.readOutput(&outColorFetchOffset, outColorFetchOffsetLoc);
  ASSERT_VEC4_EQ(outColorFetchOffset, 0.f, 0.f, 0.f, 128.f / 255.f);

  imageCtx.destroy();
}

TEST(TEST_SPV_EXEC, image_2d_lookup_frag) {
  TestContext ctx;
  ASSERT_TRUE(ctx.decode("assets/image.frag.spv"));
  ASSERT_TRUE(ctx.init());

  ImageContext imageCtx;
  imageCtx.create2d(4, 4, 2);
  imageCtx.spvmSampler->info.unnormalizedCoordinates = true;
  imageCtx.spvmSampler->info.mipmapMode = SpvSamplerFilterModeLinear;
  SPVM::uploadImageData(imageCtx.spvmImage, pixelBytes2D, sizeof(pixelBytes2D), 4, 4, 1, 0);
  SPVM::uploadImageData(imageCtx.spvmImage, pixelBytes2D_mip, sizeof(pixelBytes2D_mip), 2, 2, 1, 1);
  ctx.runtime.writeUniformBinding(imageCtx.spvmSampledImage, 0, 0, 0);

  float fragTexCoord[2]{0.f, 0.f};
  SPVM::SpvmWord outLoc = ctx.runtime.getLocationByName("outColorSample");

  // texture
  float outColorSample[4];
  ctx.runtime.writeInput(fragTexCoord, 0);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColorSample, outLoc);
  ASSERT_VEC4_EQ(outColorSample, 1.f, 0.f, 0.f, 1.f);

  fragTexCoord[0] = 1.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColorSample, outLoc);
  ASSERT_VEC4_EQ(outColorSample, 0.f, 1.f, 0.f, 1.f);

  fragTexCoord[0] = 5.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  imageCtx.spvmSampler->info.addressModeU = SpvSamplerAddressingModeClampToEdge;
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColorSample, outLoc);
  ASSERT_VEC4_EQ(outColorSample, 0.f, 0.f, 0.f, 1.f);

  fragTexCoord[0] = 5.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  imageCtx.spvmSampler->info.addressModeU = SpvSamplerAddressingModeClamp;
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColorSample, outLoc);
  ASSERT_VEC4_EQ(outColorSample, 0.f, 0.f, 0.f, 0.f);

  fragTexCoord[0] = 5.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  imageCtx.spvmSampler->info.addressModeU = SpvSamplerAddressingModeRepeat;
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColorSample, outLoc);
  ASSERT_VEC4_EQ(outColorSample, 0.f, 1.f, 0.f, 1.f);

  fragTexCoord[0] = 5.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  imageCtx.spvmSampler->info.addressModeU = SpvSamplerAddressingModeRepeatMirrored;
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColorSample, outLoc);
  ASSERT_VEC4_EQ(outColorSample, 0.f, 0.f, 1.f, 1.f);

  // textureGrad TODO

  // textureLod
  int32_t inLod[1]{1};
  fragTexCoord[0] = 0.f;
  fragTexCoord[1] = 0.f;
  ctx.runtime.writeInput(inLod, 1);
  ctx.runtime.writeInput(fragTexCoord, 0);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  outLoc = ctx.runtime.getLocationByName("outColorSampleLod");
  float outColorSampleLod[4];
  ctx.runtime.readOutput(&outColorSampleLod, outLoc);
  ASSERT_VEC4_EQ(outColorSampleLod, 128.f / 255.f, 0.f, 0.f, 128.f / 255.f);

  // textureOffset
  fragTexCoord[0] = 0.f;
  fragTexCoord[1] = 0.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  outLoc = ctx.runtime.getLocationByName("outColorSampleOffset");
  float outColorSampleOffset[4];
  ctx.runtime.readOutput(&outColorSampleOffset, outLoc);
  ASSERT_VEC4_EQ(outColorSampleOffset, 0.f, 1.f, 0.f, 1.f);

  imageCtx.destroy();
}