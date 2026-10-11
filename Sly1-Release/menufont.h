#pragma once

// -----------------------------------------------------------------------------
// TrueType fonts for the menus (Options > Customization > Text > Font).
//
// Fonts come from <exe folder>\Fonts (bundled free fonts plus anything the
// player drops in) and a list of common installed Windows fonts. They are
// loaded into the ImGui font atlas once at startup and drawn through ImGui's
// foreground draw list, positioned in the same output pixels as the menus.
//
// Text that uses the game's rich-text markup ('&' codes for controller
// glyphs) cannot be shown with a TrueType font; callers fall back to the
// game font for those strings (FMenuFontUsable returns false).
// -----------------------------------------------------------------------------

#include <glm/glm.hpp>

void StartupMenuFonts();                       // after ImGui init, before the first ImGui frame

int CMenuFonts();
const char* PchzMenuFontName(int ifont);

bool FMenuFontUsable(int ifont, const char* text);
// dyLine = the game font's line height for this text, in menu (output) pixels.
float DxMenuFontText(int ifont, const char* text, float dyLine);
// Draws text (may contain '\n') with its top-left at x, y (menu pixels).
void DrawMenuFontText(int ifont, const char* text, float x, float y, float dyLine, const glm::vec4& rgba);

// Always-on-top overlay drawing (value editor). ifont < 0 = ImGui's built-in font.
float DxOverlayText(int ifont, const char* text, float dyLine);
void DrawOverlayText(int ifont, const char* text, float x, float y, float dyLine, const glm::vec4& rgba);
void FillOverlayRect(float x0, float y0, float x1, float y1, const glm::vec4& rgba);
// Shapes for button icons (thickness <= 0 = filled).
void OverlayRect(float x0, float y0, float x1, float y1, const glm::vec4& rgba, float rounding, float thickness);
void OverlayCircle(float cx, float cy, float radius, const glm::vec4& rgba, float thickness);
void OverlayLine(float x0, float y0, float x1, float y1, const glm::vec4& rgba, float thickness);
void OverlayTriangle(float x0, float y0, float x1, float y1, float x2, float y2, const glm::vec4& rgba, float thickness);
