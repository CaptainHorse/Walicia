# Basic CMake stuff #
project(Wallicia)

if (MSVC)
  cmake_minimum_required(VERSION 3.15)
else()
  cmake_minimum_required(VERSION 3.14)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Singularity Engine Subdirectory #
add_subdirectory("Submodules/singularity_engine/")

# Wallicia #
set(PROJECT_LINK_LIBS SE_application)

# Executable
add_executable(Wallicia "")

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
)

# Libs
target_link_libraries(Wallicia 
  PRIVATE 
	${PROJECT_LINK_LIBS} 
	${CMAKE_DL_LIBS}
)