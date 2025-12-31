cmake_minimum_required(VERSION 3.28)

project(software_renderer LANGUAGES C CXX)

set(${CMAKE_CXX_STANDARD} 17)
set(${CMAKE_CXX_STANDARD_REQUIRED} TRUE)

set(SRC_PATH			"${CMAKE_SOURCE_DIR}/src")
set(LIB_INCLUDE_PATH	"${CMAKE_SOURCE_DIR}/externals/includes")

add_executable(software_renderer EXCLUDE_FROM_ALL
    "src/Entry.cpp"
	"src/jawl/jawl.cpp"
    "src/jawl/base_window.cpp"
	"src/jawl/input.cpp"
	"src/jawl/win32/win32_window.cpp"
	"src/jawl/win32/win32_input.cpp"
	"src/IShader.cpp"
    "src/math.cpp"
    "src/Matrix.cpp"
    "src/Model.cpp"    
    "src/Image.cpp"
    "src/Renderer.cpp"
    "src/Camera.cpp"
    # "src/SceneManagement/Scenes/Scene_Triangle.cpp"
    # "src/SceneManagement/Scenes/Scene_Cube.cpp"
    "src/SceneManagement/Scenes/Scene_Suzanne.cpp"
    # "src/SceneManagement/Scenes/Scene_AlphaBlending.cpp"
	# "src/SceneManagement/Scenes/Scene_OpenGL.cpp"


	"src/glad.c"
)

target_include_directories(software_renderer PUBLIC "${SRC_PATH}" "${LIB_INCLUDE_PATH}")

if(WIN32)
	if(MSVC)
		string(REGEX MATCH "xinput.lib" MATCHED_SUBSTRING "${CMAKE_CXX_STANDARD_LIBRARIES}")
		if(NOT ${MATCHED_SUBSTRING} STREQUAL "")
			message("XInput library already exists in cache!")
		else()
			set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} xinput.lib" CACHE STRING "Libraries linked by default with all C++ applications." FORCE)
		endif()

	else()
		string(REGEX MATCH "-lxinput" MATCHED_SUBSTRING "${CMAKE_CXX_STANDARD_LIBRARIES}")
		if(NOT ${MATCHED_SUBSTRING} STREQUAL "")
			message("XInput library already exists in cache!")
		else()
			set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lxinput" CACHE STRING "Libraries linked by default with all C++ applications." FORCE)
		endif()
	endif()
endif()

# Copy the resources folder into the generated executable's folder
add_custom_command(
	TARGET software_renderer 
	POST_BUILD 
	COMMAND ${CMAKE_COMMAND}
	-E copy_directory_if_different
	"${CMAKE_CURRENT_SOURCE_DIR}/resources"	
	"$<TARGET_FILE_DIR:software_renderer>/resources"
)