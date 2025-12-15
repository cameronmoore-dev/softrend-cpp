cmake_minimum_required(VERSION 3.28)

project(software_renderer LANGUAGES CXX)

set(${CMAKE_CXX_STANDARD} 17)
set(${CMAKE_CXX_STANDARD_REQUIRED} TRUE)

add_executable(software_renderer EXCLUDE_FROM_ALL
    "src/Entry.cpp"
    "src/Window.cpp"
	"src/Input.cpp"
    "src/Utils.cpp"
    "src/Matrix.cpp"
    "src/Model.cpp"    
    "src/Image.cpp"
    "src/Renderer.cpp"
    "src/Camera.cpp"
    "src/SceneManagement/Scenes/Scene_Cube.cpp"
    "src/SceneManagement/Scenes/Scene_Triangle.cpp"
    "src/SceneManagement/Scenes/Scene_Suzanne.cpp"
    "src/SceneManagement/Scenes/Scene_AlphaBlending.cpp"
)

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