#pragma once

#include <Common/common.h>
#include <Common/common_includes.h>

class Walicia : public SE::Application
{
public:
	Walicia(int argc, char** argv, SE::ApplicationParameters parameters)
		: SE::Application(argc, argv, parameters)
	{
	}

	void Setup() override;
	void Begin() override;
	void Tick() override;
	void Update() override;
	void Render() override;
	void PastRender() override;
	void Quitting() override;
};