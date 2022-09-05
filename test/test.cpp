/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "gtest/gtest.h"

#include "decoder.h"
#include "runtime.h"
#include "image.h"

#define HEAP_SIZE 128 * 1024

typedef struct TestContext_ {
  SPVM::SpvmModule module;
  SPVM::Runtime runtime;

  bool decode(const char *path) {
    return SPVM::Decoder::decodeFile(path, &module);
  }

  bool init() {
    return runtime.initWithModule(&module, HEAP_SIZE);
  }

} TestContext;

#define ASSERT_VEC3_EQ(vec, v0, v1, v2)             \
  ASSERT_EQ((vec)[0], v0);                          \
  ASSERT_EQ((vec)[1], v1);                          \
  ASSERT_EQ((vec)[2], v2);                          \

#define ASSERT_VEC4_EQ(vec, v0, v1, v2, v3)         \
  ASSERT_EQ((vec)[0], v0);                          \
  ASSERT_EQ((vec)[1], v1);                          \
  ASSERT_EQ((vec)[2], v2);                          \
  ASSERT_EQ((vec)[3], v3);                          \

#define ASSERT_FLOAT_VEC3_EQ(vec, v0, v1, v2)       \
  ASSERT_FLOAT_EQ((vec)[0], v0);                    \
  ASSERT_FLOAT_EQ((vec)[1], v1);                    \
  ASSERT_FLOAT_EQ((vec)[2], v2);                    \

#define ASSERT_FLOAT_VEC4_EQ(vec, v0, v1, v2, v3)   \
  ASSERT_FLOAT_EQ((vec)[0], v0);                    \
  ASSERT_FLOAT_EQ((vec)[1], v1);                    \
  ASSERT_FLOAT_EQ((vec)[2], v2);                    \
  ASSERT_FLOAT_EQ((vec)[3], v3);                    \


#define TEST_GLSL_SPV(name, path)                   \
  TEST(TEST_SPV_EXEC, name) {                       \
    TestContext ctx;                                \
    ASSERT_TRUE(ctx.decode(path));                  \
    ASSERT_TRUE(ctx.init());                        \
    ASSERT_TRUE(ctx.runtime.execEntryPoint());      \
    int outAssert = -1;                             \
    ctx.runtime.readOutput(&outAssert, 0);          \
    ASSERT_FLOAT_EQ(outAssert, -1);                 \
  }                                                 \


TEST(TEST_SPV_EXEC, location_frag) {
  TestContext ctx;
  ASSERT_TRUE(ctx.decode("assets/location.frag.spv"));
  ASSERT_TRUE(ctx.init());

  float inColor[3]{0.2f, 0.3f, 0.4f};
  float outFragColor[4];

  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(outFragColor, 0);
  ASSERT_FLOAT_VEC4_EQ(outFragColor, 0.f, 0.f, 0.f, 1.f)

  ctx.runtime.writeInput(inColor, 0);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(outFragColor, 0);
  ASSERT_FLOAT_VEC4_EQ(outFragColor, 0.3f, 0.2f, 0.4f, 1.f)
}

