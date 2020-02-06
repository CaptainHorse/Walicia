#include <wallicia.h>

#include <memory>
#include <random>

#include <wallicia_control.h>
#include <imgui_impl_singularityengine.h>

#include <Utilities/argument_parser.h>

using namespace SE;
using namespace input;
using namespace maths;
using namespace graphics;

static bool buddyMode = false; // TODO: "Drag here" area in control window
bool Wallicia::vulkanMode = false; // Oh yeah

VideoDecoder Wallicia::dec;

std::shared_ptr<Window> controlWindow = nullptr;
std::shared_ptr<Renderable> movingQuad = nullptr;

int main(int argc, char** argv) {
	// Pre-set the argument parser arguments before application does it
	ArgumentParser::set(argc, argv);

	if (ArgumentParser::exists("-opengl") && !ArgumentParser::exists("-vulkan"))
		Wallicia::vulkanMode = false;
#ifdef SE_GRAPHICS_VULKAN
	else if (ArgumentParser::exists("-vulkan") && !ArgumentParser::exists("-opengl"))
		Wallicia::vulkanMode = true;
#endif
	else
		Wallicia::vulkanMode = false;

	// Main window flags
	int windowFlags = 0;
	if (ArgumentParser::exists("-buddy")) {
		windowFlags = eWindowFlag_Windowed | eWindowFlag_NoDecorations | eWindowFlag_Transparent | eWindowFlag_Opaque | eWindowFlag_TopMost | eWindowFlag_NoInput;
		buddyMode = true;
	} else
		windowFlags = eWindowFlag_Borderless | eWindowFlag_NoDecorations | eWindowFlag_DesktopWindow | eWindowFlag_NoInput;

	// Skip over application creation if we just want to fix and return the wallpaper back to normal
	if (ArgumentParser::exists("-fixwp")) {
#ifdef SE_OS_WINDOWS
		SystemParametersInfoW(SPI_SETDESKWALLPAPER, NULL, NULL, SPIF_SENDCHANGE);
#endif
	} else {
		ApplicationParameters params {
			"Wallicia", // Application name
			true, 512, 512, windowFlags, // Create window, windowed resolution, flags
			true, eOpenGL, // Initialize graphics, preferred Renderer type
			true, true, false, false, false, true // Initialize input, audio, scripting, compute, networking, threading, in that order
		};

		Wallicia wallicia(argc, argv, params);
		wallicia.Start();
	}

	return 0;
}

void Wallicia::Setup()
{
}

void Wallicia::VideoOpen(const std::string& path, const bool& sound)
{
	// Decode video file to texture
	auto texture = std::make_shared<Texture>();
	auto targetFPS = dec.Decode(path, movingQuad, texture, sound);
	Wallicia::getInstance()->setFPSLimit(targetFPS);
}

void Wallicia::VideoClose()
{
	dec.Clean();
}

void Wallicia::RendererSetup()
{
	// Regain control for main window
	if (!vulkanMode) WindowManager::GetCurrentWithContext()->gainGLContext();

	// We require atleast Identity matrix for view
	RendererManager::getRenderer()->setViewMatrix(matrix4x4::Identity());

	// Handle aspect ratios for ortho matrix
	float left, right, bottom, top;
	if (WindowManager::GetCurrentWithContext()->getFrameWidth() > WindowManager::GetCurrentWithContext()->getFrameHeight()) {
		top = 1.0f;
		bottom = -top;
		right = top * WindowManager::GetCurrentWithContext()->getAspectRatio();
		left = -right;
	} else {
		right = 1.0f;
		left = -right;
		top = right / WindowManager::GetCurrentWithContext()->getAspectRatio();
		bottom = -top;
	}

	// Set projection matrix
	auto proj = matrix4x4::Orthographic(left, right, bottom, top, -10.0f, 10.0f);
	RendererManager::getRenderer()->setProjectionMatrix(proj);

	// Get a default shader, create quad with it and add it to the renderer
	movingQuad = std::make_shared<Renderable>(ModelManager::CreateQuad(vector3(1.0f)), ShaderManager::Get("Default/TexturedTransform"), nullptr);
	RendererManager::getRenderer()->renderableAdd(movingQuad);
}

void Wallicia::Begin()
{
	// ImGui Wallicia Control Window
#ifdef SE_OS_WINDOWS
	controlWindow = WindowManager::Add(std::make_shared<Win32_Window>("Wallicia Control", 500, 500, eWindowFlag_Windowed | eWindowFlag_OGLContext | eWindowFlag_StartCenter | eWindowFlag_NoDecorations | eWindowFlag_SystemTray));
#else
	controlWindow = WindowManager::Add(std::make_shared<WL_Window>("Wallicia Control", 500, 500, eWindowFlag_Windowed | eWindowFlag_OGLContext | eWindowFlag_StartCenter | eWindowFlag_NoDecorations | eWindowFlag_SystemTray));
#endif
	controlWindow->setWindowSize(512, 512); // Fix for non-properly scaled ImGui widgets at startup

	// Give control window input
	InputManager::setInputWindow(controlWindow);

	// Nya
	ImGui::CreateContext();

	// Manually initialize glbinding for ImGui to use
	glbinding::initialize(nullptr, false);

	// ImGui Singularity Engine Implementation init
	ImGui_ImplSE_Init(controlWindow);

	RendererSetup();
}

void Wallicia::Tick()
{
}

bool canMove = false;
maths::tvector2<int> oPos;
void Wallicia::Update()
{
	// Stop when control window is closed fully
	if (controlWindow != nullptr && controlWindow->Closed())
		Stop();

	// ImGui Input
	ImGui_ImplSE_Update();
	ImGui_ImplSE_UpdateKeys();

	if (controlWindow != nullptr && controlWindow->isShowing() && !controlWindow->Closed()) {
		if (InputManager::isPointerClicked(0) && InputManager::getPointerPosition().y <= 20) {
#ifdef SE_OS_WINDOWS
			POINT p = {};
			GetCursorPos(&p);
			ScreenToClient((HWND)controlWindow->getWindowHandle(), &p);
			oPos.x = p.x;
			oPos.y = p.y;
#endif
			canMove = true;
		}

		if (ImGui::IsMouseReleased(0))
			canMove = false;

		if (ImGui::IsMouseDragging() && canMove) {
#ifdef SE_OS_WINDOWS
			POINT p = {};
			GetCursorPos(&p);
#endif
			controlWindow->setPosition(p.x - oPos.x, p.y - oPos.y);
		}
	}
}

void Wallicia::PreRender()
{
}

void Wallicia::Render()
{
	dec.Frame(getTime());

	if (controlWindow != nullptr && controlWindow->isShowing() && !controlWindow->Closed()) {
		// Let Wallicia control window take GL context for drawing ImGui
		controlWindow->gainGLContext();

		// Manual clearing of color buffer to prevent winning at solitaire
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplSE_NewFrame();
		ImGui_ImplSE_UpdateCursor();
		ImGui::NewFrame();

		//ImGui::ShowDemoWindow(); // Useful to keep around when needing to debug ImGui integration issues
		Draw_Wallicia_Control(controlWindow);

		ImGui::Render();

		ImGui_ImplSE_RenderDrawData(ImGui::GetDrawData());

		// Restore GLContext back to main window
		if (!vulkanMode) WindowManager::GetCurrentWithContext()->gainGLContext();
	}
}

void Wallicia::PastRender()
{
}

void Wallicia::Present()
{
}

void Wallicia::Quitting()
{
	ImGui_ImplSE_Shutdown();
	ImGui::DestroyContext();
}