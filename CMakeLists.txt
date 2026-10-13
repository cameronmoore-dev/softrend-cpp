cmake_minimum_required(VERSION 3.28)

project(softrend-cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_PATH			"${CMAKE_SOURCE_DIR}/src")
set(LIB_INCLUDE_PATH	"${CMAKE_SOURCE_DIR}/externals/includes")
set(LIB_PATH			"${CMAKE_SOURCE_DIR}/externals/libs")

add_executable(softrend-cpp
	"src/Entry.cpp"
	"src/AppContext.cpp"
    "src/Renderer/renderer.cpp"
	"src/Renderer/shader.cpp"
    "src/Renderer/math.cpp"
    "src/Model.cpp"    
    # "src/Image.cpp"
    "src/Camera.cpp"
	"src/SceneManagement/Scenes/Scene_MainMenu.cpp"
    "src/SceneManagement/Scenes/Scene_Triangle.cpp"
    "src/SceneManagement/Scenes/Scene_Cube.cpp"
    "src/SceneManagement/Scenes/Scene_Suzanne.cpp"
    "src/SceneManagement/Scenes/Scene_AlphaBlending.cpp"
	"src/SceneManagement/Scenes/Scene_Assimp.cpp"
	
	"src/assimp_model.cpp"
	"src/assimp_mesh.cpp"
	
	"src/stb_image.cpp"
)

if (WIN32)
	target_sources(softrend-cpp PUBLIC 
		src/os/win32/win32_window.c
		src/os/win32/win32_input.c
		src/os/win32/win32_time.c
	)
	if (MSVC)
		target_link_libraries(softrend-cpp PRIVATE winmm.lib)
	else()
		target_link_libraries(softrend-cpp PRIVATE -lwinmm)
	endif()
elseif(LINUX)
	target_sources(softrend-cpp PUBLIC 
		src/os/linux/x11_window.c
		src/os/linux/x11_input.c
		src/os/linux/linux_time.c
	)
	target_link_libraries(softrend-cpp PRIVATE -lX11 -lXfixes -lX11-xcb -lxcb -ludev -levdev -lm)
endif()

target_include_directories(softrend-cpp PUBLIC "${SRC_PATH}" "${LIB_INCLUDE_PATH}")
# target_link_libraries(softrend-cpp PRIVATE ${LIB_PATH}/libassimp.dll.a)
target_compile_options(softrend-cpp PRIVATE -Wall)

add_subdirectory(externals)

# Copy the resources folder into the generated executable's folder
add_custom_command(
	TARGET softrend-cpp 
	POST_BUILD 
	COMMAND ${CMAKE_COMMAND}
	-E copy_directory_if_different
	"${CMAKE_CURRENT_SOURCE_DIR}/resources"	
	"$<TARGET_FILE_DIR:softrend-cpp>/resources"
)