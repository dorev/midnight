#include "loom/nodes/mixernode.h"

namespace Loom
{

MixerNode::MixerNode(IAudioSystem& system)
    : AudioNode(system)
    , _Gain("Gain", AudioNodeParameterType::Float32, 1.0f, true, 0.0f, 10.0f)
{
}

const char* MixerNode::GetName() const
{
    return "MixingNode";
}

u64 MixerNode::GetTypeId() const
{
    return AudioNodeId::MixingNode;
}

Result MixerNode::Execute(AudioBuffer& destinationBuffer)
{
    Result result = ExecuteInputNodes(destinationBuffer);
    LOOM_CHECK_RESULT(result);
    float gain = 1.0f;
    _Gain.GetValue<float>(gain);
    destinationBuffer.MultiplySamplesBy<float>(gain);
    return Result::Ok;
}

} // namespace Loom
