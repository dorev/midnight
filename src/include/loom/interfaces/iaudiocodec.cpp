#pragma once

#include "loom/interfaces/iaudiocodec.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioCodec::IAudioCodec(IAudioSystem& system)
    : IAudioSubsystem(system)
{
}

IAudioCodec& IAudioCodec::GetStub()
{
    static IAudioCodec instance(IAudioSystem::GetStub());
    return instance;
}

IAudioCodec& IAudioCodec::GetInterface()
{
    return *this;
}

AudioSubsystemType IAudioCodec::GetType() const
{
    return AudioSubsystemType::Codec;
}

const char* IAudioCodec::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioCodec stub";
}

Result IAudioCodec::LoadAsset(const char*, AudioAsset&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
