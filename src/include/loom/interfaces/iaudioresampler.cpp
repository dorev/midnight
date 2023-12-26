#pragma once

#include "loom/interfaces/iaudioresampler.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioResampler::IAudioResampler(IAudioSystem& system)
    : IAudioSubsystem(system)
{
}

AudioSubsystemType IAudioResampler::GetType() const
{
    return AudioSubsystemType::Resampler;
}

AudioResamplerStub::AudioResamplerStub()
    : IAudioResampler(IAudioSystem::GetStub())
{
}

AudioResamplerStub& AudioResamplerStub::GetInstance()
{
    static AudioResamplerStub instance;
    return instance;
}

const char* AudioResamplerStub::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioResampler stub";
}

Result AudioResamplerStub::Resample(const AudioBuffer&, AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
