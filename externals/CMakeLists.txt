include(FetchContent)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(Findassimp)
set_target_properties(assimp PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(softrend-cpp PUBLIC "$<LINK_LIBRARY:WHOLE_ARCHIVE,assimp>")