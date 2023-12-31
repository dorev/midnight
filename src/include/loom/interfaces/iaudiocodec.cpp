#pragma once

#include "loom/interfaces/iaudiocodec.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioCodec::IAudioCodec(IAudioSystem& system)
    : IAudioSystemComponent(system)
{
}

AudioSystemComponentType IAudioCodec::GetType() const
{
    return AudioSystemComponentType::Codec;
}

AudioCodecStub::AudioCodecStub()
    : IAudioCodec(IAudioSystem::GetStub())
{
}

AudioCodecStub& AudioCodecStub::GetInstance()
{
    static AudioCodecStub instance;
    return instance;
}

const char* AudioCodecStub::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioCodec stub";
}

Result AudioCodecStub::LoadAsset(const char*, AudioAsset&) 
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
