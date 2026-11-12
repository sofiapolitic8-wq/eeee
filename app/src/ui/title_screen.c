#include "title_screen.h"

#include "../network/server.h"
#include "../user.h"
#include "roshan_fx.h"

void ui_title_screen_init(tenv* env) {}

static ImU32 tc(float r, float g, float b, float a) {
  return igColorConvertFloat4ToU32((ImVec4){r, g, b, a});
}

// small rounded "pill" with an icon + value
static void stat_pill(ImDrawList* dl, ImVec2 p, ImVec2 size, const char* text,
                      ImVec4 accent) {
  ImVec2 p2 = {p.x + size.x, p.y + size.y};
  ImDrawList_AddRectFilled(dl, p, p2, tc(accent.x, accent.y, accent.z, 0.14f), size.y * 0.5f, 0);
  ImDrawList_AddRect(dl, p, p2, tc(accent.x, accent.y, accent.z, 0.45f), size.y * 0.5f, 0, 1.0f);
  ImVec2 ts;
  igCalcTextSize(&ts, text, NULL, false, -1);
  ImDrawList_AddText_Vec2(dl, (ImVec2){p.x + (size.x - ts.x) * 0.5f, p.y + (size.y - ts.y) * 0.5f},
                          tc(1, 1, 1, 0.92f), text, NULL);
}

