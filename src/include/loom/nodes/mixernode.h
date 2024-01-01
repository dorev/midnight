#pragma once

#include "loom/nodes/audionode.h"
#include "loom/nodes/audionodeparameter.h"

namespace Loom
{

class IAudioSystem;
class AudioBuffer;

class MixerNode : public AudioNode
{
public:
    MixerNode(IAudioSystem& system);
    Result Execute(AudioBuffer& destinationBuffer) override;
    const char* GetName() const override;
    u64 GetTypeId() const override;

private:
    AudioNodeParameter _Gain;
};

} // namespace Loom
