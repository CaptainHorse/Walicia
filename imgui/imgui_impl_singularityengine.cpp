#include "imgui.h"
#include "imgui_impl_singularityengine.h"
#include "imgui_impl_opengl3.h"

#include <wallicia.h>

// Engine integration
#include <array>
#include <utility>

#include <Input/input_manager.h>

using namespace singularity_engine;
using namespace input;
using namespace graphics;

static double g_Time = 0.0;
static bool	g_WantUpdateMonitors = true;
static std::shared_ptr<Window> g_SEWindow = nullptr;
static bool	g_MouseJustPressed[3] = { false, false, false };

void ImGui_ImplSE_CharCB(uint c) {
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(c);
}

bool ImGui_ImplSE_Init(std::shared_ptr<Window> window)
{
	g_SEWindow = window;
	g_Time = 0.0;

	g_SEWindow->setCharCallback(ImGui_ImplSE_CharCB);

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigWindowsMoveFromTitleBarOnly = true;

	io.BackendPlatformName = "imgui_impl_singularityengine";
	io.BackendRendererName = "imgui_impl_singularityengine[opengl/vulkan]";

	io.KeyMap[ImGuiKey_Tab] = Keys::KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = Keys::KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = Keys::KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = Keys::KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = Keys::KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = Keys::KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown] = Keys::KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home] = Keys::KEY_HOME;
	io.KeyMap[ImGuiKey_End] = Keys::KEY_END;
	io.KeyMap[ImGuiKey_Insert] = Keys::KEY_INSERT;
	io.KeyMap[ImGuiKey_Delete] = Keys::KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = Keys::KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = Keys::KEY_SPACE;
	io.KeyMap[ImGuiKey_Enter] = Keys::KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape] = Keys::KEY_ESCAPE;
	io.KeyMap[ImGuiKey_A] = Keys::KEY_A;
	io.KeyMap[ImGuiKey_C] = Keys::KEY_C;
	io.KeyMap[ImGuiKey_V] = Keys::KEY_V;
	io.KeyMap[ImGuiKey_X] = Keys::KEY_X;
	io.KeyMap[ImGuiKey_Y] = Keys::KEY_Y;
	io.KeyMap[ImGuiKey_Z] = Keys::KEY_Z;

	io.ImeWindowHandle = g_SEWindow->getWindowHandle();

	ImGui_ImplOpenGL3_Init("#version 330 core");

	return true;
}

void ImGui_ImplSE_Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	g_SEWindow.reset();
}

void ImGui_ImplSE_Update()
{
	ImGuiIO& io = ImGui::GetIO();

	io.MouseWheelH += InputManager::getPointerScroll().x;
	io.MouseWheel += InputManager::getPointerScroll().y;
}

void ImGui_ImplSE_UpdateCursor()
{
	ImGuiIO& io = ImGui::GetIO();

	for (uint i = 0; i < IM_ARRAYSIZE(g_MouseJustPressed); ++i)
		g_MouseJustPressed[i] = InputManager::getPointerStates()[i];

	io.MouseWheelH += InputManager::getPointerScroll().x;
	io.MouseWheel += InputManager::getPointerScroll().y;

	io.MousePos.x = InputManager::getPointerPosition().x;
	io.MousePos.y = InputManager::getPointerPosition().y;

	for (int i = 0; i < IM_ARRAYSIZE(g_MouseJustPressed); ++i) {
		io.MouseDown[i] = g_MouseJustPressed[i] || InputManager::getPointerStates()[i] != 0;
		g_MouseJustPressed[i] = false;
	}
}

void ImGui_ImplSE_UpdateKeys()
{
	ImGuiIO& io = ImGui::GetIO();

	for (uint i = 0; i < 512; i++) 
		io.KeysDown[i] = InputManager::getKeyStates()[i];

	io.KeyCtrl = io.KeysDown[Keys::KEY_LEFT_CONTROL] || io.KeysDown[Keys::KEY_RIGHT_CONTROL];
	io.KeyShift = io.KeysDown[Keys::KEY_LEFT_SHIFT] || io.KeysDown[Keys::KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[Keys::KEY_LEFT_ALT] || io.KeysDown[Keys::KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[Keys::KEY_LEFT_SUPER] || io.KeysDown[Keys::KEY_RIGHT_SUPER];
}

void ImGui_ImplSE_NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();

	int w, h;
	int display_w, display_h;
	w = g_SEWindow->getWindowFrameSize().x;
	h = g_SEWindow->getWindowFrameSize().y;
	display_w = g_SEWindow->getWindowFrameSize().x;
	display_h = g_SEWindow->getWindowFrameSize().y;
	io.DisplaySize = ImVec2((float)w, (float)h);
	if (w > 0 && h > 0)
		io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

	const ImVec2 mouse_pos_backup = io.MousePos;
	io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

	// Setup time step
	if (Wallicia::getInstance()->getDeltatime() <= 0.0 || g_Time < 0.0) // hack for initial newframe when deltatime is ded TODO: Fix initial deltatime in Application class
		io.DeltaTime = (float)(1.0f / 60.0f);
	else
		io.DeltaTime = (float)(Wallicia::getInstance()->getTime() - g_Time);

	g_Time = Wallicia::getInstance()->getTime();

	ImGui_ImplOpenGL3_NewFrame();
}

void ImGui_ImplSE_RenderDrawData(ImDrawData* draw_data)
{
	ImGui_ImplOpenGL3_RenderDrawData(draw_data);
}