#include "animation2d.hpp"

Animation2D::Animation2D() {}

Animation2D::Animation2D(std::vector<Sprite>& sprites)
    : m_sprites{ &sprites }
{
}

Sprite* Animation2D::getCurrentSprite(Timer::seconds_t eventTime) const
{
    if (m_sprites->empty())
    {
        return nullptr;
    }

    const auto one_frame_delta = 1.0 / m_fps;

    size_t numberOfFramesSinceStart =
        static_cast<size_t>(eventTime.count() / one_frame_delta);

    if ((m_mode == Mode::once) &&
        (numberOfFramesSinceStart >= m_sprites->size()))
    {
        return nullptr;
    }

    size_t currentFrameIndex = numberOfFramesSinceStart % m_sprites->size();

    return &m_sprites->at(currentFrameIndex);
}

Animation2D2Phase::Animation2D2Phase()
{
    startAnimation.setMode(Animation2D::Mode::once);
    runAnimation.setMode(Animation2D::Mode::looped);
}

Animation2D2Phase::Animation2D2Phase(std::vector<Sprite>& startSprites,
                                     std::vector<Sprite>& centerSprites)
    : startAnimation{ startSprites }
    , runAnimation{ centerSprites }
{
    startAnimation.setMode(Animation2D::Mode::once);
    runAnimation.setMode(Animation2D::Mode::looped);
}

Sprite* Animation2D2Phase::getCurrentSprite(Timer::seconds_t eventTime) const
{
    Sprite* currentSprite = startAnimation.getCurrentSprite(eventTime);
    if (!currentSprite)
    {
        currentSprite = runAnimation.getCurrentSprite(eventTime);
    }
    return currentSprite;
}

Animation2D3Phase::Animation2D3Phase()
{
    endAnimation.setMode(Animation2D::Mode::once);
}

Animation2D3Phase::Animation2D3Phase(std::vector<Sprite>& startSprites,
                                     std::vector<Sprite>& centerSprites,
                                     std::vector<Sprite>& endSprites)
    : runAnimation{ startSprites, centerSprites }
    , endAnimation{ endSprites }
{
    endAnimation.setMode(Animation2D::Mode::once);
}

Sprite* Animation2D3Phase::getCurrentSprite(Event            event,
                                            Timer::seconds_t eventTime) const
{
    Sprite* currentSprite;
    if (event == Event::Run)
    {
        currentSprite = runAnimation.getCurrentSprite(eventTime);
    }
    else
    {
        currentSprite = endAnimation.getCurrentSprite(eventTime);
    }
    return currentSprite;
}
