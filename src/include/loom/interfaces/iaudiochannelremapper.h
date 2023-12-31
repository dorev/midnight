#pragma once

#include "loom/interfaces/iaudiosubsystem.h"

namespace Loom
{

class AudioBuffer;

class IAudioChannelRemapper : public IAudioSubsystem
{
public:
    IAudioChannelRemapper(IAudioSystem& system);
    static IAudioChannelRemapper& GetStub();
    IAudioChannelRemapper& GetInterface();
    const char* GetName() const override;
    AudioSubsystemType GetType() const final override;
    virtual Result Remap(const AudioBuffer& source, AudioBuffer& destination);
};

} // namespace Loom
