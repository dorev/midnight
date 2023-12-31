#pragma once

#include "loom/interfaces/iaudiosubsystem.h"

namespace Loom
{

class AudioBuffer;

class IAudioBufferProvider : public IAudioSubsystem
{
public:
    IAudioBufferProvider(IAudioSystem& system);
    static IAudioBufferProvider& GetStub();
    IAudioBufferProvider& GetInterface();
    const char* GetName() const override;
    AudioSubsystemType GetType() const final override;
    virtual Result AllocateBuffer(AudioBuffer& buffer);
    virtual Result ReleaseBuffer(AudioBuffer& buffer);
};

} // namespace Loom
