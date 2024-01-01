#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

namespace Loom
{

enum class AudioSystemComponentType
{
    System,
    Graph,
    Codec,
    Resampler,
    ChannelRemapper,
    DeviceManager,
    BufferProvider
};

class IAudioSystem;

class IAudioSystemComponent
{
public:
    IAudioSystemComponent(IAudioSystem& system);
    IAudioSystemComponent(const IAudioSystemComponent&) = delete;
    IAudioSystemComponent& operator=(const IAudioSystemComponent&) = delete;
    virtual AudioSystemComponentType GetType() const = 0;
    virtual const char* GetName() const = 0;
    virtual ~IAudioSystemComponent();
    virtual Result Initialize();
    virtual Result Update();
    virtual void Shutdown();

protected:
    IAudioSystem& GetSystemInterface();

private:
    IAudioSystem& _System;
};

} // namespace Loom
