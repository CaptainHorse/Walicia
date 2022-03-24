#pragma once

#include <filesystem>

#include <wallicia.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imfilebrowser.h>

#include <json.hpp>

#include <Utilities/fileutilities.h>

using namespace SE;
using namespace common;
using namespace input;
using namespace graphics;
using json = nlohmann::json;
namespace fs = std::filesystem;

inline json config;
inline ImGui::FileBrowser fileDialog;

static bool openControl = true;
static bool initControl = false;

inline bool withSound = true;
inline bool showConsole = true;
inline bool windowTopMost = true;
inline bool windowBuddyMode = false;
inline bool keepaspectratio = false;
inline bool windowTransparency = false;
inline bool windowFollowCursor = false;
inline maths::vector4 clearcolor = maths::vector4(0.0f);
inline maths::vector2 windowscale = maths::vector2(1.0f);
inline maths::vector2 windowPosition = maths::vector2(0.0f);
inline maths::vector2 oldWindowscale = maths::vector2(1.0f);
inline maths::tvector<2, uint> windowSize;

inline Window* window;

inline void Reset()
{
	clearcolor = maths::vector4(0.0f);
	RendererManager::getRenderer()->setClearColor(clearcolor);

	windowscale = maths::vector2(1.0f);
	window->setWindowSize(windowSize.x * windowscale.x, windowSize.y * windowscale.y);
}

inline void Save_Config()
{
	config["clearcolor"] = clearcolor.components;
	config["windowscale"] = windowscale.components;
	config["windowposition"] = windowPosition.components;

	std::ofstream file("config.json");
	file << config;
}

inline void Load_Config() 
{
	if (fs::exists("config.json")) {
		auto s = FileUtilities::read_file("config.json");
		config = json::parse(s.cbegin(), s.cend());

		if (config.contains("clearcolor")) {
			auto c = config["clearcolor"].get<std::array<float32, 4>>();
			RendererManager::getRenderer()->setClearColor(maths::vector4(c[0], c[1], c[2], c[3]));
		}

		if (config.contains("windowscale")) {
			auto s = config["windowscale"].get<std::array<float32, 2>>();
			windowscale = maths::vector2(s[0], s[1]);
			window->setWindowSize(windowSize.x * windowscale.x, windowSize.y * windowscale.y);
		}

		if (config.contains("windowposition")) {
			auto s = config["windowposition"].get<std::array<float32, 2>>();
			windowPosition = maths::vector2(s[0], s[1]);
			window->setPosition(windowPosition);
		}
	}
}

