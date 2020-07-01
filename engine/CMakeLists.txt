cmake_minimum_required(VERSION 3.16)

if(SDL2_SRC_DIR)
    add_subdirectory( # Specifies the directory of the CMakeLists.txt file.
                     ${SDL2_SRC_DIR}
                     # Specifies the directory for the build outputs.
                     ${SDL2_BUILD_DIR}_${ANDROID_ABI}
                    )
endif()

project(mini-space-simulator LANGUAGES CXX C)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/imgui-src)

#Should be static for ImGui
add_library(engine_lib STATIC
    include/iengine.hpp
    include/engine_handler.hpp  
    include/engine_sdl.hpp
    include/imgui_engine.hpp
    include/audio_engine.hpp
    include/gltexture.hpp  
    include/glprogram.hpp  
    include/vertex.hpp
    include/opengl_debug.hpp
    include/matrix.hpp
    include/picopng.hxx

    src/engine_handler.cpp
    src/engine_sdl.cpp
    src/imgui_engine.cpp
    src/audio_engine.cpp
    src/gltexture.cpp
    src/glprogram.cpp
    src/vertex.cpp
    src/opengl_debug.cpp


    glad/include/glad/glad.h
    glad/include/KHR/khrplatform.h
    glad/src/glad.c
    shaders/imgui_vertex_shader.vert
    shaders/imgui_fragment_shader.frag
    )

set_property(TARGET engine_lib PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(engine_lib PROPERTIES ENABLE_EXPORTS TRUE)
target_compile_features(engine_lib PUBLIC cxx_std_20)

target_compile_options(engine_lib PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -fsanitize=leak -pg>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
        )

target_link_libraries(engine_lib PUBLIC imgui)

if(CMAKE_BUILD_TYPE MATCHES Debug)
target_compile_definitions(engine_lib PRIVATE "-DDEBUG_CONFIGURATION")
endif()

if(WIN32)
  target_compile_definitions(engine_lib PRIVATE "-DOM_DECLSPEC=__declspec(dllexport)")
endif(WIN32)

if(SDL2_SRC_DIR)
find_library(SDL2_LIB NAMES SDL2)
else()
find_package(SDL2)
endif()

target_include_directories (engine_lib 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-src
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/glad
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/KHR)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # using regular Clang or AppleClang
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()


if (MINGW)
    target_link_libraries(engine_lib PRIVATE
               -lSDL2
               )
elseif(APPLE)
    target_link_libraries(engine_lib PRIVATE
      -Wl,-framework,OpenGL

      )
elseif(UNIX  OR MSVC)
    if (SDL2_SRC_DIR)
        target_link_libraries(
                engine_lib
                PRIVATE
                SDL2
                SDL2main
        )
    else(SDL2_SRC_DIR)
        target_link_libraries(
                engine_lib
                PRIVATE
                SDL2::SDL2
                SDL2::SDL2main
        )
    endif()
endif()


set (source_shaders "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

set (destination_shaders_additional "${CMAKE_CURRENT_SOURCE_DIR}/../game/res/shaders")
if (SDL2_SRC_DIR)
    set (destination_shaders "${CMAKE_CURRENT_SOURCE_DIR}/../android-project/app/src/main/assets/res/shaders")
else(SDL2_SRC_DIR)
    set (destination_shaders "${CMAKE_BINARY_DIR}/game/res/shaders") 
endif()

add_custom_command(
 TARGET engine_lib POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy_directory ${source_shaders} ${destination_shaders}
 COMMAND ${CMAKE_COMMAND} -E copy_directory ${source_shaders} ${destination_shaders_additional}
 DEPENDS ${destination}
 COMMENT "copy resources folder from ${source_shaders} => ${destination_shaders}"
)






