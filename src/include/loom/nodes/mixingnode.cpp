#include "loom/nodes/mixingnode.h"

namespace Loom
{

MixingNode::MixingNode(IAudioSystem& system)
    : AudioNode(system)
    , _Gain("Gain", AudioNodeParameterType::Float32, 1.0f, true, 0.0f, 10.0f)
{
}

const char* MixingNode::GetName() const
{
    return "MixingNode";
}

u64 MixingNode::GetTypeId() const
{
    return AudioNodeId::MixingNode;
}

Result MixingNode::Execute(AudioBuffer& destinationBuffer)
{
    Result result = ExecuteInputNodes(destinationBuffer);
    LOOM_CHECK_RESULT(result);
    float gain = 1.0f;
    _Gain.GetValue<float>(gain);
    destinationBuffer.MultiplySamplesBy<float>(gain);
    return Result::Ok;
}

} // namespace Loom
