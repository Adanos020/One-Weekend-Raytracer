cmake_minimum_required(VERSION 3.7...3.13)

project(One-Weekend-Raytracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("include")
file(GLOB_RECURSE SOURCES
    "src/*.cpp" "src/*/*.cpp"
    "include/*.hpp" "include/*/*.hpp")

add_executable(One-Weekend-Raytracer ${SOURCES})

if(UNIX)
	target_link_libraries(One-Weekend-Raytracer PUBLIC pthread vulkan shaderc_combined)
elseif(WIN32)
	find_library(SHADERC shaderc_combined)
	find_package(glm CONFIG REQUIRED)
	find_package(Vulkan REQUIRED)
	find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
	target_link_libraries(One-Weekend-Raytracer PRIVATE
		glm
		Vulkan::Vulkan
		${SHADERC})
endif()