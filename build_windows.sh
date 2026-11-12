#!/usr/bin/env bash
# Roshan.io - direct build (no premake / make needed).
# Run inside an MSYS2 "MINGW64" shell:  ./build_windows.sh
# Needs: mingw-w64-x86_64-gcc, mingw-w64-x86_64-vulkan-headers, mingw-w64-x86_64-vulkan-loader
set -euo pipefail
cd "$(dirname "$0")"

PLATFORM="${PLATFORM:-windows}"
CC="${CC:-gcc}"
CXX="${CXX:-g++}"
VK_INC="${VK_INC:-$(cygpath -m /mingw64 2>/dev/null || echo /mingw64)/include}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 2)}"
OUT=build/obj
BIN=build/bin
EXE="$BIN/roshan-io.exe"
rm -rf "$OUT" "$BIN"; mkdir -p "$OUT" "$BIN"

[ -d "$VK_INC/vulkan" ] || { echo "ERROR: Vulkan headers not found in $VK_INC"; exit 1; }

INC=(-Iglfw/include -Ithermite/include -Ithermite/src -Iapp/src "-I$VK_INC")
DEFS=(-DGLFW_INCLUDE_NONE -DGLFW_INCLUDE_VULKAN -DCGLM_FORCE_DEPTH_ZERO_TO_ONE
      -DCIMGUI_NO_EXPORT '-DAPP_VERSION="2.6"' '-DSETTINGS_VERSION="1.4"' -DROSHAN_BUILD=1)
OPT=(-O2 -w)

# ---- source lists ----
FILES=()
if [ "$PLATFORM" = "windows" ]; then
  for f in context init input monitor platform vulkan window egl_context osmesa_context \
           null_init null_monitor null_window null_joystick \
           wgl_context win32_init win32_joystick win32_module win32_thread win32_time \
           win32_window win32_monitor; do
    FILES+=("glfw/src/$f.c")
  done
fi
while IFS= read -r f; do FILES+=("$f"); done < <(find thermite/src app/src \( -name '*.c' -o -name '*.cpp' \) | sort)

compile() {
  local src="$1" obj="$OUT/$(echo "$1" | tr '/' '_').o" extra=()
  case "$src" in glfw/*) extra=(-D_GLFW_WIN32);; esac
  case "$src" in
    *.cpp) "$CXX" "${OPT[@]}" "${INC[@]}" "${DEFS[@]}" "${extra[@]}" -c "$src" -o "$obj" ;;
    *)     "$CC"  "${OPT[@]}" "${INC[@]}" "${DEFS[@]}" "${extra[@]}" -c "$src" -o "$obj" ;;
  esac
  echo "  compiled $src"
}

echo "Compiling ${#FILES[@]} files with $JOBS jobs..."
pids=()
for f in "${FILES[@]}"; do
  compile "$f" & pids+=($!)
  while [ "$(jobs -rp | wc -l)" -ge "$JOBS" ]; do sleep 0.05; done
done
fail=0
for p in "${pids[@]}"; do wait "$p" || fail=1; done
[ "$fail" = 0 ] || { echo "ERROR: compile failed (see messages above)"; exit 1; }

[ "${COMPILE_ONLY:-0}" = 1 ] && { echo "Compile-only OK"; exit 0; }

echo "Linking $EXE ..."
"$CXX" -O2 -static "$OUT"/*.o -o "$EXE" -lvulkan-1 -lgdi32 -lws2_32 -lm
echo "BUILD OK: $EXE ($(du -h "$EXE" | cut -f1))"
