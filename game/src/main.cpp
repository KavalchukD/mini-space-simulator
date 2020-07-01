#include "audio_wrapper.hpp"
#include "environement.hpp"
#include "render_wrapper.hpp"
#include "utilities.hpp"
#include "world.hpp"
#include <engine_handler.hpp>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_set>

#ifdef __ANDROID__
#include <android/log.h>
extern "C" int android_main(int argc, char* argv[]);
#endif

int internal_main(int /*argc*/, char* /*argv*/[])
{
    using namespace om;
    constexpr auto             engineType = IEngine::EngineTypes::sdl;
    constexpr std::string_view gameTitle{ "Mini space simulator" };
    constexpr std::string_view config{};
    EngineHandler              engine(engineType, gameTitle, config);

    Environement  environement;
    RenderWrapper renderWrapper{ *engine, { 0, 1, 0 }, 0.05 };
    ImguiWrapper  imguiWrapper{ *engine };
    AudioWrapper  audioWrapper{ *engine };

// Type enter to reset game, esc to pause
RESET:
    Timer gameTime;

    Model::World world{ gameTime.timerNow() };

    bool isContinueLoop = true;
    bool isGameOver     = false;

    int loopCount{};

    while (isContinueLoop)
    {
        Timer                      loopTimer;
        Model::World::WorldEvents* worldEvents;
        Environement::eventSet     environementEvents;

        Timer tempTimer;
        isContinueLoop =
            environement.input(*engine, worldEvents, environementEvents);
        [[maybe_unused]] const auto inputTime = tempTimer.elapsed().count();
        tempTimer.reset();

        environement.handleEnvironementEvents(*engine, environementEvents);
        [[maybe_unused]] const auto envEventsTime = tempTimer.elapsed().count();
        tempTimer.reset();

        if (!isGameOver)
        {
            if (environement.isPause())
            {
                gameTime.pause();
            }
            gameTime.proceed();
            isGameOver = !world.update(gameTime.timerNow(), *worldEvents);
            audioWrapper.play(world);
            renderWrapper.render(world);
            imguiWrapper.createImguiObjects(world);
        }
        else
        {
            gameTime.pause();
            audioWrapper.playGameOver();
            renderWrapper.renderGameOver();
            std::clog << "Game over!" << std::endl;
        }

        [[maybe_unused]] const auto worldUpdateTime =
            tempTimer.elapsed().count();
        tempTimer.reset();

        engine->updateWindow({ 0.f, 0.0f, 0.0f, 0.f });

        [[maybe_unused]] const auto renderTime = tempTimer.elapsed().count();
        tempTimer.reset();

#ifdef DEBUG_CONFIGURATION
        if (loopCount == 20)
        {
            std::clog << world;
            std::clog << " Full time: " << loopTimer.elapsed().count()
                      << " Input time: " << inputTime
                      << " EnviromentEventsTime: " << envEventsTime
                      << " WorldUpdateTime: " << worldUpdateTime
                      << " RenderTime : " << renderTime << '\n';
            loopCount = 0;
        }
#endif

        normalizeLoopDuration(loopTimer.elapsed(),
                              engine->getDisplayRefreshRate());
        ++loopCount;
        if (environement.isReset())
        {
            goto RESET;
        }
    }

    return EXIT_SUCCESS;
}

#ifndef __ANDROID__
int main(int argc, char* argv[])
{
    return internal_main(argc, argv);
}
#endif

#ifdef __ANDROID__
class android_redirected_buf : public std::streambuf
{
public:
    android_redirected_buf() = default;

private:
    // This android_redirected_buf buffer has no buffer. So every character
    // "overflows" and can be put directly into the teed buffers.
    int overflow(int c) override
    {
        if (c == EOF)
        {
            return !EOF;
        }
        else
        {
            if (c == '\n')
            {
#ifdef __ANDROID__
                // android log function add '\n' on every print itself
                __android_log_print(ANDROID_LOG_ERROR, "OM", "%s",
                                    message.c_str());
#else
                std::printf("%s\n", message.c_str()); // TODO test only
#endif
                message.clear();
            }
            else
            {
                message.push_back(static_cast<char>(c));
            }
            return c;
        }
    }

    int sync() override { return 0; }

    std::string message;
};

int android_main(int argc, char* argv[])
{
    auto cout_buf = std::cout.rdbuf();
    auto cerr_buf = std::cerr.rdbuf();
    auto clog_buf = std::clog.rdbuf();

    android_redirected_buf logcat;

    std::cout.rdbuf(&logcat);
    std::cerr.rdbuf(&logcat);
    std::clog.rdbuf(&logcat);
    int result;
    try
    {
        result = internal_main(argc, argv);
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        result = EXIT_FAILURE;
    }
    std::cout.rdbuf(cout_buf);
    std::cerr.rdbuf(cerr_buf);
    std::clog.rdbuf(clog_buf);
    return result;
}

#endif
