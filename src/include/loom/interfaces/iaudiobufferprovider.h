#pragma once

#include "loom/interfaces/iaudiosystemcomponent.h"

namespace Loom
{

class AudioBuffer;

class IAudioBufferProvider : public IAudioSystemComponent
{
public:
    IAudioBufferProvider(IAudioSystem& system);
    AudioSystemComponentType GetType() const final override;

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