void ui_title_screen(tenv* env) {
  tuser_data* usr = env->usr;
  tcontext* ctx = env->ctx;
  user_settings* usrs = &usr->usrs;
  ImGuiStyle* style = igGetStyle();
  game_data* gdata = &usr->gdata;

  float W = (float)ctx->size[0];
  float H = (float)ctx->size[1];
  float t = (float)igGetTime();
  ImDrawList* dl = igGetWindowDrawList();

  usr->r->global.bg_opacity = 0;
  usr->r->global.bd_opacity = 0;
  usr->r->global.minimap_opacity = 0;

  // ---- animated 3D background ----
  roshan_draw_background(dl, (ImVec2){W, H}, t);

  // ---- 3D logo ----
  ImVec2 tilt = roshan_tilt((ImVec2){W, H}, t);
  float logo_fs = H * 0.13f;
  if (logo_fs < 54) logo_fs = 54;
  if (logo_fs > 120) logo_fs = 120;
  roshan_draw_text3d(dl, usr->imgui_data.regular_font_bold[FONT_SIZE_LARGE],
                     ROSHAN_NAME_UPPER, (ImVec2){W * 0.5f, H * 0.14f + logo_fs * 0.3f},
                     logo_fs, t, tilt, 12, 1.0f);

  // version (top-right)
  igPushFont(usr->imgui_data.regular_font[FONT_SIZE_SMALL],
             usr->imgui_data.regular_font[FONT_SIZE_SMALL]->LegacySize);
  char version_str[40];
  snprintf(version_str, sizeof(version_str), "%s v%s", ROSHAN_NAME, APP_VERSION);
  ImVec2 vtxtsz;
  igCalcTextSize(&vtxtsz, version_str, NULL, false, -1);
  ImDrawList_AddText_Vec2(dl, (ImVec2){W - vtxtsz.x - 12, 8}, tc(0.45f, 0.95f, 0.70f, 0.95f),
                          version_str, NULL);
  igPopFont();

  igPushFont(usr->imgui_data.regular_font[usrs->ui_font_size],
             usr->imgui_data.regular_font[usrs->ui_font_size]->LegacySize);

  float fh = igGetFrameHeight();
  float sp = 6.0f;
  float pad = 18.0f;
  float pw = 440.0f;
  if (pw > W - 24) pw = W - 24;
  float inner = pw - pad * 2;
  float play_h = fh * 1.35f;

  float panel_h = pad + fh + sp * 2 + (fh + sp) * 2 + play_h + sp + fh + sp + fh + pad;
  float x0 = (W - pw) * 0.5f;
  float y0 = H * 0.14f + logo_fs * 1.15f;
  if (y0 + panel_h > H - 70) y0 = H - 70 - panel_h;
  if (y0 < 4) y0 = 4;

  roshan_draw_panel(dl, (ImVec2){x0, y0}, (ImVec2){x0 + pw, y0 + panel_h}, t);

  float cx = x0 + pad;
  float cy = y0 + pad;

  // ---- stats pills ----
  int tot_sec = (int)usrs->play_time;
  char s_score[32], s_kills[32], s_time[32];
  snprintf(s_score, sizeof(s_score), " %d", usrs->score);
  snprintf(s_kills, sizeof(s_kills), " %d", usrs->kills);
  snprintf(s_time, sizeof(s_time), " %02d:%02d:%02d", tot_sec / 3600,
           (tot_sec % 3600) / 60, tot_sec % 60);
  float gap = 8.0f;
  float pill_w = (inner - gap * 2) / 3.0f;
  stat_pill(dl, (ImVec2){cx, cy}, (ImVec2){pill_w, fh}, s_score, (ImVec4){1.0f, 0.78f, 0.25f, 1});
  stat_pill(dl, (ImVec2){cx + pill_w + gap, cy}, (ImVec2){pill_w, fh}, s_kills,
            (ImVec4){1.0f, 0.38f, 0.48f, 1});
  stat_pill(dl, (ImVec2){cx + (pill_w + gap) * 2, cy}, (ImVec2){pill_w, fh}, s_time,
            (ImVec4){0.30f, 0.85f, 1.0f, 1});
  cy += fh + sp * 2;

  // ---- inputs ----
  igSetCursorPos((ImVec2){cx, cy});
  igPushItemWidth(inner);
  igInputTextWithHint("##nickname_input", "Nickname", usrs->nickname,
                      MAX_NICKNAME_LEN + 1, ImGuiInputTextFlags_None, NULL, NULL);
  cy += fh + sp;
  igSetCursorPos((ImVec2){cx, cy});
  igInputTextWithHint("##ipv4_input", "IPv4:Port", usrs->ipv4, MAX_IPV4_LEN + 1,
                      ImGuiInputTextFlags_None, NULL, NULL);
  igPopItemWidth();
  cy += fh + sp;

  // ---- buttons ----
  igSetCursorPos((ImVec2){cx, cy});
  if (roshan_button("  PLAY##play", (ImVec2){inner, play_h}, 1)) {
    usr->gdata.conn = CONNECTING;
    usr->gdata.curr_screen = PLAYING;
    glfwSetTime(0);
    server_connect(env);
  }
  cy += play_h + sp;

  float half = (inner - sp) * 0.5f;
  igSetCursorPos((ImVec2){cx, cy});
  if (roshan_button("  Skin editor", (ImVec2){half, fh}, 0))
    usr->gdata.curr_screen = SKIN_EDITOR;
  igSetCursorPos((ImVec2){cx + half + sp, cy});
  if (roshan_button("  Settings", (ImVec2){half, fh}, 0))
    usr->gdata.curr_screen = SETTINGS;
  cy += fh + sp;

  igSetCursorPos((ImVec2){cx, cy});
  if (roshan_button("  Quit", (ImVec2){inner, fh}, 2)) {
    env->config.running = false;
    save_user_settings(usrs);
  }
  igPopFont();

  // ---- credits (animated 3D) ----
  roshan_draw_text3d(dl, usr->imgui_data.regular_font_bold[FONT_SIZE_REGULAR], ROSHAN_CREDIT,
                     (ImVec2){W * 0.5f, H - 46.0f}, 26.0f, t, tilt, 4, 1.0f);
  igPushFont(usr->imgui_data.regular_font[FONT_SIZE_SMALL],
             usr->imgui_data.regular_font[FONT_SIZE_SMALL]->LegacySize);
  const char* sub = "roshan.io  |  client engine: Vlither by for-loop9 (GPLv3)";
  ImVec2 ssz;
  igCalcTextSize(&ssz, sub, NULL, false, -1);
  ImDrawList_AddText_Vec2(dl, (ImVec2){(W - ssz.x) * 0.5f, H - 22.0f}, tc(1, 1, 1, 0.38f), sub, NULL);
  igPopFont();
}

void ui_title_screen_destroy(tenv* env) {}
