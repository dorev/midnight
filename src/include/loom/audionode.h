#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"
#include "loom/audiobuffer.h"

namespace Loom
{

// Possible states of an audio node
enum class AudioNodeState
{
    Waiting,
    Ready,
    Busy,
    Idle,
};

class IAudioSystem;

// Base class of processing nodes of the audio graph
class AudioNode
{
public:
    AudioNode(IAudioSystem& system, const char* name);
    virtual ~AudioNode();

    // Method to override for custom node processing
    virtual Result Execute(AudioBuffer& outputBuffer) = 0;

    // Called when the graph creates the node
    virtual Result Initialize()
    {
        return Result::Ok;
    }

    // Called when the node is removed
    virtual Result Shutdown()
    {
        return Result::Ok;
    }

    AudioNodeState GetState() const
    {
        return _State;
    }

    const char* GetName() const
    {
        return _Name.c_str();
    }

    Result AddInput(shared_ptr<AudioNode> node);
    Result AddOutput(shared_ptr<AudioNode> node);
    Result Disconnect(shared_ptr<AudioNode> node);

protected:
    void SetName(const char* name)
    {
        _Name = name;
    }

    AudioBuffer& GetNodeBuffer()
    {
        return _Buffer;
    }

    void ReleaseBuffer();
    Result PullInputNodes(AudioBuffer& destinationBuffer);

private:
    friend class IAudioGraph;

    atomic<AudioNodeState> _State;
    string _Name;
    AudioBuffer _Buffer;
    set<shared_ptr<AudioNode>> _InputNodes;
    set<shared_ptr<AudioNode>> _OutputNodes;

    IAudioSystem& _System;
    bool _Visited;
};

} // namespace Loom
