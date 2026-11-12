#include "tentry.h"
#include "tfps.h"

int tentry(int argc, char** argv, size_t usr_size) {
  tenv env;

  env.config.argc = argc;
  env.config.argv = argv;
  env.config.vsync = true;
  env.config.fps_limit = 60;
  env.config.running = true;
  env.config.fullscreen = false;
  env.config.resizable = true;
  env.config.aspect_ratio = 16 / 9.0f;
  env.config.fif = 2; /* 2 frames in flight: lower latency + less memory */
  env.config.title = "app";
  env.usr = malloc(usr_size);

  tlaunch(&env);

  env.wnd = twindow_create(&env, trender, tresize);
  env.kb = tkeyboard_create(env.wnd);
  env.ms = tmouse_create(env.wnd);
  if ((env.ctx = tcontext_create(env.wnd, env.config.vsync, env.config.fif)) == NULL) {
    printf("Error creating context.\n");
    exit(1);
  }
  tinit(&env);

  while (env.config.running) {
    if (env.ctx->swapchain_ok) {
      twindow_poll_input(env.wnd);
    } else {
      twindow_wait_input(env.wnd);
      continue;
    }

    tinput(&env);
    trender(&env);

    tkeyboard_update(env.kb);
    tmouse_update(env.ms);

    /* FPS lock. Drops to 30 fps while the window is in the background. */
    {
      int cap = env.config.fps_limit;
      if (!glfwGetWindowAttrib(env.wnd->handle, GLFW_FOCUSED) &&
          (cap <= 0 || cap > 30))
        cap = 30;
      tfps_limit(cap);
    }
  }
  tcontext_wait_idle(env.ctx);

  tdestroy(&env);
  tcontext_destroy(env.ctx);
  tmouse_destroy(env.ms);
  tkeyboard_destroy(env.kb);
  twindow_destroy(env.wnd);
  free(env.usr);

  return 0;
}