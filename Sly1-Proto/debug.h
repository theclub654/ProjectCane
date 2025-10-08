#pragma once
#include <filesystem>
#include "transition.h"

void RenderMenuGui(SW* psw);
void ExportSw();
void ExportTextures();

extern std::string file;
extern std::string levelName;
extern std::string filePath;
extern ImGuiFileDialog instance_a;
extern bool g_fDisableInput;