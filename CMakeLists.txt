cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# specify name of the project and specify which language to compile for
project(opengl-starter LANGUAGES CXX C)

# If you do not set the CMAKE_INSTALL_PREFIX variable, the default path on Windows will point to "Program Files",
# where you cannot copy files without administrator rights.
# For easier out-of-the-box use, this variable is set manually.
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_SOURCE_DIR}/cmake)

include(CPM)
include(FindTargets)
include(MakeFolder)
include(Packages)

add_subdirectory(src)