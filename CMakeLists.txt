include(./Engine/.env OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.11.0)

set(NAME PXT_Engine)

# MinGW
message(STATUS "using ${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
  if (NOT MINGW_PATH) 
    message(FATAL_ERROR "MINGW_PATH not set in .env")
  endif()
  set(USE_MINGW "True")
  set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
  set(CMAKE_CXX_COMPILER  ${MINGW_PATH}/bin/g++.exe)
endif()

project(${NAME} VERSION 0.0.1)

# Vulkan
if (DEFINED VULKAN_SDK_PATH)
  set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
  set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
  set(Vulkan_FOUND "True")
else()
  find_package(Vulkan REQUIRED)
  message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
  message(FATAL_ERROR "Could not find Vulkan library!")
else()
  message(STATUS "Using Vulkan library at: ${Vulkan_LIBRARIES}")
endif()

# Vendor libraries
add_subdirectory(Engine/vendor/glfw)         
add_subdirectory(Engine/vendor/glm)          
add_subdirectory(Engine/vendor/tinyobjloader)  
add_subdirectory(Engine/vendor/entt)

# Tracy Profiler
option(TRACY_ENABLE " " ON)
add_subdirectory(Engine/vendor/tracy) # target : TracyClient or alias Tracy::TracyClient

# STB IMAGE library
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_SOURCE_DIR}/Engine/vendor/stb)


# IMGUI Library
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/Engine/vendor/imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

# Add ImGui source files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

# Create ImGui library and add Vulkan include directory for its sources
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${Vulkan_INCLUDE_DIRS})

# Link dependencies to ImGui
target_link_libraries(imgui PUBLIC glfw ${Vulkan_LIBRARIES}/vulkan-1.lib)

####

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/Application/src/*.cpp ${PROJECT_SOURCE_DIR}/Engine/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/out")

if (WIN32)
  message(STATUS "CREATING BUILD FOR WINDOWS")

  if (USE_MINGW)
    target_include_directories(${PROJECT_NAME} PUBLIC
      ${MINGW_PATH}/include
    )
    target_link_directories(${PROJECT_NAME} PUBLIC
      ${MINGW_PATH}/lib
    )
  endif()

  # Include and link Vulkan and other submodule libraries
  target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/Engine/src
    ${PROJECT_SOURCE_DIR}/Application/src
    ${Vulkan_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/Engine/vendor/entt/single_include
  )
  
  # Ensure you add the Vulkan SDK library directory for MinGW
  target_link_directories(${PROJECT_NAME} PUBLIC
    ${Vulkan_LIBRARIES}
  )

  # Link everything to the target executable
  target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw                     
    glm                      
    tinyobjloader
    stb
    vulkan-1  # This is the name of the Vulkan library
    imgui
    Tracy::TracyClient
  )


elseif (UNIX)
  message(STATUS "CREATING BUILD FOR UNIX")
  target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/Engine/src
    ${PROJECT_SOURCE_DIR}/Application/src
  )
  target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw 
    ${Vulkan_LIBRARIES}
    imgui
    stb
  )
endif()

############## SHADERS ##############

message(STATUS "Using Vulkan SDK Path: ${VULKAN_SDK_PATH}")
message(STATUS "Using Vulkan SDK: $ENV{VULKAN_SDK} , ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} ")

# Find all vertex and fragment sources within shaders directory
find_program(GLSL_VALIDATOR glslangValidator HINTS 
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
  /usr/bin 
  /usr/local/bin 
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin/ 
  $ENV{VULKAN_SDK}/Bin32/
)

message(STATUS "Found glslangValidator at: ${GLSL_VALIDATOR}")

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/assets/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/assets/shaders/*.vert"
  "${PROJECT_SOURCE_DIR}/assets/shaders/*.comp"
  "${PROJECT_SOURCE_DIR}/assets/shaders/*.geom"
  "${PROJECT_SOURCE_DIR}/assets/shaders/*.tesc"
  "${PROJECT_SOURCE_DIR}/assets/shaders/*.tese"
  "${PROJECT_SOURCE_DIR}/assets/shaders/raytracing/*.rgen"
  "${PROJECT_SOURCE_DIR}/assets/shaders/raytracing/*.rahit"
  "${PROJECT_SOURCE_DIR}/assets/shaders/raytracing/*.rchit"
  "${PROJECT_SOURCE_DIR}/assets/shaders/raytracing/*.rmiss"
  "${PROJECT_SOURCE_DIR}/assets/shaders/raytracing/*.rint"
  "${PROJECT_SOURCE_DIR}/assets/shaders/raytracing/*.rcall"
)

file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/out/shaders)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/out/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV} -I${PROJECT_SOURCE_DIR}/assets/shaders --target-env vulkan1.3
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

# Add Shaders as dependency of executable
add_dependencies(${PROJECT_NAME} Shaders)