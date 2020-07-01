#pragma once
#include "world.hpp"
#include <forward_list>
#include <iengine.hpp>

struct RocketAudio
{
    RocketAudio() = default;
    RocketAudio(const Model::Rocket* inRocketPtr,
                om::ISoundBuffer*    inAudioBuffer)
        : rocketPtr{ inRocketPtr }
        , audioBuffer{ inAudioBuffer }
    {
    }
    const Model::Rocket* rocketPtr;
    om::ISoundBuffer*    audioBuffer;
    bool                 isEnable{};
};

class AudioWrapper
{
public:
    explicit AudioWrapper(om::IEngine& engine);
    void play(const Model::World& world);
    void playGameOver();

    static constexpr om::myGlfloat backgroundVolume     = 0.6;
    static constexpr om::myGlfloat userRocketsVolume    = 0.5;
    static constexpr om::myGlfloat enemiesRocketsVolume = 0.9;
    static constexpr om::myGlfloat explosionVolume      = 1.0;

private:
    void checkRocketAudioCongig(const Model::World& world);
    void playBackground();
    void addAllTracks();
    void playOneEvent(const Model::OutEvent& event,
                      om::myGlfloat volume = AudioWrapper::explosionVolume);

    om::ISoundTrack* m_soundTrackUserRocket;
    om::ISoundTrack* m_soundTrackEnemyRocket;
    om::ISoundTrack* m_soundTrackExplosion;
    om::ISoundTrack* m_soundTrackHit;
    om::ISoundTrack* m_soundTrackShoot;

    om::ISoundBuffer* m_soundBufferUserRocket;

    om::ISoundTrack*  m_soundTrackBackground;
    om::ISoundBuffer* m_soundBufferBackground;

    om::ISoundTrack*  m_soundTrackBackgroundGameOver;
    om::ISoundBuffer* m_soundBufferBackgroundGameOver;

    std::list<RocketAudio> rocketAudios;
    RocketAudio            userRocketAudio;

    static constexpr std::string_view pathToSoundTrackBackground{
        "res/sounds/cosmic_music.wav"
    };

    static constexpr std::string_view pathToSoundTrackBackgroundGameOver{
        "res/sounds/cosmic_gameover.wav"
    };

    static constexpr std::string_view pathToSoundTrackUserRocket{
        "res/sounds/user_rocket_sound.wav"
    };

    static constexpr std::string_view pathToSoundTrackEnemyRocket{
        "res/sounds/enemy_rocket_sound.wav"
    };

    static constexpr std::string_view pathToSoundTrackExplosion{
        "res/sounds/explosion.wav"
    };
    static constexpr std::string_view pathToSoundTrackHit{
        "res/sounds/hit.wav"
    };
    static constexpr std::string_view pathToSoundTrackShoot{
        "res/sounds/shoot.wav"
    };

    std::forward_list<const Model::Rocket*> rockets;
    const Model::Rocket*                    userRocket{};
    om::IEngine&                            m_engine;
};
