#pragma once

#include <walicia.h>

#include <imgui.h>

using namespace SE;
using namespace graphics;

static bool open = true;
void Draw_Walicia_Control(std::shared_ptr<Window> ctrl)
{
	ImGui::SetNextWindowSize(ImVec2(ctrl->getWidth(), ctrl->getHeight()));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("Walicia Control", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);

	auto clear = RendererManager::getRenderer()->getClearColor();
	ImGui::ColorEdit4("Background Color", clear.elements());
	RendererManager::getRenderer()->setClearColor(clear);

	ImGui::End();
}