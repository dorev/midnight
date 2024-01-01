#include "loom/nodes/audionode.h"
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

IAudioSystem& AudioNode::GetSystem()
{
    return _System;
}

void AudioNode::ReleaseBuffer()
{
    _Buffer.Release();
}

bool AudioNode::BypassNode() const
{
    return _Bypass;
}

Result AudioNode::ExecuteInputNodes(AudioBuffer& destinationBuffer)
{
    if (_InputNodes.empty())
        LOOM_RETURN_RESULT(Result::NoData);
    vector<AudioBuffer> buffersToMix;
    buffersToMix.reserve(_InputNodes.size());
    AudioBuffer tmpBuffer = destinationBuffer;
    for (auto itr = _InputNodes.begin(); itr != _InputNodes.end(); itr++)
    {
        AudioNodePtr node = *itr;
        Result result = node->Execute(tmpBuffer);
        if (Ok(result))
        {
            buffersToMix.emplace_back(tmpBuffer);
            tmpBuffer.Release();
        }
        else if (result != Result::NodeIsVirtual)
        {
            LOOM_LOG_RESULT(result);
        }
    }
    if (buffersToMix.empty())
    {
        return Result::NoData;
    }
    else
    {
        for (auto itr = buffersToMix.begin(); itr != buffersToMix.end(); itr++)
        {
            AudioBuffer& buffer = *itr;
            if (itr == buffersToMix.begin())
            {
                _Buffer = buffer;
            }
            else
            {
                Result result = _Buffer.AddSamplesFrom(buffer);
                if (!Ok(result))
                    LOOM_LOG_RESULT(result);
            }
        }
    }
    destinationBuffer = _Buffer;
    return Result::Ok;
}

} // namespace Loom
