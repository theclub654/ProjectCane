#pragma once
bool CaneSwitchStartup(int argc, char** argv);
void CaneSwitchShutdown();
void CaneSwitchError(const char* message);
void CaneSwitchRumble(float low, float high);