inline void Draw_Wallicia_Control(Window* ctrl)
{
	if (!openControl) {
		// Hide window
		ctrl->hide();
		openControl = true;
	} else {
		if (!initControl) {
			window = WindowManager::GetCurrentWithContext();

			windowSize = window->getWindowSize();
			windowPosition = window->getPosition();
			clearcolor = RendererManager::getRenderer()->getClearColor();

			fileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_NoTitleBar);
			fileDialog.SetWindowSize(ctrl->getWidth(), ctrl->getHeight());
			fileDialog.SetTitle("Choose Video File");
			fileDialog.SetTypeFilters({ ".mp4", ".avi", ".webm", ".mov" });

			auto f = window->getWindowFlags();
			if (f & eWindowFlag_Transparent)
				windowTransparency = true;
			else
				windowTransparency = false;

			if (f & eWindowFlag_TopMost)
				windowTopMost = true;
			else
				windowTopMost = false;

			if (f & eWindowFlag_DesktopWindow)
				windowBuddyMode = false;
			else
				windowBuddyMode = true;

			Reset();
			Load_Config(); 
			
			initControl = true;
		}

		// Force ImGui window to fit real window
		ImGui::SetNextWindowSize(ImVec2(ctrl->getWidth(), ctrl->getHeight()));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("Wallicia Control", &openControl, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		// Just some basic info
		ImGui::Text("Renderer:");
		ImGui::SameLine();

		ImGui::Text(RendererManager::getRenderer()->getRendererString().c_str());

		// TODO: Out of order
		//std::vector<std::string> items = { "OpenGL", "Vulkan" }; // TODO: Get available renderer names programmatically
		//if (ImGui::BeginCombo("##renderer", RendererManager::getRenderer()->getRendererString().c_str()))
		//{
		//	for (auto& item : items) {
		//		bool is_selected = (RendererManager::getRenderer()->getRendererString() == item);
		//		if (ImGui::Selectable(item.c_str(), is_selected)) {
		//			Wallicia::VideoClose();
		//			if (item == "Vulkan") {
		//				Wallicia::getInstance()->switchRenderer(eRendererVulkan);
		//				Wallicia::vulkanMode = true;
		//			} else if (item == "OpenGL") {
		//				//Wallicia::getInstance()->switchRenderer(eRendererOpenGL);
		//				//Wallicia::vulkanMode = false;
		//			} 
		//			Wallicia::ProjectionSetup();
		//			Wallicia::RendererSetup();
		//		}

		//		if (is_selected)
		//			ImGui::SetItemDefaultFocus();
		//	}
		//	ImGui::EndCombo();
		//}

		ImGui::SameLine(ImGui::GetWindowWidth() - 140.0f);

		// Save, Load and Reset config options
		if (ImGui::Button("Save"))
			Save_Config();

		ImGui::SameLine();
		if (ImGui::Button("Load"))
			Load_Config();

		ImGui::SameLine();
		if (ImGui::Button("Reset"))
			Reset();

		// More info
		auto [v0, v1, v2] = RendererManager::getRenderer()->getAPIVersion();
		ImGui::Text(std::format("API Version: {}.{}.{}", v0, v1, v2).c_str());
		ImGui::Text(std::format("FPS:{} / UPS:{}", Wallicia::getInstance()->getFPS(), Wallicia::getInstance()->getUPS()).c_str());

		ImGui::Separator();

#ifdef _WIN32
		// Win32 Console control
		if (ImGui::Checkbox("Show Console", &showConsole)) {
			const auto console = GetConsoleWindow();
			ShowWindow(console, showConsole);
		}
#endif

		// Window Top Most option
		if (ImGui::Checkbox("Window Top Most", &windowTopMost)) {
			auto f = window->getWindowFlags();
			windowPosition = window->getPosition();
			if (!windowTopMost)
				f &= ~eWindowFlag_TopMost;
			else
				f |= eWindowFlag_TopMost;

			window->setWindowFlags(f);
			window->setPosition(windowPosition);
			ctrl->gainGLContext();
		}

		// Window Buddy Mode option
		if (ImGui::Checkbox("Window Buddy Mode", &windowBuddyMode)) {
			auto f = window->getWindowFlags();
			windowPosition = window->getPosition();
			if (!windowBuddyMode) {
				f &= ~eWindowFlag_Windowed;
				f |= eWindowFlag_Borderless;
				f |= eWindowFlag_DesktopWindow;
			} else {
				f &= ~eWindowFlag_Borderless;
				f &= ~eWindowFlag_DesktopWindow;
				f |= eWindowFlag_Windowed;
			}

			window->setWindowFlags(f);
			window->setPosition(windowPosition);
			Wallicia::ProjectionSetup();
			ctrl->gainGLContext();
		}

		// Window transparency option
		if (ImGui::Checkbox("Window Black Transparency", &windowTransparency)) {
			auto f = window->getWindowFlags();
			windowPosition = window->getPosition();
			if (!windowTransparency)
				f &= ~eWindowFlag_Transparent;
			else
				f |= eWindowFlag_Transparent;

			window->setWindowFlags(f);
			window->setPosition(windowPosition);
			ctrl->gainGLContext();
		}

		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "- FPS heavy, keep window size small");

		ImGui::Separator();

		// Video load option
		if (ImGui::Button("Load Video"))
			fileDialog.Open();

		// Video stop option
		if (ImGui::Button("Stop Video"))
			Wallicia::VideoClose();

		// Sound check
		ImGui::Checkbox("With Sound", &withSound);

		fileDialog.Display();

		if (fileDialog.HasSelected()) {
			Wallicia::VideoClose();
			Wallicia::VideoOpen(fileDialog.GetSelected().string(), withSound);
			fileDialog.ClearSelected();
		}

		ImGui::Separator();

		// Clear color option
		if (ImGui::ColorEdit4("Clear Color", clearcolor.components.data()))
			RendererManager::getRenderer()->setClearColor(clearcolor);

		ImGui::Separator();

		if (!windowBuddyMode) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		// Window scale option
		ImGui::Checkbox("Keep Aspect Ratio", &keepaspectratio);
		if (ImGui::DragFloat2("Window Scale", windowscale.components.data(), 0.01f, 0.1f, 1000.0f)) {
			if (keepaspectratio) {
				if (windowscale.x != oldWindowscale.x)
					windowscale.y = windowscale.x;
				else
					windowscale.x = windowscale.y;
			}
			window->setWindowSize(windowSize.x * windowscale.x, windowSize.y * windowscale.y);
			oldWindowscale = windowscale;
			Wallicia::ProjectionSetup();
		}

		// Window pos options
		if (windowFollowCursor) windowPosition = window->getPosition();
		if (ImGui::DragFloat2("Window Position", windowPosition.components.data()))
			window->setPosition(windowPosition);

		ImGui::Checkbox("Have window follow cursor", &windowFollowCursor);
		if (windowFollowCursor) {
			ImGui::TextColored(ImVec4(1.0f, cos(Wallicia::getInstance()->getTime() * 1.25f), sin(Wallicia::getInstance()->getTime()), 1.0f), "Press ESC to release");
#ifdef _WIN32
			if (GetKeyState(VK_ESCAPE) & 0x8000) // Normal window based input check doesn't work if focus shifts while window is being moved
#endif
				windowFollowCursor = false;
#ifdef _WIN32
			POINT pp;
			GetCursorPos(&pp);
			pp.x -= window->getWidth() / 2;
			pp.y -= window->getHeight() / 2;
			window->setPosition(pp.x, pp.y);
#endif
		}

		if (!windowBuddyMode) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::End();
	}
}