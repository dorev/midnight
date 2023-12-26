#include "loom/audionode.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

AudioNode::AudioNode(IAudioSystem& system)
    : _System(system)
    , _State(AudioNodeState::Idle)
{
}

AudioNode::~AudioNode()
{
}

u64 AudioNode::GetId() const
{
    return reinterpret_cast<u64>(this);
}

Result AudioNode::Initialize()
{
    return Result::Ok;
}

Result AudioNode::Shutdown()
{
    return Result::Ok;
}

AudioNodeState AudioNode::GetState() const
{
    return _State.load(std::memory_order_relaxed);
}

Result AudioNode::AddInput(AudioNodePtr node)
{
    if (node == nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);
    if (_InputNodes.insert(node).second)
        return Result::Ok;
    else
        LOOM_RETURN_RESULT(Result::UnableToConnect);
}

Result AudioNode::AddOutput(AudioNodePtr node)
{
    if (node ==  nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);
    if (_OutputNodes.insert(node).second)
        return Result::Ok;
    else
        LOOM_RETURN_RESULT(Result::UnableToConnect);
}

Result AudioNode::Disconnect(AudioNodePtr node)
{
    if (node ==  nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);
    _InputNodes.erase(node);
    _OutputNodes.erase(node);
    return Result::Ok;
}

AudioBuffer& AudioNode::GetBuffer()
{
    return _Buffer;
}

void AudioNode::ReleaseBuffer()
{
    _Buffer.Release();
}

Result AudioNode::ExecuteInputNodes(AudioBuffer& destinationBuffer)
{
    if (_InputNodes.empty())
        LOOM_RETURN_RESULT(Result::NoData);
    for (auto itr = _InputNodes.begin(); itr != _InputNodes.end(); itr++)
    {
        AudioNodePtr node = *itr;
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
