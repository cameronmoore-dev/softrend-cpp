cmake_minimum_required(VERSION 3.28)

project(software_renderer LANGUAGES CXX)

set(${CMAKE_CXX_STANDARD} 17)
set(${CMAKE_CXX_STANDARD_REQUIRED} TRUE)

add_executable(software_renderer EXCLUDE_FROM_ALL
    "src/Entry.cpp"
    "src/Window.cpp"
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

# Copy the resources folder into the generated executable's folder
add_custom_command(
	TARGET software_renderer 
	POST_BUILD 
	COMMAND ${CMAKE_COMMAND}
	-E copy_directory_if_different
	"${CMAKE_CURRENT_SOURCE_DIR}/resources"	
	"$<TARGET_FILE_DIR:software_renderer>/resources"
)