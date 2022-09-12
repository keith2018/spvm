/*
 * spvm-shadertoy
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

namespace SPVM {
namespace ShaderToy {

const std::string ASSETS_DIR = "assets/shaders/";
const std::string SHADER_WRAPPER_PATH = "assets/wrapper.frag";

class Settings {
 public:
  Settings() {
    shaderPaths_["basic"] = ASSETS_DIR + "basic.frag";
    shaderPaths_["fstyD4"] = ASSETS_DIR + "fstyD4.frag";
    shaderPaths_["ftdfWn"] = ASSETS_DIR + "ftdfWn.frag";
    shaderPaths_["Nt3BzM"] = ASSETS_DIR + "Nt3BzM.frag";
    shaderPaths_["XdlSDs"] = ASSETS_DIR + "XdlSDs.frag";

    shaderPathKey_ = shaderPaths_.begin()->first;
  }

  inline std::string getShaderPath() {
    auto it = shaderPaths_.find(shaderPathKey_);
    return it == shaderPaths_.end() ? "" : it->second;
  }

 public:
  std::string shaderWrapperPath_ = SHADER_WRAPPER_PATH;
  std::function<void(std::string)> loadShaderCallback_;

  std::string shaderPathKey_;
  std::unordered_map<std::string, std::string> shaderPaths_;
};

}
}
