#pragma once

#include <memory>
#include <Graphics/Window/window.h>

IMGUI_API bool        ImGui_ImplSE_Init(SE::graphics::Window* window);
IMGUI_API void        ImGui_ImplSE_Shutdown();
IMGUI_API void        ImGui_ImplSE_NewFrame();
IMGUI_API void        ImGui_ImplSE_RenderDrawData(ImDrawData* draw_data);

// input callbacks
IMGUI_API void        ImGui_ImplSE_Update();
IMGUI_API void        ImGui_ImplSE_UpdateCursor();
IMGUI_API void        ImGui_ImplSE_UpdateKeys();