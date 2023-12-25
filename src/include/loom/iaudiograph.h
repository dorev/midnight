#pragma once

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
    IAudioGraph(IAudioSystem& system);
    AudioSubsystemType GetType() const final override;
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
    AudioGraphStub();
    static AudioGraphStub& GetInstance();
    const char* GetName() const final override;
    Result Execute(AudioBuffer&) final override;
    AudioGraphState GetState() const final override;
};

} // namespace Loom
