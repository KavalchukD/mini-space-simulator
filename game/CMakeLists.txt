cmake_minimum_required(VERSION 3.16)

project(mini-space-simulator LANGUAGES CXX)

if (SDL2_SRC_DIR)
  add_library(mini-space-simulator SHARED)
else()
  add_executable(mini-space-simulator)
endif()

target_sources(mini-space-simulator PRIVATE
    include/global.hpp
    include/environement.hpp  
    include/utilities.hpp     
    include/render_wrapper.hpp  
    include/render_objects.hpp 
    include/animation2d.hpp   
    include/sprite.hpp 
    include/imgui_wrapper.hpp    
    include/audio_wrapper.hpp 
    include/world.hpp
    include/world_physics.hpp
    include/world_objects.hpp
    include/collision_detection.hpp
    include/world_constants.hpp
    
    ../engine/include/matrix.hpp
    ../engine/include/engine_handler.hpp
    
    src/main.cpp
    src/global.cpp
    src/environement.cpp
    src/utilities.cpp  
    src/render_wrapper.cpp
    src/render_objects.cpp
    src/animation2d.cpp
    src/sprite.cpp
    src/imgui_wrapper.cpp
    src/audio_wrapper.cpp 
    src/world.cpp
    src/world_objects.cpp
    src/collision_detection.cpp

    res/shaders/game_vertex_shader.vert
    res/shaders/game_fragment_shader.frag

    res/textures/background_nasa_photo.png
    res/textures/background_gameover.png
    res/textures/proc_sheet_nebula_transp.png
    res/textures/blue_star.png
    res/textures/planet.png
    res/textures/asteroid.png
    res/textures/topdownfighter.png
    res/textures/trail_cloud.png
    res/textures/bullet.png

    res/textures/flame_blueish_flame.png
    res/textures/flame_fire.png
    res/textures/explosion.png
    res/textures/hit.png
        
    res/sounds/user_rocket_sound.wav
    res/sounds/enemy_rocket_sound.wav
    res/sounds/cosmic_music.wav
    res/sounds/explosion.wav
    res/sounds/hit.wav
    res/sounds/shoot.wav
    res/sounds/cosmic_gameover.wav
    )

target_compile_features(mini-space-simulator PUBLIC cxx_std_17)

if(CMAKE_BUILD_TYPE MATCHES Debug)
target_compile_definitions(mini-space-simulator PRIVATE "-DDEBUG_CONFIGURATION")
endif()

target_include_directories (mini-space-simulator
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_compile_options(mini-space-simulator PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -fsanitize=leak -pg>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # using regular Clang or AppleClang
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

target_link_libraries(
        mini-space-simulator
        PRIVATE
        engine_lib
)

if (SDL2_SRC_DIR)
  target_link_libraries(
          mini-space-simulator
          PRIVATE
          -llog
  )
endif()


set (source_resources "${CMAKE_CURRENT_SOURCE_DIR}/res")

if (SDL2_SRC_DIR)
  set (destination_resources "${CMAKE_CURRENT_SOURCE_DIR}/../android-project/app/src/main/assets/res")
else()
  set (destination_resources "${CMAKE_CURRENT_BINARY_DIR}/res")
endif()

add_custom_command(
 TARGET mini-space-simulator POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy_directory ${source_resources} ${destination_resources}
 DEPENDS ${destination_resources}
 COMMENT "copy resources folder ${source_resources}->${destination_resources}"
)






