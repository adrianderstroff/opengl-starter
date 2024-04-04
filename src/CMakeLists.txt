#-----------------------------------------------------------------------------#
# list of all source files
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.h")
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS "${ASSETS_DIR}/shaders/*.glsl")

#-----------------------------------------------------------------------------#
# list all files that will either be used for compilation or that should show
# up in the ide of your choice
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
GroupSourcesByFolder(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC src)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11) # use c++11

file(RELATIVE_PATH
  ASSETS_LOCATION
  ${CMAKE_INSTALL_PREFIX}/bin
  ${ASSETS_INSTALL_DIR}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE ASSETS_PATH="${ASSETS_LOCATION}")

if(MSVC)
  set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY VS_DEBUGGER_COMMAND ${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:${CMAKE_PROJECT_NAME}>)
  set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
endif()

# specify libraries to link with after compilation
target_link_libraries(${CMAKE_PROJECT_NAME}
  PRIVATE
  glfw
  ${GLAD_LIBRARY}
  glm)

install(TARGETS ${CMAKE_PROJECT_NAME}
EXPORT ${CMAKE_PROJECT_NAME}-targets
RUNTIME DESTINATION bin
ARCHIVE DESTINATION lib
LIBRARY DESTINATION lib)