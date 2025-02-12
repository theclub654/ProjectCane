#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "transition.h"
#include <filesystem>

extern std::string file;
extern std::string levelName;
extern std::vector<ALO*> allSWAloObjs;
static std::string filePath;
void DeleteWorld(SW* psw);

static ImGuiFileDialog instance_a;
extern inline bool fRenderModels = true;
extern inline bool fRenderCollision = false;

void RenderMenuGui(SW* psw);
void ExportSwObj();