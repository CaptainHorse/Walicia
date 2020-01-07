#pragma once

#include <Common/common.h>
#include <Common/common_includes.h>

class Wallicia : public SE::Application
{
private:
	static SE::graphics::VideoDecoder dec;

public:
	Wallicia(int argc, char** argv, SE::ApplicationParameters parameters)
		: SE::Application(argc, argv, parameters)
	{
	}

	static void VideoOpen(const std::string& path, const bool& sound = true);
	static void VideoClose();

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