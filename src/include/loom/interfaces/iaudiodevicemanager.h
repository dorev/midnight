#pragma once

#include "loom/interfaces/iaudiosubsystem.h"
#include "loom/audiobuffer.h"

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
    bool defaultDevice;
    AudioBuffer bufferTemplate;
    AudioDeviceType deviceType;
};

class AudioBuffer;
using AudioDevicePlaybackCallback = void(*)(AudioBuffer& outputBuffer, void* userData);

class IAudioDeviceManager : public IAudioSubsystem
{
public:
    IAudioDeviceManager(IAudioSystem& system);
    AudioSubsystemType GetType() const final override;
    virtual Result RegisterPlaybackCallback(AudioDevicePlaybackCallback callback, void* userData) = 0;
    virtual Result EnumerateDevices(u32& deviceCount, const AudioDeviceDescription*& devices) = 0;
    virtual Result SelectPlaybackDevice(const AudioDeviceDescription* device) = 0;
    virtual Result SelectDefaultPlaybackDevice(AudioDeviceDescription& defaultDeviceDescription) = 0;
    virtual Result Start() = 0;
    virtual Result Stop() = 0;
};

class AudioDeviceManagerStub : public IAudioDeviceManager
{
public:
    AudioDeviceManagerStub();
    static AudioDeviceManagerStub& GetInstance();
    const char* GetName() const final override;
    Result RegisterPlaybackCallback(AudioDevicePlaybackCallback, void*) final override;
    Result EnumerateDevices(u32&, const AudioDeviceDescription*&) final override;
    Result SelectPlaybackDevice(const AudioDeviceDescription*) final override;
    Result SelectDefaultPlaybackDevice(AudioDeviceDescription&) final override;
    Result Start() final override;
    Result Stop() final override;
};

} // namespace Loom
