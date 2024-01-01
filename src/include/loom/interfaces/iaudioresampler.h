#pragma once

#include "loom/interfaces/iaudiosystemcomponent.h"

namespace Loom
{

class AudioBuffer;

class IAudioResampler : public IAudioSystemComponent
{
public:
    IAudioResampler(IAudioSystem& system);
    AudioSystemComponentType GetType() const final override;
    virtual Result Resample(const AudioBuffer& source, AudioBuffer& destination) = 0;
};

class AudioResamplerStub : public IAudioResampler
{
public:
    AudioResamplerStub();
    static AudioResamplerStub& GetInstance();
    const char* GetName() const final override;
    Result Resample(const AudioBuffer&, AudioBuffer&) final override;
};

} // namespace Loom
