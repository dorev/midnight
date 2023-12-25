#pragma once

#include "loom/defines.h"
#include "loom/types.h"

#include "loom/iaudiosubsystem.h"

namespace Loom
{

class IAudioBufferProvider : public IAudioSubsystem
{
public:
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::BufferProvider;
    }

    virtual Result AllocateBuffer(AudioBuffer& buffer) = 0;
    virtual Result ReleaseBuffer(AudioBuffer& buffer) = 0;
};

class AudioBufferProviderStub : public IAudioBufferProvider
{
public:
    static AudioBufferProviderStub& GetInstance()
    {
        static AudioBufferProviderStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioBufferProvider stub";
    }

    Result AllocateBuffer(AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result ReleaseBuffer(AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }
};

} // namespace Loom
