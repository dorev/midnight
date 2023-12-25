#pragma once

#include "loom/defines.h"
#include "loom/types.h"

#include "loom/iaudiosubsystem.h"

namespace Loom
{

enum class AudioDeviceType
{
    Playback,
    Recording
};

struct AudioDeviceDescription
{
    string name;
    u32 channels;
    u32 sampleRate;
    bool defaultDevice;
    AudioDeviceType deviceType;
};

class AudioBuffer;
using AudioDevicePlaybackCallback = void(*)(AudioBuffer& outputBuffer, void* userData);

class IAudioDeviceManager : public IAudioSubsystem
{
public:
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::DeviceManager;
    }

    virtual Result RegisterPlaybackCallback(AudioDevicePlaybackCallback callback, void* userData) = 0;
    virtual Result EnumerateDevices(u32& deviceCount, const AudioDeviceDescription*& devices) = 0;
    virtual Result SelectPlaybackDevice(const AudioDeviceDescription* device) = 0;
    virtual Result SelectDefaultPlaybackDevice() = 0;
    virtual Result Start() = 0;
    virtual Result Stop() = 0;
};

class AudioDeviceManagerStub : public IAudioDeviceManager
{
public:
    static AudioDeviceManagerStub& GetInstance()
    {
        static AudioDeviceManagerStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioDeviceManager stub";
    }

    Result RegisterPlaybackCallback(AudioDevicePlaybackCallback, void*) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result EnumerateDevices(u32&, const AudioDeviceDescription*&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result SelectPlaybackDevice(const AudioDeviceDescription*) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result SelectDefaultPlaybackDevice() final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result Start() final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result Stop() final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }
};

} // namespace Loom
