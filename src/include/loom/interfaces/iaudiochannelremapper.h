#pragma once

#include "loom/interfaces/iaudiosubsystem.h"

namespace Loom
{

class AudioBuffer;

class IAudioChannelRemapper : public IAudioSubsystem
{
public:
    IAudioChannelRemapper(IAudioSystem& system);
    AudioSubsystemType GetType() const final override;
    virtual Result Remap(const AudioBuffer& source, AudioBuffer& destination) = 0;
};

class AudioChannelRemapperStub : public IAudioChannelRemapper
{
public:
    AudioChannelRemapperStub();
    static AudioChannelRemapperStub& GetInstance();
    const char* GetName() const final override;
    Result Remap(const AudioBuffer&, AudioBuffer&) final override;
};


} // namespace Loom
