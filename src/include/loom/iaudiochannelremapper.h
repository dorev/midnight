#pragma once

#include "loom/defines.h"
#include "loom/types.h"

#include "loom/iaudiosubsystem.h"

namespace Loom
{

class AudioBuffer;

class IAudioChannelRemapper : public IAudioSubsystem
{
public:
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::ChannelRemapper;
    }

    virtual Result Remap(const AudioBuffer& source, AudioBuffer& destination) = 0;
};

class AudioChannelRemapperStub : public IAudioChannelRemapper
{
public:
    static AudioChannelRemapperStub& GetInstance()
    {
        static AudioChannelRemapperStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioChannelRemapper stub";
    }

    Result Remap(const AudioBuffer&, AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }
};


} // namespace Loom
