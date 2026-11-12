#include "roshan_fx.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static ImU32 c4(float r, float g, float b, float a) {
  return igColorConvertFloat4ToU32((ImVec4){r, g, b, a});
}

static ImU32 hsv(float h, float s, float v, float a) {
  float r, g, b;
  igColorConvertHSVtoRGB(h - floorf(h), s, v, &r, &g, &b);
  return c4(r, g, b, a);
}

static float clamp01(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }

void roshan_draw_background(ImDrawList* dl, ImVec2 size, float t) {
  float s1 = 0.5f + 0.5f * sinf(t * 0.25f);
  float s2 = 0.5f + 0.5f * sinf(t * 0.19f + 2.0f);

  ImDrawList_AddRectFilledMultiColor(
      dl, (ImVec2){0, 0}, size, c4(0.05f + 0.04f * s1, 0.03f, 0.15f + 0.06f * s2, 1),
      c4(0.03f, 0.05f + 0.04f * s2, 0.17f + 0.05f * s1, 1), c4(0.02f, 0.02f, 0.06f, 1),
      c4(0.07f + 0.03f * s2, 0.02f, 0.12f, 1));

  // glowing orbs (3 stacked translucent circles each = soft glow)
  for (int i = 0; i < 12; i++) {
    float fi = (float)i;
    float x = size.x * (0.5f + 0.48f * sinf(t * (0.07f + 0.013f * fi) + fi * 1.7f));
    float y = size.y * (0.42f + 0.36f * cosf(t * (0.05f + 0.011f * fi) + fi * 2.3f));
    float r = 46.0f + 24.0f * fmodf(fi * 7.0f, 5.0f);
    float h = 0.60f + 0.055f * fi + t * 0.01f;
    ImDrawList_AddCircleFilled(dl, (ImVec2){x, y}, r, hsv(h, 0.65f, 1, 0.025f), 28);
    ImDrawList_AddCircleFilled(dl, (ImVec2){x, y}, r * 0.66f, hsv(h, 0.55f, 1, 0.035f), 24);
    ImDrawList_AddCircleFilled(dl, (ImVec2){x, y}, r * 0.33f, hsv(h, 0.40f, 1, 0.05f), 18);
  }

  // retro perspective grid floor (3D feel)
  float cx = size.x * 0.5f;
  float hz = size.y * 0.66f;
  ImDrawList_AddRectFilledMultiColor(dl, (ImVec2){0, hz - size.y * 0.10f},
                                     (ImVec2){size.x, hz}, c4(0.5f, 0.3f, 1, 0),
                                     c4(0.5f, 0.3f, 1, 0), c4(0.5f, 0.3f, 1, 0.10f),
                                     c4(0.5f, 0.3f, 1, 0.10f));
  for (int i = 0; i < 14; i++) {
    float u = fmodf((float)i + t * 0.45f, 14.0f) / 14.0f;
    float y = hz + (size.y - hz) * u * u;
    ImDrawList_AddLine(dl, (ImVec2){0, y}, (ImVec2){size.x, y},
                       c4(0.55f, 0.40f, 1.0f, 0.04f + 0.30f * u), 1.0f);
  }
  for (int i = -14; i <= 14; i++) {
    float fx = (float)i;
    ImDrawList_AddLine(dl, (ImVec2){cx + fx * size.x * 0.010f, hz},
                       (ImVec2){cx + fx * size.x * 0.115f, size.y},
                       c4(0.30f, 0.80f, 1.0f, 0.16f), 1.0f);
  }
}

ImVec2 roshan_tilt(ImVec2 screen, float t) {
  ImGuiIO* io = igGetIO_Nil();
  float mx = screen.x > 1 ? (io->MousePos.x / screen.x - 0.5f) * 2.0f : 0;
  float my = screen.y > 1 ? (io->MousePos.y / screen.y - 0.5f) * 2.0f : 0;
  if (mx < -1.5f || mx > 1.5f || my < -1.5f || my > 1.5f) mx = my = 0;
  return (ImVec2){-mx * 0.9f + 0.5f * sinf(t * 0.8f),
                  0.55f - my * 0.5f + 0.15f * cosf(t * 0.6f)};
}

