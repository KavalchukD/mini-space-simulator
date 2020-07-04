#include "audio_wrapper.hpp"
#include "global.hpp"
#include <algorithm>
#include <stdexcept>

AudioWrapper::AudioWrapper(om::IEngine& engine)
    : userRocket{}
    , m_engine{ engine }
{
    addAllTracks();
}

void AudioWrapper::addAllTracks()
{
    m_soundTrackBackground = m_engine.addSoundTrack(pathToSoundTrackBackground);
    m_soundBufferBackground = m_engine.addSoundBuffer(m_soundTrackBackground);
    m_soundBufferBackground->setVolume(backgroundVolume);

    m_soundTrackBackgroundGameOver =
        m_engine.addSoundTrack(pathToSoundTrackBackgroundGameOver);
    m_soundBufferBackgroundGameOver =
        m_engine.addSoundBuffer(m_soundTrackBackgroundGameOver);
    m_soundBufferBackgroundGameOver->setVolume(backgroundVolume);

    m_soundTrackUserRocket = m_engine.addSoundTrack(pathToSoundTrackUserRocket);
    m_soundBufferUserRocket = m_engine.addSoundBuffer(m_soundTrackUserRocket);

    m_soundTrackEnemyRocket =
        m_engine.addSoundTrack(pathToSoundTrackEnemyRocket);

    m_soundTrackExplosion = m_engine.addSoundTrack(pathToSoundTrackExplosion);

    m_soundTrackHit       = m_engine.addSoundTrack(pathToSoundTrackHit);
    m_soundTrackExplosion = m_engine.addSoundTrack(pathToSoundTrackExplosion);
    m_soundTrackShoot     = m_engine.addSoundTrack(pathToSoundTrackShoot);
}

void AudioWrapper::playBackground() {}

void AudioWrapper::checkRocketAudioConfig(const Model::World& world)
{
    if (userRocketAudio.rocketPtr != world.userShipPtr)
    {
        userRocketAudio.rocketPtr   = world.userShipPtr;
        userRocketAudio.isEnable    = false;
        userRocketAudio.audioBuffer = m_soundBufferUserRocket;
    }

    const auto sizeWorldRocket = world.rockets.size() - 1;
    const auto sizeAudioRocket = rocketAudios.size();

    auto       rocketAudioIter = rocketAudios.begin();
    auto       worldRocketIter = ++world.rockets.begin();
    const auto minSize         = std::min(sizeWorldRocket, sizeAudioRocket);
    for (size_t i = 0; i < minSize; ++i)
    {
        auto tempIter = rocketAudioIter;
        ++rocketAudioIter;
        const auto worldCurrentRocket = &(*worldRocketIter);
        const auto audioCurrentRocket = tempIter->rocketPtr;
        if (audioCurrentRocket != worldCurrentRocket)
        {
            tempIter->audioBuffer->stop();
            rocketAudios.erase(tempIter);
        }
        ++worldRocketIter;
    }

    const auto sizeAudioRocketAfterErasing = rocketAudios.size();

    if (sizeWorldRocket > sizeAudioRocketAfterErasing)
    {
        for (; worldRocketIter != world.rockets.end(); ++worldRocketIter)
        {
            auto newSoundBuffer =
                m_engine.addSoundBuffer(m_soundTrackEnemyRocket);
            rocketAudios.emplace_back(&(*worldRocketIter), newSoundBuffer);
        }
    }
    else if (sizeWorldRocket < sizeAudioRocketAfterErasing)
    {
        std::for_each(rocketAudioIter, rocketAudios.end(),
                      [](RocketAudio& audio) { audio.audioBuffer->stop(); });
        rocketAudios.erase(rocketAudioIter, rocketAudios.end());
    }
}