TEST(TEST_SPV_EXEC, built_in_vert) {
  TestContext ctx;
  ASSERT_TRUE(ctx.decode("assets/built_in.vert.spv"));
  ASSERT_TRUE(ctx.init());

  int inVertexIndex = 0;
  float outBuiltInPosition[4];
  float outFragColor[3];

  // index 0
  inVertexIndex = 0;
  ctx.runtime.writeInputBuiltIn(&inVertexIndex, SpvBuiltInVertexIndex);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutputBuiltIn(outBuiltInPosition, SpvBuiltInPosition);
  ctx.runtime.readOutput(outFragColor, 0);
  ASSERT_FLOAT_VEC4_EQ(outBuiltInPosition, 1.f, 1.f, 0.f, 1.f)
  ASSERT_FLOAT_VEC3_EQ(outFragColor, 1.f, 0.f, 0.f)

  // index 1
  inVertexIndex = 1;
  ctx.runtime.writeInputBuiltIn(&inVertexIndex, SpvBuiltInVertexIndex);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutputBuiltIn(outBuiltInPosition, SpvBuiltInPosition);
  ctx.runtime.readOutput(outFragColor, 0);
  ASSERT_FLOAT_VEC4_EQ(outBuiltInPosition, -1.f, 1.f, 0.f, 1.f)
  ASSERT_FLOAT_VEC3_EQ(outFragColor, 0.f, 1.f, 0.f)

  // index 2
  inVertexIndex = 2;
  ctx.runtime.writeInputBuiltIn(&inVertexIndex, SpvBuiltInVertexIndex);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutputBuiltIn(outBuiltInPosition, SpvBuiltInPosition);
  ctx.runtime.readOutput(outFragColor, 0);
  ASSERT_FLOAT_VEC4_EQ(outBuiltInPosition, 0.f, -1.f, 0.f, 1.f)
  ASSERT_FLOAT_VEC3_EQ(outFragColor, 0.f, 0.f, 1.f)
}

TEST(TEST_SPV_EXEC, uniform_block_vert) {
  TestContext ctx;
  ASSERT_TRUE(ctx.decode("assets/uniform_block.vert.spv"));
  ASSERT_TRUE(ctx.init());

  float vPosition[3]{0.2f, 0.3f, 0.4f};
  struct {
    float data[4] = {0.f, 0.f, 0.f, 0.f};
    float viewProj[16] = {
        1.f, 1.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
  } cameraData;
  struct {
    float data[4] = {0.f, 0.f, 0.f, 0.f};
    float renderMatrix[16] = {
        1.f, 2.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
  } pushConstants;
  float outBuiltInPosition[4];

  ctx.runtime.writeInput(vPosition, 0);
  ctx.runtime.writeUniformBinding(&cameraData, sizeof(cameraData), 0, 0);
  ctx.runtime.writeUniformPushConstants(&pushConstants, sizeof(pushConstants));
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutputBuiltIn(outBuiltInPosition, SpvBuiltInPosition);
  ASSERT_FLOAT_VEC4_EQ(outBuiltInPosition, 0.2f, 0.9f, 0.4f, 1.f)
}

TEST(TEST_SPV_EXEC, array_frag) {
  TestContext ctx;
  ASSERT_TRUE(ctx.decode("assets/array.frag.spv"));
  ASSERT_TRUE(ctx.init());

  struct {
    float a[3] = {4.2f, 5.0f, 5.2f};
    float b[6] = {1.2f, 1.0f, 1.3f, 0.2f, 0.3f, 0.5f};
  } tb;
  ctx.runtime.writeUniformBinding(&tb, sizeof(tb), 0, 0);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());

  int outAssert = -1;
  ctx.runtime.readOutput(&outAssert, 0);
  ASSERT_FLOAT_EQ(outAssert, -1);
}

TEST(TEST_SPV_EXEC, image_frag) {
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

TEST_GLSL_SPV(control_flow_frag, "assets/control_flow.frag.spv")
TEST_GLSL_SPV(arithmetic_0_frag, "assets/arithmetic_0.frag.spv")
TEST_GLSL_SPV(arithmetic_1_frag, "assets/arithmetic_1.frag.spv")
TEST_GLSL_SPV(function_frag, "assets/function.frag.spv")
TEST_GLSL_SPV(conversion_frag, "assets/conversion.frag.spv")
TEST_GLSL_SPV(composite_frag, "assets/composite.frag.spv")
TEST_GLSL_SPV(bit_frag, "assets/bit.frag.spv")
TEST_GLSL_SPV(relational_logical, "assets/relational_logical.frag.spv")

TEST_GLSL_SPV(glsl_std_450_0, "assets/glsl_std_450_0.frag.spv")
TEST_GLSL_SPV(glsl_std_450_1, "assets/glsl_std_450_1.frag.spv")
TEST_GLSL_SPV(glsl_std_450_2, "assets/glsl_std_450_2.frag.spv")