void roshan_draw_text3d(ImDrawList* dl, ImFont* font, const char* text,
                        ImVec2 center, float fsize, float t, ImVec2 tilt,
                        int depth, float alpha) {
  int n = (int)strlen(text);
  if (n <= 0 || n > 63) return;

  float adv[64];
  float total = 0;
  float spacing = fsize * 0.03f;
  for (int i = 0; i < n; i++) {
    char tmp[2] = {text[i], 0};
    ImVec2 sz;
    ImFont_CalcTextSizeA(&sz, font, fsize, FLT_MAX, 0, tmp, NULL, NULL);
    adv[i] = sz.x;
    total += sz.x + spacing;
  }
  total -= spacing;

  float step = fsize * 0.020f;
  float x0 = center.x - total * 0.5f;
  float y0 = center.y - fsize * 0.5f;

  // soft glow (cheap: 4 offsets)
  if (depth > 0) {
    float x = x0;
    for (int i = 0; i < n; i++) {
      char tmp[2] = {text[i], 0};
      float bob = sinf(t * 2.0f + (float)i * 0.55f) * fsize * 0.035f;
      float g = fsize * 0.05f;
      ImU32 gc = hsv(0.66f + 0.08f * sinf(t * 0.7f + (float)i * 0.4f), 0.7f, 1, 0.07f * alpha);
      ImVec2 o[4] = {{-g, 0}, {g, 0}, {0, -g}, {0, g}};
      for (int k = 0; k < 4; k++)
        ImDrawList_AddText_FontPtr(dl, font, fsize,
                                   (ImVec2){x + o[k].x, y0 + bob + o[k].y + step * depth * tilt.y},
                                   gc, tmp, NULL, 0, NULL);
      x += adv[i] + spacing;
    }
  }

  for (int layer = depth; layer >= 0; layer--) {
    float k = (float)layer * step;
    float x = x0;
    for (int i = 0; i < n; i++) {
      char tmp[2] = {text[i], 0};
      float bob = sinf(t * 2.0f + (float)i * 0.55f) * fsize * 0.035f;
      ImVec2 pos = {x + tilt.x * k * 1.6f, y0 + bob + tilt.y * k};
      ImU32 col;
      if (layer == 0) {
        float h = 0.56f + 0.14f * sinf(t * 0.8f + (float)i * 0.5f);
        col = hsv(h, 0.55f, 1.0f, alpha);
      } else {
        float f = (float)layer / (float)(depth > 0 ? depth : 1);
        col = c4(0.30f - 0.20f * f, 0.16f - 0.10f * f, 0.62f - 0.34f * f, alpha);
      }
      ImDrawList_AddText_FontPtr(dl, font, fsize, pos, col, tmp, NULL, 0, NULL);

      if (layer == 0) {  // moving shine over the front face
        float sh = sinf(t * 1.7f - (float)i * 0.42f);
        sh = sh > 0 ? powf(sh, 10.0f) : 0;
        if (sh > 0.02f)
          ImDrawList_AddText_FontPtr(dl, font, fsize, pos, c4(1, 1, 1, clamp01(sh * 0.85f) * alpha),
                                     tmp, NULL, 0, NULL);
      }
      x += adv[i] + spacing;
    }
  }
}

void roshan_draw_wave_text(ImDrawList* dl, ImFont* font, const char* text,
                           ImVec2 pos, float fsize, float t, float alpha) {
  int n = (int)strlen(text);
  float x = pos.x;
  for (int i = 0; i < n && i < 63; i++) {
    char tmp[2] = {text[i], 0};
    ImVec2 sz;
    ImFont_CalcTextSizeA(&sz, font, fsize, FLT_MAX, 0, tmp, NULL, NULL);
    float bob = sinf(t * 2.4f + (float)i * 0.5f) * fsize * 0.10f;
    float h = 0.52f + 0.20f * (0.5f + 0.5f * sinf(t * 1.1f - (float)i * 0.35f));
    ImDrawList_AddText_FontPtr(dl, font, fsize, (ImVec2){x, pos.y + bob},
                               hsv(h, 0.55f, 1.0f, alpha), tmp, NULL, 0, NULL);
    x += sz.x;
  }
}

