#include <wallicia.h>

#include <memory>
#include <random>

#include <glbinding/glbinding.h>

#include <wallicia_control.h>
#include <imgui_impl_singularityengine.h>

#include <Utilities/argument_parser.h>

using namespace SE;
using namespace common;
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
int64 lastVideoPts = 0, lastAudioPts = 0;
float64 videoClock = 0.0;
std::atomic_bool reachedEnd = false;
std::vector<uint> workingThreads;

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

	uint appFlags = eInitAudio | eInitGraphics | eInitInput | eInitThreading | eCreateDefaultWindow;
	if (Wallicia::vulkanMode) 
		appFlags |= eRendererVulkan;
	else
		appFlags |= eRendererOpenGL;

	// Main window flags
	uint windowFlags;
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
		const ApplicationParameters params {
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
	const auto res = videoDecoder.Init(path, sound);
	if (sound)
		AudioManager::getImplementation()->straightPlay(res.audio); // Does initialization on first call
	
	videoClock = 0.0;
	lastVideoPts = 0;
	lastAudioPts = 0;
	reachedEnd = false;

	videoTexture->updateSize(res.width, res.height);
	RendererManager::getRenderer()->textureUpdateSize(videoTexture);

	keepDecoding = true;
	workingThreads.emplace_back(
		ThreadManager::AddJobToFreeThread([&]() {
			while (keepDecoding) {
				if (!reachedEnd && videoDecoder.Decode(10)) reachedEnd = true;
				videoDecoder.VideoFrame(videoFrames, 5);
				videoDecoder.AudioFrame(audioFrames, 5);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
	));
}

void Wallicia::VideoClose()
{
	if (keepDecoding) {
		keepDecoding = false;
		for (auto thread : workingThreads)
			ThreadManager::WaitForThread(thread);

		videoDecoder.Clean();
		AudioManager::getImplementation()->straightPlayReset();
		videoTexture->data = {}; // Clear out old data
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
	const auto proj = matrix4x4::Orthographic(left, right, bottom, top, -10.0f, 10.0f);
	RendererManager::getRenderer()->setProjectionMatrix(proj);
}

void Wallicia::RendererSetup()
{
	// Get a default shader, create quad with it and add it to the renderer
	videoTexture = std::make_shared<Texture>(64, 64, true); // Streaming texture needs initial size larger than 0 to avoid issues
	RendererManager::getRenderer()->addTexture(videoTexture);
	videoRenderable = std::make_shared<Renderable>(ModelManager::CreateQuad(vector3(1.0f)), ShaderManager::Get("Default/Video"), videoTexture);
	RendererManager::getRenderer()->renderableAdd(videoRenderable);
}

void Wallicia::Begin()
{
	// ImGui Wallicia Control Window
#ifdef _WIN32
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
#ifdef _WIN32
			POINT p = {};
			GetCursorPos(&p);
			ScreenToClient(static_cast<HWND>(controlWindow->getWindowHandle()), &p);
			oPos.x = p.x;
			oPos.y = p.y;
#endif
			canMove = true;
		}

		if (ImGui::IsMouseReleased(0))
			canMove = false;

		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && canMove) {
#ifdef _WIN32
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
	if (keepDecoding) {
		videoClock += getInstance()->getDeltatime() * 1000000.0;

		while (!videoFrames.empty() && videoClock >= lastVideoPts) {
			videoTexture->data = videoFrames.front().data;
			RendererManager::getRenderer()->textureUpdate(videoTexture);
			lastVideoPts = videoFrames.front().pts;
			videoFrames.pop();
		}

		while (!audioFrames.empty() && videoClock >= lastAudioPts && AudioManager::getImplementation()->straightPlay(audioFrames.front().audio)) {
			lastAudioPts = audioFrames.front().pts;
			audioFrames.pop();
		}
		
		// Do a forced synchronization when video loops
		if (reachedEnd && audioFrames.empty() && videoFrames.empty()) {
			videoClock = 0.0;
			lastVideoPts = 0;
			lastAudioPts = 0;
			reachedEnd = false;
		}
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