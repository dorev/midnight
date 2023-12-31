#pragma once

#include "loom/interfaces/iaudiosubsystem.h"

namespace Loom
{

class AudioBuffer;

class IAudioResampler : public IAudioSubsystem
{
public:
    IAudioResampler(IAudioSystem& system);
    static IAudioResampler& GetStub();
    IAudioResampler& GetInterface();
    const char* GetName() const override;
    AudioSubsystemType GetType() const final override;
    virtual Result Resample(const AudioBuffer& source, AudioBuffer& destination);
};

} // namespace Loom