void roshan_draw_panel(ImDrawList* dl, ImVec2 p, ImVec2 p2, float t) {
  ImDrawList_AddRectFilled(dl, (ImVec2){p.x - 8, p.y - 4}, (ImVec2){p2.x + 8, p2.y + 12},
                           c4(0, 0, 0, 0.28f), 26, 0);
  ImDrawList_AddRectFilled(dl, p, p2, c4(0.07f, 0.07f, 0.15f, 0.82f), 18, 0);
  ImDrawList_AddRect(dl, p, p2, c4(0.50f, 0.38f, 1.0f, 0.38f + 0.14f * sinf(t * 1.3f)), 18, 0, 1.5f);
  // accent line on top edge
  float w = p2.x - p.x;
  float a = 0.5f + 0.5f * sinf(t * 0.9f);
  ImDrawList_AddRectFilled(dl, (ImVec2){p.x + w * (0.12f + 0.2f * a), p.y},
                           (ImVec2){p.x + w * (0.52f + 0.2f * a), p.y + 2.5f},
                           c4(0.25f, 0.85f, 1.0f, 0.8f), 2, 0);
}

bool roshan_button(const char* label, ImVec2 size, int variant) {
  ImVec2 p;
  igGetCursorScreenPos(&p);
  bool pressed = igInvisibleButton(label, size, 0);
  bool hov = igIsItemHovered(0);
  bool act = igIsItemActive();
  ImDrawList* dl = igGetWindowDrawList();
  ImVec2 p2 = {p.x + size.x, p.y + size.y};

  float r, g, b;
  if (variant == 1) { r = 0.42f; g = 0.30f; b = 0.95f; }
  else if (variant == 2) { r = 0.80f; g = 0.22f; b = 0.34f; }
  else { r = 0.20f; g = 0.20f; b = 0.34f; }
  float k = act ? 0.75f : (hov ? 1.25f : 1.0f);

  if (hov && !act) {
    for (int i = 3; i >= 1; i--)
      ImDrawList_AddRect(dl, (ImVec2){p.x - i * 2.0f, p.y - i * 2.0f},
                         (ImVec2){p2.x + i * 2.0f, p2.y + i * 2.0f},
                         c4(r * 1.3f, g * 1.3f, b * 1.3f, 0.10f), 12 + i * 2, 0, 2.0f);
  }
  ImDrawList_AddRectFilled(dl, p, p2, c4(r * k, g * k, b * k, 1.0f), 12, 0);
  ImDrawList_AddRectFilled(dl, (ImVec2){p.x + 1, p.y + 1}, (ImVec2){p2.x - 1, p.y + size.y * 0.48f},
                           c4(1, 1, 1, 0.07f), 11, ImDrawFlags_RoundCornersTop);
  ImDrawList_AddRect(dl, p, p2, c4(1, 1, 1, hov ? 0.40f : 0.14f), 12, 0, 1.2f);

  ImVec2 ts;
  igCalcTextSize(&ts, label, NULL, true, -1);
  const char* label_end = strstr(label, "##");  // hide the ImGui id suffix
  ImDrawList_AddText_Vec2(dl, (ImVec2){p.x + (size.x - ts.x) * 0.5f, p.y + (size.y - ts.y) * 0.5f},
                          c4(1, 1, 1, 1), label, label_end);
  return pressed;
}

void roshan_draw_overlay(tenv* env) {
  tuser_data* usr = env->usr;
  user_settings* usrs = &usr->usrs;
  ImGuiViewport* vp = igGetMainViewport();
  ImDrawList* dl = igGetForegroundDrawList_ViewportPtr(vp);
  float t = (float)igGetTime();

  ImFont* f = usr->imgui_data.regular_font_bold[FONT_SIZE_SMALL];
  if (usr->gdata.curr_screen == PLAYING) {
    roshan_draw_wave_text(dl, f, "roshan.io", (ImVec2){vp->Pos.x + 12, vp->Pos.y + vp->Size.y - 30},
                          16.0f, t, 0.50f);
  }
  if (usrs->show_fps) {
    char buf[24];
    snprintf(buf, sizeof(buf), "%d FPS", (int)(tfps_current() + 0.5f));
    ImDrawList_AddText_FontPtr(dl, f, 16.0f, (ImVec2){vp->Pos.x + vp->Size.x - 74, vp->Pos.y + 6},
                               c4(0.35f, 1.0f, 0.55f, 0.9f), buf, NULL, 0, NULL);
  }
}
