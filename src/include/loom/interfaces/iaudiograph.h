#pragma once

#include "loom/interfaces/iaudiosubsystem.h"
#include "loom/nodes/audionode.h"

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
    static IAudioGraph& GetStub();
    IAudioGraph& GetInterface();
    const char* GetName() const override;
    AudioSubsystemType GetType() const final override;

    virtual Result Execute(AudioBuffer& outputBuffer);
    virtual AudioGraphState GetState() const;
    virtual Result RemoveNode(AudioNodePtr& node);
    virtual Result ConnectNodes(AudioNodePtr& sourceNode, AudioNodePtr& destinationNode);
    virtual Result ConnectNodes(initializer_list<AudioNodePtr>&& nodes);

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

    virtual Result InsertNode(AudioNodePtr& node);
    virtual void OnNodeInsertSuccess(AudioNodePtr& node);
    virtual void OnNodeInsertFailure(AudioNodePtr& node, const Result& result);
    virtual void OnNodeCreationFailure(AudioNodePtr& node);
};

} // namespace Loom
