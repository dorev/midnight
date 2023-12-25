#include "loom/audionode.h"
#include "loom/iaudiosystem.h"

namespace Loom
{

AudioNode::AudioNode(IAudioSystem& system, const char* name)
    : _System(system)
    , _Name(name)
    , _State(AudioNodeState::Idle)
{
}

AudioNode::~AudioNode()
{
}

Result AudioNode::AddInput(shared_ptr<AudioNode> node)
{
    if (node == nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);
    if (_InputNodes.insert(node).second)
        return Result::Ok;
    else
        LOOM_RETURN_RESULT(Result::UnableToConnect);
}

Result AudioNode::AddOutput(shared_ptr<AudioNode> node)
{
    if (node ==  nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);
    if (_OutputNodes.insert(node).second)
        return Result::Ok;
    else
        LOOM_RETURN_RESULT(Result::UnableToConnect);
}

Result AudioNode::Disconnect(shared_ptr<AudioNode> node)
{
    if (node ==  nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);
    _InputNodes.erase(node);
    _OutputNodes.erase(node);
    return Result::Ok;
}

void AudioNode::ReleaseBuffer()
{
    _Buffer.Release();
}

Result AudioNode::PullInputNodes(AudioBuffer& destinationBuffer)
{
    if (_InputNodes.empty())
        LOOM_RETURN_RESULT(Result::NoData);
    for (auto itr = _InputNodes.begin(); itr != _InputNodes.end(); itr++)
    {
        shared_ptr<AudioNode> node = *itr;
        node->Execute(destinationBuffer);
        if (itr == _InputNodes.begin())
        {
            _Buffer = destinationBuffer;
        }
        else
        {
            Result result = _Buffer.AddSamplesFrom(destinationBuffer);
            LOOM_CHECK_RESULT(result);
        }
        destinationBuffer.Release();
    }
    destinationBuffer = _Buffer;
    return Result::Ok;
}

} // namespace Loom
