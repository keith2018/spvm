/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "test.h"

TEST(TEST_SPV_EXEC, location_frag) {
  TestContext ctx;
  ASSERT_TRUE(ctx.decode("assets/location.frag.spv"));
  ASSERT_TRUE(ctx.init());

  SPVM::SpvmWord inColorIdx = ctx.runtime.getLocationByName("inColor");
  SPVM::SpvmWord outColorIdx = ctx.runtime.getLocationByName("outFragColor");

  ASSERT_EQ(inColorIdx, 0);
  ASSERT_EQ(outColorIdx, 0);

  float inColor[3]{0.2f, 0.3f, 0.4f};
  float outFragColor[4];

  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(outFragColor, outColorIdx);
  ASSERT_FLOAT_VEC4_EQ(outFragColor, 0.f, 0.f, 0.f, 1.f)

  ctx.runtime.writeInput(inColor, inColorIdx);
  ASSERT_TRUE(ctx.runtime.execEntryPoint());
  ctx.runtime.readOutput(outFragColor, outColorIdx);
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

  SPVM::SpvmWord vPositionIdx = ctx.runtime.getLocationByName("vPosition");
  SPVM::SpvmUniformBinding cameraDataIdx = ctx.runtime.getBindingByName("cameraData");

  ASSERT_EQ(vPositionIdx, 0);
  ASSERT_EQ(cameraDataIdx.binding, 0);
  ASSERT_EQ(cameraDataIdx.set, 0);

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

  ctx.runtime.writeInput(vPosition, vPositionIdx);
  ctx.runtime.writeUniformBinding(&cameraData, sizeof(cameraData),
                                  cameraDataIdx.binding, cameraDataIdx.set);
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

TEST_GLSL_SPV(control_flow_frag, "assets/control_flow.frag.spv")
TEST_GLSL_SPV(arithmetic_0_frag, "assets/arithmetic_0.frag.spv")
TEST_GLSL_SPV(arithmetic_1_frag, "assets/arithmetic_1.frag.spv")
TEST_GLSL_SPV(function_frag, "assets/function.frag.spv")
TEST_GLSL_SPV(conversion_frag, "assets/conversion.frag.spv")
TEST_GLSL_SPV(composite_frag, "assets/composite.frag.spv")
TEST_GLSL_SPV(bit_frag, "assets/bit.frag.spv")
TEST_GLSL_SPV(relational_logical, "assets/relational_logical.frag.spv")
