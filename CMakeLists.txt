cmake_minimum_required(VERSION 3.20)

project(
  Wallicia
  VERSION 0.2.0
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Singularity Engine Subdirectory #
add_subdirectory("Submodules/singularity_engine/")

# Executable
if(MSVC)
  add_executable(Wallicia ${CMAKE_CURRENT_SOURCE_DIR}/icon.rc)
else()
  add_executable(Wallicia "")
endif()

# Sources
target_sources(Wallicia
  PRIVATE
	"Source/wallicia.h"
	"Source/wallicia.cpp"
	"Source/wallicia_control.h"

	"imgui/imconfig.h"
	"imgui/imgui.h"
	"imgui/imgui.cpp"
	"imgui/imgui_demo.cpp"
	"imgui/imgui_draw.cpp"
	"imgui/imgui_internal.h"
	"imgui/imgui_tables.cpp"
	"imgui/imgui_widgets.cpp"
	"imgui/imstb_rectpack.h"
	"imgui/imstb_textedit.h"
	"imgui/imstb_truetype.h"
	"imgui/imgui_stdlib.h"
	"imgui/imgui_stdlib.cpp"
	"imgui/imfilebrowser.h"

	"imgui/imgui_impl_singularityengine.h"
	"imgui/imgui_impl_singularityengine.cpp"

	"imgui/imgui_impl_opengl3.h"
	"imgui/imgui_impl_opengl3.cpp"

	"nlohmann/json.hpp"
)

# Include folders
target_include_directories(Wallicia 
  PRIVATE
	"Source/"
	"imgui/"
	"nlohmann/"
	"Submodules/singularity_engine/Source/"
	"Submodules/singularity_engine/Submodules/glbinding/source/glbinding/include"
	"${CMAKE_BINARY_DIR}/Submodules/singularity_engine/Submodules/glbinding/source/glbinding/include"
)

# Libs
target_link_libraries(Wallicia 
  PRIVATE 
    SE::Application
	${CMAKE_DL_LIBS}
)