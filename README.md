# Mini space simulator
  
[![pipeline status](https://gitlab.com/kavalchuk.d.v/mini-space-simulator/badges/master/pipeline.svg)](https://gitlab.com/kavalchuk.d.v/mini-space-simulator/-/commits/master)

Mini space simulator (MSP) - free 2D game about traveling in space on rocket. Based on "almost" realistic physics include gravity and physical-based control of the rocket. 
The goal of game is to last as long as you can in virtual world and and enjoy the process=). I hope you will. 
Game contains its own tiny engine based on free components such as SDL2, OpenGL, Dear Imgui.

##Controls
Ship control based on physical laws therefore your commands just control enabling and disabling engines. Engines apply some force to rocket and rocket moves. 
In order to survive you should remember that every objects in game have inertion so you can't just stop moving or rotating in one monent.

Keys:

Rocket controls:
- W     - move forward
- A     - rotate left (counterclock-wise)
- D     - rotate right (clockwise)
- SPACE - attack

Game controls:
- mouse wheel rolling - changing scale of view of the game world
- middle mouse button - reset scale to initial value.
- ESC - pause/unpause game

Game control panel:
- Stabiliz 1 - enable stabilization level 1 (double thrust for rotation in order counter to current rotation order)
- Stabiliz 2 - enable stabilization level 2 (auto stabilization)

## Supported platforms
- Linux
- Windows
- Android

## Usage
Building and run on Linux:

    cd [repositories-folder]
    git clone https://gitlab.com/kavalchuk.d.v/mini-space-simulator.git
    cd mini-space-simulator
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ./build/game
    
For building on Windows you should use MSVC compiler because mingw doesnt support std::thread.
The most convenient is using [VCPKG](https://github.com/microsoft/vcpkg).

For building for Android you should download source files of SDL2.0.12 and also create and fill the file "local.properties". Using Android studio is recommended.

## Gallery
![first_plan](screenshots/rocket_first_plan.png)
![star](screenshots/star.png)
![prebangs](screenshots/prebangs.png)
![bangs](screenshots/bangs.png)
