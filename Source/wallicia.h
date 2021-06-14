#pragma once

#include <Common/common_includes.h>
#include <Graphics/Video/video_decoder.h>

class Wallicia : public SE::Application
{
public:
	static bool buddyMode;
	static bool vulkanMode;

private:
	static std::atomic<bool> keepDecoding;
	static SE::graphics::VideoDecoder videoDecoder;
	static std::shared_ptr<SE::graphics::Texture> videoTexture;
	static std::shared_ptr<SE::graphics::Renderable> videoRenderable;

public:
	Wallicia(int argc, char** argv, SE::ApplicationParameters parameters)
		: SE::Application(argc, argv, parameters)
	{
	}

	static void VideoOpen(const std::string& path, const bool& sound = true);
	static void VideoClose();

	static void ProjectionSetup();
	static void RendererSetup();

	void Setup() override;
	void Begin() override;
	void Tick() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PastRender() override;
	void Present() override;
	void Quitting() override;
};