#include "audio_engine.hpp"
#include "engine_sdl.hpp"
#include <iostream>
#include <map>
#include <stdexcept>

namespace om
{
static std::string_view get_sound_format_name(uint16_t format_value)
{
    static const std::map<int, std::string_view> format = {
        { AUDIO_U8, "AUDIO_U8" },         { AUDIO_S8, "AUDIO_S8" },
        { AUDIO_U16LSB, "AUDIO_U16LSB" }, { AUDIO_S16LSB, "AUDIO_S16LSB" },
        { AUDIO_U16MSB, "AUDIO_U16MSB" }, { AUDIO_S16MSB, "AUDIO_S16MSB" },
        { AUDIO_S32LSB, "AUDIO_S32LSB" }, { AUDIO_S32MSB, "AUDIO_S32MSB" },
        { AUDIO_F32LSB, "AUDIO_F32LSB" }, { AUDIO_F32MSB, "AUDIO_F32MSB" },
    };

    auto it = format.find(format_value);
    return it->second;
}

static std::size_t get_sound_format_size(uint16_t format_value)
{
    static const std::map<int, std::size_t> format = {
        { AUDIO_U8, 1 },     { AUDIO_S8, 1 },     { AUDIO_U16LSB, 2 },
        { AUDIO_S16LSB, 2 }, { AUDIO_U16MSB, 2 }, { AUDIO_S16MSB, 2 },
        { AUDIO_S32LSB, 4 }, { AUDIO_S32MSB, 4 }, { AUDIO_F32LSB, 4 },
        { AUDIO_F32MSB, 4 },
    };

    auto it = format.find(format_value);
    return it->second;
}

uint_least8_t* SoundTrack::loadWav(std::string_view path,
                                   SDL_AudioSpec&   fileAudioSpec,
                                   uint_least32_t&  length)
{
    SDL_RWops* fileSrcWav = SDL_RWFromFile(path.data(), "rb");
    if (fileSrcWav == nullptr)
    {
        throw std::runtime_error(std::string("can't open audio file: ") +
                                 path.data() + ". " + SDL_GetError());
    }

    const int needFreeingSrcWav{ 1 };

    uint_least8_t* wavBuffer;

    if (!SDL_LoadWAV_RW(fileSrcWav, needFreeingSrcWav, &fileAudioSpec,
                        &wavBuffer, &length))
    {
        throw std::runtime_error(std::string("can't load wav: ") + path.data() +
                                 ". " + SDL_GetError());
    }

    std::clog << "--------------------------------------------\n";
    std::clog << "audio format for: " << path << '\n'
              << "format: " << get_sound_format_name(fileAudioSpec.format)
              << '\n'
              << "sample_size: " << get_sound_format_size(fileAudioSpec.format)
              << '\n'
              << "channels: " << static_cast<uint32_t>(fileAudioSpec.channels)
              << '\n'
              << "frequency: " << fileAudioSpec.freq << '\n'
              << "length: " << length << '\n'
              << "time: "
              << static_cast<double>(length) /
                     (fileAudioSpec.channels *
                      static_cast<uint32_t>(fileAudioSpec.freq) *
                      get_sound_format_size(fileAudioSpec.format))
              << "sec" << std::endl;
    std::clog << "--------------------------------------------\n";

    return wavBuffer;
}

uint_least8_t* SoundTrack::transformBufferToDeviceFormat(
    uint_least8_t* bufferFileFormat, uint_least32_t lengthFromFile,
    uint_least32_t& newLength, const SDL_AudioSpec& fileAudioSpec,
    const SDL_AudioSpec& deviceAudioSpec)
{
    if (fileAudioSpec.channels != deviceAudioSpec.channels ||
        fileAudioSpec.format != deviceAudioSpec.format ||
        fileAudioSpec.freq != deviceAudioSpec.freq)
    {
        SDL_AudioCVT cvt;
        SDL_BuildAudioCVT(&cvt, fileAudioSpec.format, fileAudioSpec.channels,
                          fileAudioSpec.freq, deviceAudioSpec.format,
                          deviceAudioSpec.channels, deviceAudioSpec.freq);
        if (cvt.needed) // obviously, this one is always needed.
        {
            // read your data into cvt.buf here.
            cvt.len = static_cast<int>(lengthFromFile);
            // we have to make buffer for inplace conversion
            size_t new_buffer_size =
                static_cast<size_t>(cvt.len * cvt.len_mult);
            uint8_t* temporaryBuffer = new uint8_t[new_buffer_size];
            // copy old buffer to new memory
            std::copy_n(bufferFileFormat, lengthFromFile, temporaryBuffer);
            // cvt.buf has cvt.len_cvt bytes of converted data now.
            SDL_FreeWAV(bufferFileFormat);
            cvt.buf = temporaryBuffer;
            if (SDL_ConvertAudio(&cvt) != 0)
            {
                throw std::runtime_error(
                    std::string(
                        "failed to convert audio data to device format") +
                    ". " + SDL_GetError());
            }

            newLength   = static_cast<uint32_t>(cvt.len_cvt);
            m_audioSpec = deviceAudioSpec;
            return temporaryBuffer;
        }
        else
        {
            // TODO no need to convert buffer, use as is
        }
    }
    newLength   = lengthFromFile;
    m_audioSpec = fileAudioSpec;
    return bufferFileFormat;
}

SoundTrack::SoundTrack(std::string_view     path,
                       const SDL_AudioSpec& deviceAudioSpec)
    : m_path{ path }
{
    SDL_AudioSpec  fileAudioSpec;
    uint_least32_t lengthOfBufferFromFile{};
    auto bufferFromFile = loadWav(path, fileAudioSpec, lengthOfBufferFromFile);

    uint_least32_t lengthOfBufferAfterTransformation{};
    auto           bufferAfterTransformation = transformBufferToDeviceFormat(
        bufferFromFile, lengthOfBufferFromFile,
        lengthOfBufferAfterTransformation, fileAudioSpec, deviceAudioSpec);

    m_buffer = bufferAfterTransformation;
    m_length = lengthOfBufferAfterTransformation;
    m_isConverted =
        (bufferAfterTransformation == bufferFromFile) ? false : true;
}

SoundTrack::~SoundTrack()
{
    if (!m_isConverted)
    {
        SDL_FreeWAV(m_buffer);
    }
    else
    {
        delete m_buffer;
    }

    m_length = 0;
}

SoundBuffer::SoundBuffer(ISoundTrack* soundTrack, SDL_AudioDeviceID device_)
    : m_soundTrack{ static_cast<SoundTrack*>(soundTrack) }
    , buffer{ m_soundTrack->m_buffer }
    , length{ m_soundTrack->m_length }
    , device(device_)
{
}

//    sound_buffer_impl(std::string_view path, SDL_AudioDeviceID device,
//                      SDL_AudioSpec audio_spec);

void SoundBuffer::play(const properties prop)
{
    // Lock callback function
    SDL_LockAudioDevice(device);

    // here we can change properties
    // of sound and dont collade with multithreaded playing
    current_index = 0;
    is_playing    = true;
    is_looped     = (prop == properties::looped);
    is_disposable = (prop == properties::disposable);

    // unlock callback for continue mixing of audio
    SDL_UnlockAudioDevice(device);
}

void SoundBuffer::stop()
{
    SDL_LockAudioDevice(device);
    current_index = 0;
    is_playing    = false;
    is_looped     = false;
    SDL_UnlockAudioDevice(device);
}

void SoundBuffer::proceed()
{
    SDL_LockAudioDevice(device);
    is_playing = true;
    SDL_UnlockAudioDevice(device);
}

void SoundBuffer::pause()
{
    SDL_LockAudioDevice(device);
    is_playing = false;
    SDL_UnlockAudioDevice(device);
}

void SoundBuffer::setVolume(float inVolume)
{
    SDL_LockAudioDevice(device);
    volume = inVolume;
    SDL_UnlockAudioDevice(device);
}

void SoundBuffer::setLeftRightBalance(float inLeftRightBalance)
{
    SDL_LockAudioDevice(device);
    leftRightBalance = inLeftRightBalance;
    SDL_UnlockAudioDevice(device);
}

void SoundBuffer::setStereo(float inStereoBalance)
{
    SDL_LockAudioDevice(device);
    lastStereoBalance = stereoBalance;
    stereoBalance     = inStereoBalance;
    SDL_UnlockAudioDevice(device);
}

SoundBuffer::~SoundBuffer() {}

bool AudioEngine::initialize()
{
    // initialize audio
    m_audioDeviceSpec.freq     = 48000;
    m_audioDeviceSpec.format   = AUDIO_S16LSB;
    m_audioDeviceSpec.channels = 2;
    m_audioDeviceSpec.samples  = 1024; // must be power of 2
    m_audioDeviceSpec.callback = AudioEngine::audioCallback;
    m_audioDeviceSpec.userdata = this;

#ifdef DEBUG_CONFIGURATION
    printListOfDrivers();
    printListOfDevices();
#endif

    if (!handleWindowsSpecific())
    {
        return false;
    }

    auto defaultAudioDeviceName{ getDefaultDeviceName() };

    if (defaultAudioDeviceName.empty())
    {
        return false;
    }

    m_audioDevice = SDL_OpenAudioDevice(defaultAudioDeviceName.data(), 0,
                                        &m_audioDeviceSpec, nullptr,
                                        SDL_AUDIO_ALLOW_ANY_CHANGE);

    if (m_audioDevice == 0)
    {
        std::cerr << "failed open audio device: " << SDL_GetError();
        return false;
    }
    m_audioDeviceName = defaultAudioDeviceName;

#ifdef DEBUG_CONFIGURATION
    printAudioSpec();
#endif

    // unpause device
    SDL_PauseAudioDevice(m_audioDevice, SDL_FALSE);
    return true;
}

std::string_view AudioEngine::getDefaultDeviceName()
{
    std::string_view default_audio_device_name{};
    const int        num_audio_devices = SDL_GetNumAudioDevices(SDL_FALSE);
    if (num_audio_devices > 0)
    {
        default_audio_device_name = SDL_GetAudioDeviceName(0, SDL_FALSE);
    }
    else
    {
        std::cerr << "Number of audio devices is 0" << std::endl;
    }
    return default_audio_device_name;
}

void AudioEngine::printListOfDevices()
{
    // SDL_FALSE - mean get only OUTPUT audio devices
    const int num_audio_devices = SDL_GetNumAudioDevices(SDL_FALSE);
    if (num_audio_devices > 0)
    {
        for (int i = 0; i < num_audio_devices; ++i)
        {
            std::cout << "audio device #" << i << ": "
                      << SDL_GetAudioDeviceName(i, SDL_FALSE) << '\n';
        }
    }
    std::cout << std::flush;
}

void AudioEngine::printListOfDrivers()
{
    const int num_audio_drivers = SDL_GetNumAudioDrivers();
    for (int i = 0; i < num_audio_drivers; ++i)
    {
        std::cout << "audio_driver #:" << i << " " << SDL_GetAudioDriver(i)
                  << '\n';
    }
    std::cout << std::flush;
}

void AudioEngine::printAudioSpec()
{
    std::cout << "--------------------------------------------\n";
    std::cout << "audio device selected: " << m_audioDeviceName << '\n'
              << "freq: " << m_audioDeviceSpec.freq << '\n'
              << "format: " << get_sound_format_name(m_audioDeviceSpec.format)
              << '\n'
              << "channels: "
              << static_cast<uint32_t>(m_audioDeviceSpec.channels) << '\n'
              << "samples: " << m_audioDeviceSpec.samples << '\n'
              << std::flush;
}

bool AudioEngine::handleWindowsSpecific()
{
    // TODO on windows 10 only directsound - works for me
    if (std::string_view("Windows") == SDL_GetPlatform())
    {
        const char* selected_audio_driver = SDL_GetAudioDriver(1);
#ifdef DEBUG_CONFIGURATION
        std::cout << "selected_audio_driver: " << selected_audio_driver
                  << std::endl;
#endif

        if (0 != SDL_AudioInit(selected_audio_driver))
        {
            std::cerr << "can't init SDL audio for windows\n" << std::flush;
            return false;
        }
    }
    return true;
}

void AudioEngine::shutdown()
{
    SDL_PauseAudioDevice(m_audioDevice, SDL_TRUE);
    std::for_each(m_soundBuffers.begin(), m_soundBuffers.end(),
                  [](SoundBuffer* soundBuffer) { delete soundBuffer; });

    std::for_each(m_soundTracks.begin(), m_soundTracks.end(),
                  [](SoundTrack* soundTrack) { delete soundTrack; });
}

ISoundTrack* AudioEngine::addSoundTrack(std::string_view path)
{
    auto track = new SoundTrack{ path, m_audioDeviceSpec };
    m_soundTracks.push_front(track);
    return track;
}

void AudioEngine::eraseSoundTrack(ISoundTrack* soundTrack)
{
    auto reducedToChildTrack = static_cast<SoundTrack*>(soundTrack);
    m_soundTracks.remove(reducedToChildTrack);
    SDL_LockAudioDevice(m_audioDevice);
    delete reducedToChildTrack;
    SDL_UnlockAudioDevice(m_audioDevice);
}

ISoundBuffer* AudioEngine::addSoundBuffer(ISoundTrack* soundTrack)
{
    auto s = new SoundBuffer(soundTrack, m_audioDevice);
    SDL_LockAudioDevice(m_audioDevice);
    m_soundBuffers.push_front(s);
    SDL_UnlockAudioDevice(m_audioDevice);
    return s;
}

void AudioEngine::playSoundBufferOnce(ISoundTrack*  soundTrack,
                                      om::myGlfloat volume,
                                      om::myGlfloat leftRightBalance,
                                      om::myGlfloat stereo)
{
    auto buffer = new SoundBuffer(soundTrack, m_audioDevice);
    buffer->setVolume(volume);
    buffer->setLeftRightBalance(leftRightBalance);
    buffer->setStereo(stereo);
    SDL_LockAudioDevice(m_audioDevice);
    m_soundBuffers.push_front(buffer);
    SDL_UnlockAudioDevice(m_audioDevice);
    buffer->play(ISoundBuffer::properties::disposable);
}

void AudioEngine::eraseSoundBuffer(ISoundBuffer* soundBuffer)
{
    auto reducedToChildSound = static_cast<SoundBuffer*>(soundBuffer);

    // TODO FIXME first remove from sounds collection
    SDL_LockAudioDevice(m_audioDevice);
    m_soundBuffers.remove(reducedToChildSound);
    delete soundBuffer;
    SDL_UnlockAudioDevice(m_audioDevice);
}

void AudioEngine::playSoundInternal(SoundBuffer* buffer, uint8_t* stream,
                                    int                    stream_size,
                                    const SDL_AudioFormat& format)
{
    if (buffer->volume < std::numeric_limits<float>::epsilon())
    {
        return;
    }

    uint32_t rest = buffer->length - buffer->current_index;

    const auto current_stream_size =
        (rest <= static_cast<uint32_t>(stream_size))
            ? rest
            : static_cast<uint32_t>(stream_size);

    auto current_buff_begin = &buffer->buffer[buffer->current_index];

    const uint_least8_t*       convertedBuff{};
    std::vector<uint_least8_t> temp_buff{};

    bool isLeftRightBalanceRequired = std::abs(buffer->leftRightBalance) >
                                      std::numeric_limits<float>::epsilon() * 2;
    bool isStereoBalanceRequired = std::abs(buffer->stereoBalance) >
                                   std::numeric_limits<float>::epsilon() * 2;
    if (isLeftRightBalanceRequired || isStereoBalanceRequired)
    {
        temp_buff.reserve(current_stream_size);

        std::copy_n(current_buff_begin, current_stream_size,
                    std::back_inserter(temp_buff));

        if (isStereoBalanceRequired)
        {
            size_t beginCountDelayedChannel =
                (buffer->stereoBalance > 0) ? 2 : 0;

            const auto numberOfFrames48000BetweenEars =
                static_cast<int>(0.25f / 343 * 48000);

            //            static const float stereoBalanceChangingBound{ 1.0f };

            //            auto changeStereoBalance =
            //                buffer->stereoBalance - buffer->lastStereoBalance;

            //            const auto isChangeInBound =
            //                (std::abs(changeStereoBalance) <
            //                stereoBalanceChangingBound);

            //            if (!isChangeInBound)
            //            {
            //                std::cerr << "111";
            //            }

            //            const auto maxDif = (changeStereoBalance > 0)
            //                                    ? stereoBalanceChangingBound
            //                                    : -stereoBalanceChangingBound;

            //            const auto changeStereoBalanceWithBound =
            //                (isChangeInBound) ? changeStereoBalance : maxDif;

            //            const auto stereoBalance{ buffer->lastStereoBalance +
            //                                      changeStereoBalanceWithBound
            //                                      };

            //            buffer->lastStereoBalance = stereoBalance;

            size_t lastDelayInBytes =
                std::round(numberOfFrames48000BetweenEars *
                           std::abs(buffer->lastStereoBalance));

            size_t delayInBytes = std::round(numberOfFrames48000BetweenEars *
                                             std::abs(buffer->stereoBalance)) +
                                  lastDelayInBytes;

            delayInBytes *= 4;

            if (buffer->current_index < delayInBytes)
            {
                delayInBytes = buffer->current_index;
            }

            const auto streamSizeWithoutDelay =
                static_cast<int64_t>(current_stream_size) -
                static_cast<int64_t>(delayInBytes);

            for (int count = beginCountDelayedChannel - delayInBytes;
                 count < streamSizeWithoutDelay; count += 4)
            {
                temp_buff[count + delayInBytes] = current_buff_begin[count];
                temp_buff[count + delayInBytes + 1] =
                    current_buff_begin[count + 1];
            }
        }
        if (isLeftRightBalanceRequired)
        {
            size_t beginCount  = (buffer->leftRightBalance > 0) ? 2 : 0;
            float  multipicant = (buffer->leftRightBalance > 0)
                                    ? (1.0 - buffer->leftRightBalance)
                                    : (1.0 + buffer->leftRightBalance);

            for (int count = beginCount;
                 count < static_cast<int64_t>(current_stream_size); count += 4)
            {
                auto temp_current_sample{ static_cast<int_least16_t>(
                    temp_buff[count + 1] << 8 | temp_buff[count]) };

                temp_current_sample =
                    std::round(multipicant * temp_current_sample);

                temp_buff[count] = temp_current_sample & 0x00FF;
                temp_current_sample >>= 8;
                temp_buff[count + 1] = (temp_current_sample & 0x00FF);
            }
        }
        convertedBuff = temp_buff.data();
    }
    else
    {
        convertedBuff = current_buff_begin;
    }
    const auto intVolume =
        static_cast<int>(std::round(buffer->volume * SDL_MIX_MAXVOLUME));

    // copy rest to buffer
    SDL_MixAudioFormat(stream, convertedBuff, format, current_stream_size,
                       intVolume);

    buffer->current_index += current_stream_size;
}

void AudioEngine::audioCallback(void* audioEnginePtr, uint8_t* stream,
                                int stream_size)
{
    // no sound default
    std::fill_n(stream, stream_size, '\0');

    AudioEngine* audioEngine = static_cast<AudioEngine*>(audioEnginePtr);

    for (SoundBuffer* snd : audioEngine->m_soundBuffers)
    {
        if (snd->is_playing)
        {
            playSoundInternal(snd, stream, stream_size,
                              audioEngine->m_audioDeviceSpec.format);

            if (snd->current_index == snd->length)
            {
                if (snd->is_looped)
                {
                    // start from begining
                    snd->current_index = 0;
                }
                else
                {
                    snd->is_playing = false;
                }
            }
        }
    }

    audioEngine->m_soundBuffers.remove_if([](SoundBuffer* snd) {
        return snd->is_disposable && !snd->is_playing;
    });
}

} // namespace om
