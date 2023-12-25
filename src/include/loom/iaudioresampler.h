#pragma once

#include "loom/iaudiosubsystem.h"

namespace Loom
{

class AudioBuffer;

class IAudioResampler : public IAudioSubsystem
{
public:
    IAudioResampler(IAudioSystem& system);
    AudioSubsystemType GetType() const final override;
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