void playOneShip(RocketAudio&  rocketAudio,
                 om::myGlfloat volume = AudioWrapper::userRocketsVolume)
{

    const auto& currentRocket = *rocketAudio.rocketPtr;

    const auto isMainEngineEnabled =
        currentRocket.mainEngine.getEngineState().getState();

    std::array<bool, 4> isSideEnginesEnabled;

    std::transform(currentRocket.sideEngines.begin(),
                   currentRocket.sideEngines.end(),
                   isSideEnginesEnabled.begin(),
                   [](const Model::RocketEngine& sideEngine) {
                       return sideEngine.getEngineState().getState();
                   });

    const auto mainEngineSound =
        (isMainEngineEnabled)
            ? currentRocket.mainEngine.enginePercentThrust / 100.f
            : 0.f;

    const auto isAnyOfSideEnginesEnabled = std::any_of(
        isSideEnginesEnabled.begin(), isSideEnginesEnabled.end(),
        [](const bool isSideEngineEnabled) { return isSideEngineEnabled; });

    if (!isMainEngineEnabled && !isAnyOfSideEnginesEnabled)
    {
        rocketAudio.isEnable = false;
        rocketAudio.audioBuffer->stop();
    }

    std::array<float, 4> sideEnginesSound;

    std::transform(
        isSideEnginesEnabled.begin(), isSideEnginesEnabled.end(),
        sideEnginesSound.begin(), [&currentRocket](const bool isEngineEnabled) {
            return (isEngineEnabled)
                       ? currentRocket.sideEngines[0].enginePercentThrust /
                             100.f * 0.05f
                       : 0.f;
        });

    const auto sumSoundOfSideEngines =
        std::accumulate(sideEnginesSound.begin(), sideEnginesSound.end(), 0.f);

    const auto engineSoundPower =
        static_cast<float>(mainEngineSound * 0.8f + sumSoundOfSideEngines);

    const auto userWorldPos = Global::getUserWorldPosition();

    const auto dx = currentRocket.x - userWorldPos[0];
    const auto dy = currentRocket.y - userWorldPos[1];

    const auto distance2 = static_cast<float>(dx * dx + dy * dy);

    const auto distance = std::sqrt(distance2);

    const auto maxHearableDistance = 2000.f;

    auto engineDistanceKoef = std::pow(
        1.f - std::min(distance, maxHearableDistance) / maxHearableDistance, 2);

    const auto enginesSoundVolume = engineSoundPower * engineDistanceKoef;

    auto sinA{ 1.f };

    if (distance > std::numeric_limits<float>::epsilon() * 2)
    {
        sinA = std::abs(dx / distance);
    }

    const auto sign = (dx > 0.f) ? -1.0 : 1.0;

    const auto maxLeftRightRegulation{ 0.2f };

    auto leftRightValue = sign * sinA * maxLeftRightRegulation;

    auto stereoValue = sign * sinA;

    if (std::abs(dx) < 250.f && std::abs(dy) < 250.f)
    {
        const auto regulation =
            std::abs(dx) * std::abs(dy) * 1.f / 250.f * 1.f / 250.f;
        leftRightValue *= regulation;
        stereoValue *= regulation;
    }

    rocketAudio.audioBuffer->setVolume(enginesSoundVolume * volume);

    rocketAudio.audioBuffer->setLeftRightBalance(leftRightValue);

    rocketAudio.audioBuffer->setStereo(stereoValue);

    if (!rocketAudio.isEnable)
    {
        rocketAudio.isEnable = true;
        rocketAudio.audioBuffer->play(om::ISoundBuffer::properties::looped);
    }
}

void AudioWrapper::playOneEvent(const Model::OutEvent& event,
                                om::myGlfloat          volume)
{
    const auto userWorldPos = Global::getUserWorldPosition();

    const auto dx = event.x - userWorldPos[0];
    const auto dy = event.y - userWorldPos[1];

    const auto distance2 = static_cast<float>(dx * dx + dy * dy);

    const auto distance = std::sqrt(distance2);

    const auto maxHearableDistance = static_cast<om::myGlfloat>(
        Model::Bullet::bulletRelativeSpeed * Model::Bullet::timeToLive.count());

    auto distanceKoef = std::pow(
        1.f - std::min(distance, maxHearableDistance) / maxHearableDistance, 2);

    const auto soundVolume = distanceKoef * volume;

    auto sinA{ 1.f };

    if (distance > std::numeric_limits<float>::epsilon() * 2)
    {
        sinA = std::abs(dx / distance);
    }

    const auto sign = (dx > 0.f) ? -1.0 : 1.0;

    const auto maxLeftRightRegulation{ 0.2f };

    auto leftRightValue = sign * sinA * maxLeftRightRegulation;

    auto stereoValue = sign * sinA;

    om::ISoundTrack* soundTrack{};
    switch (event.type)
    {
        case Model::OutEvent::Type::explosion:
            soundTrack = m_soundTrackExplosion;
            break;
        case Model::OutEvent::Type::hit:
            soundTrack = m_soundTrackHit;
            break;
        case Model::OutEvent::Type::shoot:
            soundTrack = m_soundTrackShoot;
            break;
        default:
            soundTrack = m_soundTrackExplosion;
    }

    m_engine.playSoundBufferOnce(soundTrack, soundVolume, leftRightValue,
                                 stereoValue);
}

void AudioWrapper::play(const Model::World& world)
{
    if (userRocketAudio.rocketPtr == nullptr)
    {
        m_soundBufferBackgroundGameOver->stop();
        m_soundBufferBackground->play(om::ISoundBuffer::properties::looped);
    }

    checkRocketAudioConfig(world);
    playOneShip(userRocketAudio, AudioWrapper::userRocketsVolume);
    for (auto& audioRocket : rocketAudios)
    {
        playOneShip(audioRocket, AudioWrapper::enemiesRocketsVolume);
    }

    for (const auto& event : world.outEvents)
    {
        playOneEvent(event);
    }
}

void AudioWrapper::playGameOver()
{
    if (userRocketAudio.rocketPtr != nullptr)
    {
        m_soundBufferBackground->stop();
        userRocketAudio.isEnable = false;
        userRocketAudio.audioBuffer->stop();
        userRocketAudio.rocketPtr = nullptr;
        std::for_each(
            rocketAudios.begin(), rocketAudios.end(),
            [](RocketAudio& rocketAudio) { rocketAudio.audioBuffer->stop(); });
        rocketAudios.clear();
        m_soundBufferBackgroundGameOver->play(
            om::ISoundBuffer::properties::looped);
    }
}
