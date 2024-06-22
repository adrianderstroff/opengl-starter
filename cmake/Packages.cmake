Set(FETCHCONTENT_QUIET FALSE)

CPMAddPackage(
  NAME glfw
  VERSION 3.3.10
  GITHUB_REPOSITORY glfw/glfw
  GIT_TAG 3.3.10
  OPTIONS "BUILD_SHARED_LIBS ON"
          "GLFW_BUILD_EXAMPLES OFF"
          "GLFW_BUILD_TESTS OFF"
          "GLFW_BUILD_DOCS OFF"
)
find_targets(glfw_targets ${glfw_SOURCE_DIR})
make_folder("glfw" ${glfw_targets})

CPMAddPackage(
  NAME glad
  GITHUB_REPOSITORY Dav1dde/glad
  VERSION 2.0.6
  DOWNLOAD_ONLY
)
set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${glad_SOURCE_DIR}/cmake)
include(GladConfig)
add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
set(GLAD_LIBRARY glad_gl_core_46)
# https://github.com/Dav1dde/glad/wiki/C#generating-during-build-process
glad_add_library(${GLAD_LIBRARY} SHARED API gl:core=4.6)
make_folder("glad" ${GLAD_LIBRARY})
install(TARGETS ${GLAD_LIBRARY}
  EXPORT ${GLAD_LIBRARY}-targets
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)

CPMAddPackage(
  NAME glm
  GITHUB_REPOSITORY g-truc/glm
  GIT_TAG 1.0.1
)
find_targets(glm_targets ${glm_SOURCE_DIR})
make_folder("glm" ${glm_targets})

# To fix build errors in Qt Creator for the latest versions of CMake.
set(CMP0169 OLD)
# Source file grouping of visual studio and xcode
CPMAddPackage(
  NAME GroupSourcesByFolder.cmake
  GITHUB_REPOSITORY TheLartians/GroupSourcesByFolder.cmake
  VERSION 1.0
)
