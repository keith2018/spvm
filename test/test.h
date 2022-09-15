/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include "gtest/gtest.h"

#include "decoder.h"
#include "runtime.h"
#include "quad.h"
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

typedef struct TestQuadContext_ {
  SPVM::SpvmModule module;
  SPVM::RuntimeQuadContext quadContext;

  bool decode(const char *path) {
    return SPVM::Decoder::decodeFile(path, &module);
  }

  bool init() {
    return quadContext.initWithModule(&module, HEAP_SIZE);
  }

} TestQuadContext;

#define ASSERT_VEC2_EQ(vec, v0, v1)                 \
  ASSERT_EQ((vec)[0], v0);                          \
  ASSERT_EQ((vec)[1], v1);

#define ASSERT_VEC3_EQ(vec, v0, v1, v2)             \
  ASSERT_EQ((vec)[0], v0);                          \
  ASSERT_EQ((vec)[1], v1);                          \
  ASSERT_EQ((vec)[2], v2);

#define ASSERT_VEC4_EQ(vec, v0, v1, v2, v3)         \
  ASSERT_EQ((vec)[0], v0);                          \
  ASSERT_EQ((vec)[1], v1);                          \
  ASSERT_EQ((vec)[2], v2);                          \
  ASSERT_EQ((vec)[3], v3);

#define ASSERT_FLOAT_VEC3_EQ(vec, v0, v1, v2)       \
  ASSERT_FLOAT_EQ((vec)[0], v0);                    \
  ASSERT_FLOAT_EQ((vec)[1], v1);                    \
  ASSERT_FLOAT_EQ((vec)[2], v2);

#define ASSERT_FLOAT_VEC4_EQ(vec, v0, v1, v2, v3)   \
  ASSERT_FLOAT_EQ((vec)[0], v0);                    \
  ASSERT_FLOAT_EQ((vec)[1], v1);                    \
  ASSERT_FLOAT_EQ((vec)[2], v2);                    \
  ASSERT_FLOAT_EQ((vec)[3], v3);


#define TEST_GLSL_SPV(name, path)                   \
  TEST(TEST_SPV_EXEC, name) {                       \
    TestContext ctx;                                \
    ASSERT_TRUE(ctx.decode(path));                  \
    ASSERT_TRUE(ctx.init());                        \
    ASSERT_TRUE(ctx.runtime.execEntryPoint());      \
    int outAssert = -1;                             \
    ctx.runtime.readOutput(&outAssert, 0);          \
    ASSERT_FLOAT_EQ(outAssert, -1);                 \
  }
