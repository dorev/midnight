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
    static IAudioDeviceManager& GetStub();
    IAudioDeviceManager& GetInterface();
    const char* GetName() const override;
    AudioSubsystemType GetType() const final override;
    virtual Result RegisterPlaybackCallback(AudioDevicePlaybackCallback callback, void* userData);
    virtual Result EnumerateDevices(u32& deviceCount, const AudioDeviceDescription*& devices);
    virtual Result SelectPlaybackDevice(const AudioDeviceDescription* device);
    virtual Result SelectDefaultPlaybackDevice();
    virtual Result Start();
    virtual Result Stop();
};

} // namespace Loom
