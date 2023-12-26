#pragma once

#include "loom/iaudiograph.h"
#include "loom/audionode.h"

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
    Result InsertNode(AudioNodePtr& node) override;
    void OnNodeInsertSuccess(AudioNodePtr& node) override;
    void OnNodeInsertFailure(AudioNodePtr& node, const Result& result) override;
    void OnNodeCreationFailure(AudioNodePtr& node) override;
    Result RemoveNode(AudioNodePtr& node) override;
    Result ConnectNodes(AudioNodePtr& sourceNode, AudioNodePtr& destinationNode) override;
    Result ConnectNodes(initializer_list<AudioNodePtr>&& nodes) override;

private:
    struct NodeConnection
    {
        AudioNodePtr sourceNode;
        AudioNodePtr destinationNode;

        NodeConnection(const AudioNodePtr& sourceNode, const AudioNodePtr& destinationNode)
            : sourceNode(sourceNode)
            , destinationNode(destinationNode)
        {
        }
    };

    Result UpdateNodes();
    void SearchOutputNodes(AudioNodePtr node, set<AudioNodePtr>& outputNodesSearchResult);
    void ClearNodesVisitedFlag();

private:
    atomic<AudioGraphState> _State;
    AudioNodePtr _OutputNode;
    set<AudioNodePtr> _Nodes;
    set<AudioNodePtr> _NodesToAdd;
    set<AudioNodePtr> _NodesToRemove;
    vector<NodeConnection> _NodesToConnect;
    mutex _UpdateNodesMutex;
};


} // namespace Loom
