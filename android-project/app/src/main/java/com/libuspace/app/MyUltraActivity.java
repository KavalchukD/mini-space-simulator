package com.libuspace.app;

import org.libsdl.app.SDLActivity;

public class MyUltraActivity extends SDLActivity
{
    @Override
    protected String getMainFunction() {
        return "android_main";
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
                "c++_shared",
                "hidapi",
                "SDL2",
                // "SDL2_image",
                // "SDL2_mixer",
                // "SDL2_net",
                // "SDL2_ttf",
                "homework_11_1_world_update"

        };
    }
}