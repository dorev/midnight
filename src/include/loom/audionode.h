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

struct AudioNodeId
{
    static constexpr u64 AudioSource = 1;
    static constexpr u64 MixingNode = 2;
};

class IAudioSystem;

class AudioNode
{
public:
    AudioNode(IAudioSystem& system);
    virtual ~AudioNode();

    // Method to override for custom node processing
    virtual Result Execute(AudioBuffer& outputBuffer) = 0;
    virtual const char* GetName() const = 0;
    virtual u64 GetTypeId() const = 0;
    virtual u64 GetId() const;
    virtual Result Initialize();
    virtual Result Shutdown();

    AudioNodeState GetState() const;
    Result AddInput(shared_ptr<AudioNode> node);
    Result AddOutput(shared_ptr<AudioNode> node);
    Result Disconnect(shared_ptr<AudioNode> node);

protected:
    AudioBuffer& GetBuffer();
    void ReleaseBuffer();
    Result ExecuteInputNodes(AudioBuffer& destinationBuffer);

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
