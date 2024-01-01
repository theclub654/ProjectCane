#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "transition.h"

extern std::string file;
void DeleteWorld(SW* psw);

static ImGuiFileDialog instance_a;
extern inline bool fRenderModels = true;
extern inline bool fRenderCollision = false;
extern inline bool fRenderWireFrame = false;

void RenderMenuGui(SW* psw);