#pragma once

#include "loom/defines.h"
#include "loom/types.h"

#include "loom/iaudiosubsystem.h"

namespace Loom
{

class IAudioResampler : public IAudioSubsystem
{
public:
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::Resampler;
    }
    virtual Result Resample(const AudioBuffer& source, AudioBuffer& destination) = 0;
};

class AudioResamplerStub : public IAudioResampler
{
public:
    static AudioResamplerStub& GetInstance()
    {
        static AudioResamplerStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioResampler stub";
    }

    Result Resample(const AudioBuffer&, AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }
};


} // namespace Loom
