#pragma once

#include "loom/interfaces/iaudiosubsystem.h"
#include "loom/audionode.h"

namespace Loom
{

enum class AudioGraphState
{
    Invalid,
    Idle,
    Busy
};

class AudioBuffer;

class IAudioGraph : public IAudioSubsystem
{
public:
    IAudioGraph(IAudioSystem& system);
    AudioSubsystemType GetType() const final override;

    virtual Result Execute(AudioBuffer& outputBuffer) = 0;
    virtual AudioGraphState GetState() const = 0;
    virtual Result RemoveNode(AudioNodePtr& node) = 0;
    virtual Result ConnectNodes(AudioNodePtr& sourceNode, AudioNodePtr& destinationNode) = 0;
    virtual Result ConnectNodes(initializer_list<AudioNodePtr>&& nodes) = 0;

    template <class NodeType, class... Args>
    AudioNodePtr CreateNode(Args&&... args)
    {
        static_assert(std::is_base_of_v<AudioNode, NodeType>, "NodeType must be derived from AudioNode");

        AudioNodePtr node = shared_ptr_cast<AudioNode>(make_shared<NodeType>(GetSystemInterface(), std::forward<Args>(args)...));
        if (node != nullptr)
        {
            Result result = InsertNode(node);
            if (result == Result::Ok)
                OnNodeInsertSuccess(node);
            else
                OnNodeInsertFailure(node, result);
        }
        else
        {
            OnNodeCreationFailure(node);
        }
        return node;
    }

protected:
    void VisitNode(const AudioNodePtr& node);
    void ClearNodeVisit(const AudioNodePtr& node);
    bool NodeWasVisited(const AudioNodePtr& node);
    set<AudioNodePtr>& GetNodeOutputNodes(const AudioNodePtr& node);
    set<AudioNodePtr>& GetNodeInputNodes(const AudioNodePtr& node);

    virtual Result InsertNode(AudioNodePtr& node) = 0;
    virtual void OnNodeInsertSuccess(AudioNodePtr& node) = 0;
    virtual void OnNodeInsertFailure(AudioNodePtr& node, const Result& result) = 0;
    virtual void OnNodeCreationFailure(AudioNodePtr& node) = 0;
};

class AudioGraphStub : public IAudioGraph
{
public:
    AudioGraphStub();
    static AudioGraphStub& GetInstance();
    const char* GetName() const final override;
    Result Execute(AudioBuffer&) final override;
    AudioGraphState GetState() const final override;
    Result InsertNode(AudioNodePtr& node) final override;
    void OnNodeInsertSuccess(AudioNodePtr& node) final override;
    void OnNodeInsertFailure(AudioNodePtr& node, const Result& result) final override;
    void OnNodeCreationFailure(AudioNodePtr& node) final override;
    Result RemoveNode(AudioNodePtr& node) final override;
    Result ConnectNodes(AudioNodePtr& sourceNode, AudioNodePtr& destinationNode) final override;
    Result ConnectNodes(initializer_list<AudioNodePtr>&& nodes) final override;
};

} // namespace Loom
