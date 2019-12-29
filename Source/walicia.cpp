#include <walicia.h>

#include <memory>

using namespace SE;
using namespace graphics;

int main(int argc, char** argv) {
	ApplicationParameters params {
		"Walicia", // Application name
		true, 1000, 1000, eWindowFlag_Borderless | eWindowFlag_OGLContext | eWindowFlag_NoTitlebar | eWindowFlag_DesktopWindow, // Create window, windowed resolution, flags
		true, eOpenGL, // Initialize graphics, Renderer type
		false, false, false, false, false, true // Initialize input, audio, scripting, compute, networking, threading, in that order
	};

	Walicia walicia(argc, argv, params);
	walicia.Start();

	return 0;
}

void Walicia::Setup()
{
}

std::shared_ptr<Renderable> movingQuad;

void Walicia::Begin()
{
	auto shader = ShaderManager::Get("GLSL/DefaultUntexturedTransform");
	movingQuad = std::make_shared<Renderable>(ModelManager::CreateQuad(maths::vector3(0.25f)), shader);
	RendererManager::getRenderer()->renderableAdd(movingQuad);

	float size = 0.25f;
	auto proj = maths::matrix4x4::Orthographic(-8.0f * size, 8.0f * size, -4.5f * size, 4.5f * size, -10.0f, 10.0f);
	RendererManager::getRenderer()->setProjectionMatrix(proj);

	auto view = maths::matrix4x4::Identity();
	RendererManager::getRenderer()->setViewMatrix(view);
}

void Walicia::Tick()
{
}

void Walicia::Update()
{
	movingQuad->position.x = sinf(getTime());
	movingQuad->position.y = cosf(getTime());
}

void Walicia::Render()
{
}

void Walicia::PastRender()
{
}

void Walicia::Quitting()
{
}
