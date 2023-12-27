#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

namespace Loom
{

enum class AudioSubsystemType
{
    Graph,
    Codec,
    Resampler,
    ChannelRemapper,
    DeviceManager,
    BufferProvider
};

class IAudioSystem;

class IAudioSubsystem
{
public:
    virtual AudioSubsystemType GetType() const = 0;
    virtual const char* GetName() const = 0;
    IAudioSubsystem(IAudioSystem& system);
    IAudioSubsystem(const IAudioSubsystem&) = delete;
    IAudioSubsystem& operator=(const IAudioSubsystem&) = delete;
    virtual ~IAudioSubsystem();
    virtual Result Initialize();
    virtual void Shutdown();

protected:
    IAudioSystem& GetSystemInterface();

private:
    IAudioSystem& _System;
};

} // namespace Loom
