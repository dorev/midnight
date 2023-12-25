#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

#include "loom/iaudiosubsystem.h"

namespace Loom
{

enum class AudioGraphState
{
    Idle,
    Busy
};

class AudioBuffer;
class AudioNode;

class IAudioGraph : public IAudioSubsystem
{
public:
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::Graph;
    }

    virtual Result Execute(AudioBuffer& outputBuffer) = 0;
    virtual AudioGraphState GetState() const = 0;

protected:
    void VisitNode(shared_ptr<AudioNode> node);
    void ClearNodeVisit(shared_ptr<AudioNode> node);
    bool NodeWasVisited(shared_ptr<AudioNode> node);
    set<shared_ptr<AudioNode>>& GetNodeOutputNodes(shared_ptr<AudioNode> node);
    set<shared_ptr<AudioNode>>& GetNodeInputNodes(shared_ptr<AudioNode> node);
};

class AudioGraphStub : public IAudioGraph
{
public:
    static AudioGraphStub& GetInstance()
    {
        static AudioGraphStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioGraph stub";
    }

    Result Execute(AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    AudioGraphState GetState() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return AudioGraphState::Busy;
    }
};

} // namespace Loom
