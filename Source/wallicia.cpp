#include <wallicia.h>

#include <memory>
#include <random>

#include <wallicia_control.h>
#include <imgui_impl_singularityengine.h>

#include <Utilities/argument_parser.h>

using namespace SE;
using namespace audio;
using namespace input;
using namespace maths;
using namespace system;
using namespace graphics;

bool Wallicia::buddyMode = false; // TODO: "Drag here" area in control window
bool Wallicia::vulkanMode = false;
VideoDecoder Wallicia::videoDecoder;
std::atomic<bool> Wallicia::keepDecoding = false;
std::shared_ptr<SE::graphics::Texture> Wallicia::videoTexture;
std::shared_ptr<SE::graphics::Renderable> Wallicia::videoRenderable;

Window* controlWindow = nullptr;
std::queue<FrameResult> videoFrames, audioFrames;

int main(int argc, char** argv) {
	// Pre-set the argument parser arguments before application does it since we have custom arguments
	ArgumentParser::set(argc, argv);

	if (ArgumentParser::exists("-opengl") && !ArgumentParser::exists("-vulkan"))
		Wallicia::vulkanMode = false;
#ifdef SE_GRAPHICS_VULKAN
	else if (ArgumentParser::exists("-vulkan") && !ArgumentParser::exists("-opengl"))
		Wallicia::vulkanMode = true;
#endif
	else
		Wallicia::vulkanMode = false;

	int appFlags = eInitAudio | eInitGraphics | eInitInput | eInitThreading | eCreateDefaultWindow;
	if (Wallicia::vulkanMode) 
		appFlags |= eRendererVulkan;
	else
		appFlags |= eRendererOpenGL;

	// Main window flags
	int windowFlags = 0;
	if (ArgumentParser::exists("-buddy")) {
		windowFlags = eWindowFlag_Windowed | eWindowFlag_NoDecorations | eWindowFlag_Transparent | eWindowFlag_Opaque | eWindowFlag_TopMost | eWindowFlag_NoInput;
		Wallicia::buddyMode = true;
	} else
		windowFlags = eWindowFlag_Borderless | eWindowFlag_NoDecorations | eWindowFlag_DesktopWindow | eWindowFlag_NoInput;

	// Skip over application creation if we just want to fix and return the wallpaper back to normal
	if (ArgumentParser::exists("-fixwp")) {
#ifdef SE_OS_WINDOWS
		SystemParametersInfoW(SPI_SETDESKWALLPAPER, NULL, NULL, SPIF_SENDCHANGE);
#endif
	} else {
		ApplicationParameters params {
			"Wallicia",					// Application name
			appFlags,					// Application flags
			{ 1680, 1050, windowFlags }	// Application window creation info
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
	auto res = videoDecoder.Init(path, sound);
	if (sound)
		AudioManager::getImplementation()->straightPlay(res.audio); // Does initialization on first call

	videoTexture->updateSize(res.width, res.height);

	keepDecoding = true;
	ThreadManager::AddJobToThread([&]() {
		while (keepDecoding) {
			videoDecoder.Decode();
			videoDecoder.VideoFrame(videoFrames, 2);
			videoDecoder.AudioFrame(audioFrames, 2);
			if (!audioFrames.empty() && AudioManager::getImplementation()->straightPlay(std::move(audioFrames.front().audio)))
				audioFrames.pop();
		}
	}, 1);
}

void Wallicia::VideoClose()
{
	if (keepDecoding) {
		keepDecoding = false;
		ThreadManager::WaitForThread(1);
		videoDecoder.Clean();
		AudioManager::getImplementation()->straightPlayReset();
		videoTexture->updateData({}); // Clear out old data
		videoFrames = {};
		audioFrames = {};
	}
}

void Wallicia::ProjectionSetup()
{
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
}

void Wallicia::RendererSetup()
{
	// Get a default shader, create quad with it and add it to the renderer
	videoTexture = std::make_shared<Texture>(64, 64, true); // Streaming texture needs initial size larger than 0 to avoid issues
	videoRenderable = std::make_shared<Renderable>(ModelManager::CreateQuad(vector3(1.0f)), ShaderManager::Get("Default/Video"), videoTexture);
	RendererManager::getRenderer()->renderableAdd(videoRenderable);
}

void Wallicia::Begin()
{
	// ImGui Wallicia Control Window
#ifdef SE_OS_WINDOWS
	controlWindow = WindowManager::Add(std::make_unique<Win32_Window>("Wallicia Control", 500, 500, eWindowFlag_Windowed | eWindowFlag_OGLContext | eWindowFlag_StartCenter | eWindowFlag_NoDecorations | eWindowFlag_SystemTray));
#else
	controlWindow = WindowManager::Add(std::make_unique<WL_Window>("Wallicia Control", 500, 500, eWindowFlag_Windowed | eWindowFlag_OGLContext | eWindowFlag_StartCenter | eWindowFlag_NoDecorations | eWindowFlag_SystemTray));
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

	// Control for main window
	if (!vulkanMode) WindowManager::GetCurrentWithContext()->gainGLContext();

	// Setup renderer projection matrices
	ProjectionSetup();
	// Setup renderer resources
	RendererSetup();
}

void Wallicia::Tick()
{
}

bool canMove = false;
maths::tvector<2, int> oPos;
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

		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && canMove) {
#ifdef SE_OS_WINDOWS
			POINT p = {};
			GetCursorPos(&p);
			controlWindow->setPosition(p.x - oPos.x, p.y - oPos.y);
#endif
		}
	}
}

void Wallicia::PreRender()
{
}

void Wallicia::Render()
{
	while (!videoFrames.empty()) {
		videoTexture->updateData(std::move(videoFrames.front().data));
		videoFrames.pop();
	}

	if (controlWindow != nullptr && controlWindow->isShowing() && !controlWindow->Closed()) {
		// Let Wallicia control window take GL context for drawing ImGui
		controlWindow->gainGLContext();

		// Manual clearing of color buffer to prevent winning at solitaire
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplSE_NewFrame();
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