#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

#include "loom/iaudiograph.h"

namespace Loom
{

class IAudioSystem;

class AudioGraph : public IAudioGraph
{
public:
    AudioGraph(IAudioSystem& system);
    Result Execute(AudioBuffer& destinationBuffer) override;
    const char* GetName() const override;
    AudioGraphState GetState() const override;
    Result InsertNode(shared_ptr<AudioNode>& node) override;
    void OnNodeInsertSuccess(shared_ptr<AudioNode>& node) override;
    void OnNodeInsertFailure(shared_ptr<AudioNode>& node, const Result& result) override;
    void OnNodeCreationFailure(shared_ptr<AudioNode>& node) override;
    Result RemoveNode(shared_ptr<AudioNode>& node) override;
    Result ConnectNodes(shared_ptr<AudioNode>& sourceNode, shared_ptr<AudioNode>& destinationNode) override;
    Result ConnectNodes(initializer_list<shared_ptr<AudioNode>>&& nodes) override;

private:
    struct NodeConnection
    {
        shared_ptr<AudioNode> sourceNode;
        shared_ptr<AudioNode> destinationNode;

        NodeConnection(const shared_ptr<AudioNode>& sourceNode, const shared_ptr<AudioNode>& destinationNode)
            : sourceNode(sourceNode)
            , destinationNode(destinationNode)
        {
        }
    };

    Result UpdateNodes();
    void SearchOutputNodes(shared_ptr<AudioNode> node, set<shared_ptr<AudioNode>>& outputNodesSearchResult);
    void ClearNodesVisitedFlag();

private:
    atomic<AudioGraphState> _State;
    shared_ptr<AudioNode> _OutputNode;
    set<shared_ptr<AudioNode>> _Nodes;
    set<shared_ptr<AudioNode>> _NodesToAdd;
    set<shared_ptr<AudioNode>> _NodesToRemove;
    vector<NodeConnection> _NodesToConnect;
    mutex _UpdateNodesMutex;
};


} // namespace Loom
