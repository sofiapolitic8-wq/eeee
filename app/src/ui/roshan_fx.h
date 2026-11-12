#ifndef ROSHAN_FX_H
#define ROSHAN_FX_H

#include "../user.h"

// Roshan.io visual effects. Everything here is drawn with ImGui draw lists
// (no extra textures / buffers), so it costs almost no RAM and very little CPU.

#define ROSHAN_NAME "Roshan.io"
#define ROSHAN_NAME_UPPER "ROSHAN.IO"
#define ROSHAN_CREDIT "Made by Roshan"

// Animated gradient + glowing orbs + moving 3D perspective grid floor.
void roshan_draw_background(ImDrawList* dl, ImVec2 size, float t);

// Extruded "3D" text. `tilt` is the extrusion direction (-1..1 on both axes),
// `depth` the number of extrusion layers (0 = flat). Text is centered on
// `center`.
void roshan_draw_text3d(ImDrawList* dl, ImFont* font, const char* text,
                        ImVec2 center, float fsize, float t, ImVec2 tilt,
                        int depth, float alpha);

// Single-layer text with a travelling colour wave (left-top anchored).
void roshan_draw_wave_text(ImDrawList* dl, ImFont* font, const char* text,
                           ImVec2 pos, float fsize, float t, float alpha);

// Glass-like rounded card.
void roshan_draw_panel(ImDrawList* dl, ImVec2 p, ImVec2 p2, float t);

// Custom button. variant: 0 = normal, 1 = primary (Play), 2 = danger (Quit).
bool roshan_button(const char* label, ImVec2 size, int variant);

// Extrusion direction from the mouse position + a slow idle sway.
ImVec2 roshan_tilt(ImVec2 screen, float t);

// In-game: small animated watermark + optional FPS counter.
void roshan_draw_overlay(tenv* env);

#endif
