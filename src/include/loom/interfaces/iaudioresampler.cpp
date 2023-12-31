#pragma once

#include "loom/interfaces/iaudioresampler.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioResampler::IAudioResampler(IAudioSystem& system)
    : IAudioSubsystem(system)
{
}

IAudioResampler& IAudioResampler::GetStub()
{
    static IAudioResampler instance(IAudioSystem::GetStub());
    return instance;
}

IAudioResampler& IAudioResampler::GetInterface()
{
    return *this;
}

AudioSubsystemType IAudioResampler::GetType() const
{
    return AudioSubsystemType::Resampler;
}

const char* IAudioResampler::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioResampler stub";
}

Result IAudioResampler::Resample(const AudioBuffer&, AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
