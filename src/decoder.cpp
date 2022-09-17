/*
 * spvm
 * @author 	: keith@robot9.me
 *
 */

#include "decoder.h"
#include <cstdio>

#include "spvm.h"
#include "logger.h"

namespace SPVM {

typedef enum {
  I32_ENDIAN_UNKNOWN,
  I32_ENDIAN_LITTLE = 0x03020100ul,
  I32_ENDIAN_BIG = 0x00010203ul,
} I32_ENDIAN;

static const union {
  unsigned char bytes[4];
  SpvmU32 value;
} o32_host_order = {{0, 1, 2, 3}};

#define I32_ENDIAN_HOST (o32_host_order.value)

bool Decoder::checkEndian(const SpvmWord *ptr) {
  uint8_t bytes[4];
  memcpy(bytes, ptr, sizeof(SpvmU32));

  I32_ENDIAN endian = I32_ENDIAN_UNKNOWN;
  if (0x03 == bytes[0] && 0x02 == bytes[1] && 0x23 == bytes[2] && 0x07 == bytes[3]) {
    endian = I32_ENDIAN_LITTLE;
  }

  if (0x07 == bytes[0] && 0x23 == bytes[1] && 0x02 == bytes[2] && 0x03 == bytes[3]) {
    endian = I32_ENDIAN_BIG;
  }

  return I32_ENDIAN_HOST == endian;
}

bool Decoder::decodeFile(const char *path, SpvmModule *module) {
  if (path == nullptr) {
    LOGE("invalid file path");
    return false;
  }

  FILE *file = fopen(path, "rb");
  if (!file) {
    LOGE("error open file");
    return false;
  }

  fseek(file, 0, SEEK_END);
  SpvmWord fileLength = ftell(file);
  if (fileLength <= 0) {
    LOGE("error get file size");
    fclose(file);
    return false;
  }

  module->buffer = new SpvmByte[fileLength];
  rewind(file);
  fread(module->buffer, 1, fileLength, file);
  fclose(file);

  return decodeBytes(module->buffer, fileLength, module);
}

bool Decoder::decodeBytes(const SpvmByte *bytes, SpvmWord length, SpvmModule *module) {
  auto *ptr = (SpvmWord *) bytes;

  // check endian by magic number
  if (!checkEndian(ptr++)) {
    LOGE("binary endian not support");
    return false;
  }

  SpvmWord version = *ptr++;
  module->versionMajor = (version & 0x00FF0000) >> 16;
  module->versionMinor = (version & 0x0000FF00) >> 8;

  module->generatorMagic = *ptr++;
  module->bound = *ptr++;
  module->instructionSchema = *ptr++;

  module->code = ptr;
  module->codeSize = length - 5 * sizeof(SpvmWord);

  module->hasDerivativeOpcodes = false;
  module->inited = false;

  // check derivative opcodes
  while ((ptr - module->code) * sizeof(SpvmWord) < module->codeSize) {
    SpvmOpcode insOp = readOpcode(ptr);
    if (checkDerivativeOpcodes(insOp)) {
      module->hasDerivativeOpcodes = true;
      break;
    }
    ptr += insOp.wordCount;
  }

  return true;
}

bool Decoder::checkDerivativeOpcodes(SpvmOpcode opcode) {
  if (opcode.op == SpvOpDPdx || opcode.op == SpvOpDPdy || opcode.op == SpvOpFwidth
      || opcode.op == SpvOpDPdxFine || opcode.op == SpvOpDPdyFine || opcode.op == SpvOpFwidthFine
      || opcode.op == SpvOpDPdxCoarse || opcode.op == SpvOpDPdyCoarse || opcode.op == SpvOpFwidthCoarse) {
    return true;
  }
  return false;
}

}