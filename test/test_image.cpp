/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "test.h"

TEST(TEST_SPV_EXEC, image_2d_frag) {
  TestContext ctx;
  ASSERT_TRUE(ctx.decode("assets/image.frag.spv"));
  ASSERT_TRUE(ctx.init());

  SPVM::SpvmImageInfo imageInfo;
  imageInfo.dim = SpvDim2D;
  imageInfo.format = SPVM::SPVM_FORMAT_R8G8B8A8_UNORM;
  imageInfo.width = 4;
  imageInfo.height = 4;
  imageInfo.depth = 1;
  imageInfo.baseMipLevel = 0;
  imageInfo.mipLevels = 1;
  imageInfo.baseArrayLayer = 0;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = 1;
  auto spvmImage = SPVM::createImage(&imageInfo);
  uint8_t pixelBytes[] = {255, 0, 0, 255, 0, 255, 0, 255,
                          0, 0, 255, 255, 0, 0, 0, 255,
                          255, 0, 0, 255, 255, 0, 0, 255,
                          255, 0, 0, 255, 255, 0, 0, 255,
                          255, 0, 0, 255, 255, 0, 0, 255,
                          255, 0, 0, 255, 255, 0, 0, 255,
                          255, 0, 0, 255, 255, 0, 0, 255,
                          255, 0, 0, 255, 255, 0, 0, 255,};
  SPVM::uploadImageData(spvmImage, pixelBytes, sizeof(pixelBytes), 4, 4, 1);

  auto spvmSampler = SPVM::createSampler();
  spvmSampler->info.unnormalizedCoordinates = true;

  auto spvmSampledImage = SPVM::createSampledImage(spvmImage, spvmSampler);
  ctx.runtime.writeUniformBinding(spvmSampledImage, 0, 0, 0);

  float fragTexCoord[2]{0.f, 0.f};
  float outColor[4];
  ctx.runtime.writeInput(fragTexCoord, 0);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColor, 0);
  ASSERT_VEC4_EQ(outColor, 1.f, 0.f, 0.f, 1.f);

  fragTexCoord[0] = 1.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColor, 0);
  ASSERT_VEC4_EQ(outColor, 0.f, 1.f, 0.f, 1.f);

  fragTexCoord[0] = 5.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  spvmSampler->info.addressModeU = SpvSamplerAddressingModeClampToEdge;
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColor, 0);
  ASSERT_VEC4_EQ(outColor, 0.f, 0.f, 0.f, 1.f);

  fragTexCoord[0] = 5.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  spvmSampler->info.addressModeU = SpvSamplerAddressingModeClamp;
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColor, 0);
  ASSERT_VEC4_EQ(outColor, 0.f, 0.f, 0.f, 0.f);

  fragTexCoord[0] = 5.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  spvmSampler->info.addressModeU = SpvSamplerAddressingModeRepeat;
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColor, 0);
  ASSERT_VEC4_EQ(outColor, 0.f, 1.f, 0.f, 1.f);

  fragTexCoord[0] = 5.f;
  ctx.runtime.writeInput(fragTexCoord, 0);
  spvmSampler->info.addressModeU = SpvSamplerAddressingModeRepeatMirrored;
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(&outColor, 0);
  ASSERT_VEC4_EQ(outColor, 0.f, 0.f, 1.f, 1.f);

  SPVM::destroyImage(spvmImage);
  SPVM::destroySampler(spvmSampler);
  SPVM::destroySampledImager(spvmSampledImage);
}