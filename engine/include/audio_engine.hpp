#pragma once
#include "iengine.hpp"
#include <SDL.h>
#include <forward_list>

namespace om
{
class SoundTrack : public ISoundTrack
{
public:
    explicit SoundTrack(std::string_view     path,
                        const SDL_AudioSpec& deviceAudioSpec);
    ~SoundTrack() override;
    uint_least8_t*   m_buffer{};
    uint_least32_t   m_length{};
    std::string_view m_path{};
    SDL_AudioSpec    m_audioSpec;

private:
    [[nodiscard]] uint_least8_t* loadWav(std::string_view path,
                                         SDL_AudioSpec&   fileAudioSpec,
                                         uint_least32_t&  length);

    [[nodiscard]] uint_least8_t* transformBufferToDeviceFormat(
        uint_least8_t* bufferFileFormat, uint_least32_t length,
        uint_least32_t& newLength, const SDL_AudioSpec& fileAudioSpec,
        const SDL_AudioSpec& deviceAudioSpec);

    bool m_isConverted{};
};

#pragma pack(push, 1)
class SoundBuffer final : public ISoundBuffer
{
public:
    SoundBuffer(ISoundTrack* soundTrack, SDL_AudioDeviceID device_);

    //    sound_buffer_impl(std::string_view path, SDL_AudioDeviceID device,
    //                      SDL_AudioSpec audio_spec);
    ~SoundBuffer() override;

    void play(const properties prop) override;

    void stop() override;

    void proceed() override;
    void pause() override;

    void setVolume(float inVolume) override;

    void setLeftRightBalance(float inLeftRightBalance) override;

    void setStereo(float inStereoBalance) override;

    SoundTrack*          m_soundTrack;
    const uint8_t* const buffer;
    uint32_t             length;
    uint32_t             current_index{ 0 };
    SDL_AudioDeviceID    device;
    bool                 is_playing{ false };
    bool                 is_looped{ false };
    bool                 is_disposable{ false };
    float                volume{ 1.0 };
    float                leftRightBalance{ 0.0 };
    float                stereoBalance{ 0.0 };
    float                lastStereoBalance{ 0.0 };
};
#pragma pack(pop)

class AudioEngine
{
public:
    bool          initialize();
    void          shutdown();
    ISoundTrack*  addSoundTrack(std::string_view path);
    void          eraseSoundTrack(ISoundTrack* soundTrack);
    ISoundBuffer* addSoundBuffer(ISoundTrack* soundTrack);
    void playSoundBufferOnce(ISoundTrack* soundTrack, om::myGlfloat volume = 1,
                             om::myGlfloat leftRightBalance = 0,
                             om::myGlfloat stereo           = 0);
    void eraseSoundBuffer(ISoundBuffer* soundBuffer);

private:
    std::string_view getDefaultDeviceName();
    void             printListOfDevices();
    void             printListOfDrivers();
    void             printAudioSpec();
    bool             handleWindowsSpecific();

    static void playSoundInternal(SoundBuffer* buffer, uint8_t* stream,
                                  int                    stream_size,
                                  const SDL_AudioFormat& format);
    static void audioCallback(void* audioEnginePtr, uint8_t* stream,
                              int stream_size);

    std::string_view                m_audioDeviceName{};
    SDL_AudioDeviceID               m_audioDevice{};
    SDL_AudioSpec                   m_audioDeviceSpec{};
    std::forward_list<SoundTrack*>  m_soundTracks{};
    std::forward_list<SoundBuffer*> m_soundBuffers{};
};

} // namespace om
