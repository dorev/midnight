#include "loom/nodes/mixingnode.h"

namespace Loom
{

MixingNode::MixingNode(IAudioSystem& system)
    : AudioNode(system, "MixingNode")
    , _Gain("Gain", AudioNodeParameterType::Float32, 1.0f, true, 0.0f, 10.0f)
{
}

Result MixingNode::Execute(AudioBuffer& destinationBuffer)
{
    Result result = PullInputNodes(destinationBuffer);
    LOOM_CHECK_RESULT(result);
    float gain = 1.0f;
    _Gain.GetValue<float>(gain);
    destinationBuffer.MultiplySamplesBy<float>(gain);
    return Result::Ok;
}

} // namespace Loom
