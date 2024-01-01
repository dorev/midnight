#pragma once

#include "loom/interfaces/iaudiosystemcomponent.h"

namespace Loom
{

class AudioBuffer;

class IAudioChannelRemapper : public IAudioSystemComponent
{
public:
    IAudioChannelRemapper(IAudioSystem& system);
    AudioSystemComponentType GetType() const final override;
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
