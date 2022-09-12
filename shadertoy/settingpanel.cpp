/*
 * spvm-shadertoy
 * @author 	: keith@robot9.me
 *
 */

#include "settingpanel.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

namespace SPVM {
namespace ShaderToy {

void SettingPanel::init(void *window, int width, int height) {
  frameWidth_ = width;
  frameHeight_ = height;

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  ImGuiStyle *style = &ImGui::GetStyle();
  style->Alpha = 0.8f;

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *) window, true);
  ImGui_ImplOpenGL3_Init("#version 150");
}

void SettingPanel::onDraw() {
  if (!visible) {
    return;
  }

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Settings window
  ImGui::Begin("Settings", nullptr,
               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                   | ImGuiWindowFlags_AlwaysAutoResize);
  drawSettings();
  ImGui::SetWindowPos(ImVec2(frameWidth_ - ImGui::GetWindowWidth(), 0));
  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SettingPanel::drawSettings() {
  // fps
  ImGui::Separator();
  ImGui::Text("fps: %.01f", ImGui::GetIO().Framerate);
  ImGui::Text("frame: %.02f ms", 1000.0f / ImGui::GetIO().Framerate);

  // shader
  ImGui::Separator();
  ImGui::Text("load shader");
  for (const auto &kv : settings_.shaderPaths_) {
    if (ImGui::RadioButton(kv.first.c_str(), settings_.shaderPathKey_ == kv.first)) {
      settings_.shaderPathKey_ = kv.first;
      if (settings_.loadShaderCallback_) {
        settings_.loadShaderCallback_(settings_.getShaderPath());
      }
    }
  }
}

void SettingPanel::destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void SettingPanel::updateSize(int width, int height) {
  frameWidth_ = width;
  frameHeight_ = height;
}

void SettingPanel::toggleShowState() {
  visible = !visible;
}

bool SettingPanel::wantCaptureKeyboard() {
  ImGuiIO &io = ImGui::GetIO();
  return io.WantCaptureKeyboard;
}

bool SettingPanel::wantCaptureMouse() {
  ImGuiIO &io = ImGui::GetIO();
  return io.WantCaptureMouse;
}

}
}
