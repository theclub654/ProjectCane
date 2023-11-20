#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "transition.h"

extern std::string file;
void DeleteWorld(SW* psw);

void RenderOpenFileGui();
void RenderCloseWorldGui(SW* psw);