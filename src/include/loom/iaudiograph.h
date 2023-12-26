#pragma once

#include "loom/iaudiosubsystem.h"

namespace Loom
{

enum class AudioGraphState
{
    Invalid,
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
    virtual Result RemoveNode(shared_ptr<AudioNode>& node) = 0;
    virtual Result ConnectNodes(shared_ptr<AudioNode>& sourceNode, shared_ptr<AudioNode>& destinationNode) = 0;
    virtual Result ConnectNodes(initializer_list<shared_ptr<AudioNode>>&& nodes) = 0;

    template <class NodeType, class... Args>
    shared_ptr<AudioNode> CreateNode(Args&&... args)
    {
        static_assert(std::is_base_of_v<AudioNode, NodeType>, "NodeType must be derived from AudioNode");

        shared_ptr<AudioNode> node = shared_ptr_cast<AudioNode>(make_shared<NodeType>(GetSystemInterface(), std::forward<Args>(args)...));
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
    void VisitNode(const shared_ptr<AudioNode>& node);
    void ClearNodeVisit(const shared_ptr<AudioNode>& node);
    bool NodeWasVisited(const shared_ptr<AudioNode>& node);
    set<shared_ptr<AudioNode>>& GetNodeOutputNodes(const shared_ptr<AudioNode>& node);
    set<shared_ptr<AudioNode>>& GetNodeInputNodes(const shared_ptr<AudioNode>& node);

    virtual Result InsertNode(shared_ptr<AudioNode>& node) = 0;
    virtual void OnNodeInsertSuccess(shared_ptr<AudioNode>& node) = 0;
    virtual void OnNodeInsertFailure(shared_ptr<AudioNode>& node, const Result& result) = 0;
    virtual void OnNodeCreationFailure(shared_ptr<AudioNode>& node) = 0;
};

class AudioGraphStub : public IAudioGraph
{
public:
    AudioGraphStub();
    static AudioGraphStub& GetInstance();
    const char* GetName() const final override;
    Result Execute(AudioBuffer&) final override;
    AudioGraphState GetState() const final override;
    Result InsertNode(shared_ptr<AudioNode>& node) final override;
    void OnNodeInsertSuccess(shared_ptr<AudioNode>& node) final override;
    void OnNodeInsertFailure(shared_ptr<AudioNode>& node, const Result& result) final override;
    void OnNodeCreationFailure(shared_ptr<AudioNode>& node) final override;
    Result RemoveNode(shared_ptr<AudioNode>& node) final override;
    Result ConnectNodes(shared_ptr<AudioNode>& sourceNode, shared_ptr<AudioNode>& destinationNode) final override;
    Result ConnectNodes(initializer_list<shared_ptr<AudioNode>>&& nodes) final override;
};

} // namespace Loom
