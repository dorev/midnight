#pragma once

#include "loom/defines.h"
#include "loom/types.h"

#include "loom/userheader.h"

namespace Loom
{

enum class AudioSubsystemType
{
    Graph,
    Decoder,
    Resampler,
    ChannelRemapper,
    DeviceManager,
    BufferProvider
};

class IAudioSubsystem
{
public:
    virtual AudioSubsystemType GetType() const = 0;
    virtual const char* GetName() const = 0;

    IAudioSubsystem()
    {
    }
    IAudioSubsystem(const IAudioSubsystem&) = delete;
    IAudioSubsystem& operator=(const IAudioSubsystem&) = delete;

    virtual ~IAudioSubsystem()
    {
    }

    virtual Result Initialize()
    {
        return Result::Ok;
    }

    virtual void Shutdown()
    {
    }
};

} // namespace Loom
