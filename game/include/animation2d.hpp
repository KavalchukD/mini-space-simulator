#pragma once
#include "iengine.hpp"
#include "sprite.hpp"
#include "utilities.hpp"
#include <vector>

class Animation2D
{
public:
    enum class Mode
    {
        once,
        looped
    };

    Animation2D();
    Animation2D(std::vector<Sprite>& sprites);

    void sprites(std::vector<Sprite>& sprites) { m_sprites = &sprites; }

    float getFps() const { return m_fps; }
    void  setFps(float fps) { m_fps = fps; }

    Mode getMode() const { return m_mode; }
    void setMode(Mode mode) { m_mode = mode; }

    Sprite* getCurrentSprite(Timer::seconds_t eventTime) const;

private:
    std::vector<Sprite>* m_sprites{};
    float                m_fps{};
    Mode                 m_mode{};
};

class Animation2D2Phase
{
public:
    Animation2D2Phase();

    Animation2D2Phase(std::vector<Sprite>& startSprites,
                      std::vector<Sprite>& centerSprites);

    void sprites(std::vector<Sprite>& startSprites,
                 std::vector<Sprite>& centerSprites)
    {
        startAnimation.sprites(startSprites);
        runAnimation.sprites(centerSprites);
    }

    float getFps() const { return m_fps; }
    void  setFps(float fps)
    {
        m_fps = fps;
        startAnimation.setFps(m_fps);
        runAnimation.setFps(m_fps);
    }

    Sprite* getCurrentSprite(Timer::seconds_t eventTime) const;

private:
    Animation2D startAnimation{};
    Animation2D runAnimation{};
    float       m_fps{};
};

class Animation2D3Phase
{
public:
    enum class Event
    {
        Run,
        Stop
    };

    Animation2D3Phase();

    Animation2D3Phase(std::vector<Sprite>& startSprites,
                      std::vector<Sprite>& centerSprites,
                      std::vector<Sprite>& endSprites);

    void sprites(std::vector<Sprite>& startSprites,
                 std::vector<Sprite>& centerSprites,
                 std::vector<Sprite>& endSprites)
    {
        runAnimation.sprites(startSprites, centerSprites);
        endAnimation.sprites(endSprites);
    }

    float getFps() const { return m_fps; }
    void  setFps(float fps)
    {
        m_fps = fps;
        runAnimation.setFps(m_fps);
        endAnimation.setFps(m_fps);
    }

    Sprite* getCurrentSprite(Event event, Timer::seconds_t eventTime) const;

private:
    Animation2D2Phase runAnimation;
    Animation2D       endAnimation{};
    float             m_fps{};
};
