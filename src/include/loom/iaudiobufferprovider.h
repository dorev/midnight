#pragma once

#include "loom/iaudiosubsystem.h"

namespace Loom
{

class AudioBuffer;

class IAudioBufferProvider : public IAudioSubsystem
{
public:
    IAudioBufferProvider(IAudioSystem& system);
    AudioSubsystemType GetType() const final override;

    virtual Result AllocateBuffer(AudioBuffer& buffer) = 0;
    virtual Result ReleaseBuffer(AudioBuffer& buffer) = 0;
};

class AudioBufferProviderStub : public IAudioBufferProvider
{
public:
    AudioBufferProviderStub();
    static AudioBufferProviderStub& GetInstance();
    const char* GetName() const final override;
    Result AllocateBuffer(AudioBuffer&) final override;
    Result ReleaseBuffer(AudioBuffer&) final override;
};

} // namespace Loom
