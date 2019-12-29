# Basic CMake stuff #
if (MSVC)
  cmake_minimum_required(VERSION 3.15)
else()
  cmake_minimum_required(VERSION 3.14)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Singularity Engine Subdirectory #
add_subdirectory("Submodules/singularity_engine/")

# Walicia #
set(PROJECT_LINK_LIBS SE_application)

# Executable
add_executable(Walicia "")

# Sources
target_sources(Walicia
  PRIVATE
	"Source/walicia.h"
	"Source/walicia.cpp"

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

	"imgui/imgui_impl_singularityengine.h"
	"imgui/imgui_impl_singularityengine.cpp"

	"imgui/imgui_impl_opengl3.h"
	"imgui/imgui_impl_opengl3.cpp"
)

# Include folders
target_include_directories(Walicia 
  PRIVATE
	"Source/"
	"imgui/"
	"Submodules/singularity_engine/Source/"
)

# Libs
target_link_libraries(Walicia 
  PRIVATE 
	${PROJECT_LINK_LIBS} 
	${CMAKE_DL_LIBS}
)