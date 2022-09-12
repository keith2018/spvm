/*
 * spvm-shadertoy
 * @author 	: keith@robot9.me
 *
 */

#pragma once

#include "imgui/imgui.h"
#include "settings.h"

namespace SPVM {
namespace ShaderToy {

class SettingPanel {
 public:
  explicit SettingPanel(Settings &settings) : settings_(settings) {}

  void init(void *window, int width, int height);
  void onDraw();
  void destroy();
  void updateSize(int width, int height);
  void toggleShowState();

  bool wantCaptureKeyboard();
  bool wantCaptureMouse();

 private:
  void drawSettings();

 private:
  Settings &settings_;

  bool visible = true;
  int frameWidth_ = 0;
  int frameHeight_ = 0;
};

}
}
