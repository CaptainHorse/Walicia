#include <walicia.h>

#include <walicia_control.h>
#include <imgui_impl_singularityengine.h>

#include <memory>
#include <random>

using namespace SE;
using namespace input;
using namespace maths;
using namespace graphics;

int main(int argc, char** argv) {
	ApplicationParameters params {
		"Walicia", // Application name
		true, 1280, 720, eWindowFlag_Borderless | eWindowFlag_OGLContext | eWindowFlag_NoTitlebar | eWindowFlag_DesktopWindow, // Create window, windowed resolution, flags
		true, eOpenGL, // Initialize graphics, Renderer type
		false, false, false, false, false, true // Initialize input, audio, scripting, compute, networking, threading, in that order
	};

	Walicia walicia(argc, argv, params);
	walicia.Start();

	return 0;
}

void Walicia::Setup()
{
	ImGui::CreateContext();
}

std::shared_ptr<Window> controlWindow;
std::shared_ptr<Renderable> movingQuad;

void Walicia::Begin()
{
	// ImGui Walicia Control Window
	controlWindow = WindowManager::Add(std::make_shared<Win32_Window>("Walicia Control", 648, 1024, eWindowFlag_Windowed | eWindowFlag_OGLContext | eWindowFlag_StartCenter));

	// Manual initialization of InputManager, would default to main Walicia window otherwise
	InputManager::Init(controlWindow);

	// ImGui Singularity Engine Implementation init
	ImGui_ImplSE_Init(controlWindow);

	// Regain control for main window
	WindowManager::GetCurrentWithContext()->gainGLContext();

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
	auto shader = ShaderManager::Get("GLSL/DefaultUntexturedTransform");
	movingQuad = std::make_shared<Renderable>(ModelManager::CreateQuad(vector3(0.5f)), shader);
	RendererManager::getRenderer()->renderableAdd(movingQuad);
}

void Walicia::Tick()
{
	// Random color each second
	std::random_device rd;
	std::mt19937 gen(rd());	
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	movingQuad->color.x = dis(gen);
	movingQuad->color.y = dis(gen);
	movingQuad->color.z = dis(gen);
}

void Walicia::Update()
{
	// Manual updating of InputManager
	InputManager::Update(getTime());

	// Aspect ratio aware sin/cos rotation
	if (WindowManager::GetCurrentWithContext()->getFrameWidth() > WindowManager::GetCurrentWithContext()->getFrameHeight()) {
		movingQuad->position.x = sinf(getTime()) * WindowManager::GetCurrentWithContext()->getAspectRatio();
		movingQuad->position.y = cosf(getTime());
	} else {
		movingQuad->position.x = sinf(getTime());
		movingQuad->position.y = cosf(getTime()) / WindowManager::GetCurrentWithContext()->getAspectRatio();
	}
}

void Walicia::PreRender()
{
}

void Walicia::Render()
{
}

void Walicia::PastRender()
{
	// Let Walicia control window take GLContext for drawing ImGui
	controlWindow->gainGLContext();

	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplSE_NewFrame();
	ImGui::NewFrame();

	Draw_Walicia_Control(controlWindow);

	ImGui::Render();

	ImGui_ImplSE_RenderDrawData(ImGui::GetDrawData());

	// Restore GLContext back to main window
	WindowManager::GetCurrentWithContext()->gainGLContext();
}

void Walicia::Quitting()
{
	ImGui_ImplSE_Shutdown();
	ImGui::DestroyContext();
}