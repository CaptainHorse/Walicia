# Basic CMake stuff #
if (MSVC)
  cmake_minimum_required(VERSION 3.15)
else()
  cmake_minimum_required(VERSION 3.14)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Singularity Engine Subdirectory #
add_subdirectory("Submodules/singularity_engine")

# Walicia #
set(PROJECT_LINK_LIBS SE_application)

# Executable
add_executable(Walicia "")

# Sources
target_sources(Walicia
  PRIVATE
	"Source/walicia.h"
	"Source/walicia.cpp"
)

# Include folders
target_include_directories(Walicia 
  PRIVATE
	"Source/"
	"Submodules/singularity_engine/Source"
)

# Libs
target_link_libraries(Walicia 
  PRIVATE 
	${PROJECT_LINK_LIBS} 
	${CMAKE_DL_LIBS}
)