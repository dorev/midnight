#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

#include "loom/audionode.h"
#include "loom/audionodeparameter.h"

namespace Loom
{

class IAudioSystem;
class AudioBuffer;

class MixingNode : public AudioNode
{
public:
    MixingNode(IAudioSystem& system);
    Result Execute(AudioBuffer& destinationBuffer) override;
    const char* GetName() const override;
    u64 GetTypeId() const override;

private:
    AudioNodeParameter _Gain;
};

} // namespace Loom
