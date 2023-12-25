#include "loom/audiograph.h"
#include "loom/audionode.h"
#include "loom/iaudiosystem.h"
#include "loom/nodes/mixingnode.h"

namespace Loom
{

AudioGraph::AudioGraph(IAudioSystem& system)
    : IAudioGraph(system)
    , _State(AudioGraphState::Idle)
{
}

Result AudioGraph::Execute(AudioBuffer& destinationBuffer)
{
    AudioGraphState idleState = AudioGraphState::Idle;
    if (!_State.compare_exchange_strong(idleState, AudioGraphState::Busy))
        LOOM_RETURN_RESULT(Result::Busy);
    Result result = Result::Ok;
    result = UpdateNodes();
    LOOM_CHECK_RESULT(result);
    return _OutputNode->Execute(destinationBuffer);
}


Result AudioGraph::UpdateNodes()
{
    scoped_lock lock(_UpdateNodesMutex);
    for (const shared_ptr<AudioNode>& node : _NodesToRemove)
    {
        node->Shutdown();
        _Nodes.erase(node);
    }
    _NodesToRemove.clear();
    for (const shared_ptr<AudioNode>& node : _NodesToAdd)
        _Nodes.insert(node);
    _NodesToAdd.clear();
    if (_Nodes.empty())
    {
        _OutputNode = nullptr; // Just in case
        LOOM_RETURN_RESULT(Result::MissingOutputNode);
    }
    for (NodeConnection& connection : _NodesToConnect)
        connection.sourceNode->AddOutput(connection.destinationNode);
    _NodesToConnect.clear();

    // Evaluate output node
    set<shared_ptr<AudioNode>> outputNodes;
    ClearNodesVisitedFlag();
    for (const shared_ptr<AudioNode>& node : _Nodes)
        SearchOutputNodes(node, outputNodes);
    bool nodesContainsOutputNode = _OutputNode != nullptr && _Nodes.find(_OutputNode) != _Nodes.end();
    if (outputNodes.size() == 1)
    {
        shared_ptr<AudioNode> node = *outputNodes.begin();
        if (node == _OutputNode)
        {
            // The only leaf is the current output node
            return Result::Ok;
        }
        else
        {
            if (nodesContainsOutputNode)
            {
                // This means the output node has an output node behind it...
                // That's not normal!
                LOOM_RETURN_RESULT(Result::UnexpectedState);
            }
            else
            {
                // Update the node found as the official output node
                _OutputNode = node;
            }
        }
    }
    else
    {
        if (nodesContainsOutputNode)
        {
            // All the output nodes that are not the official output node should be connected to it
            outputNodes.erase(_OutputNode);
        }
        else
        {
            // A new output node must be created
            _OutputNode = shared_ptr_cast<AudioNode>(make_shared<MixingNode>(GetSystemInterface()));
            _Nodes.insert(_OutputNode);
        }
        for (shared_ptr<AudioNode> node : outputNodes)
            node->AddOutput(_OutputNode);
    }
    return Result::Ok;
}

void AudioGraph::SearchOutputNodes(shared_ptr<AudioNode> node, set<shared_ptr<AudioNode>>& outputNodesSearchResult)
{
    if (NodeWasVisited(node))
        return;
    VisitNode(node);
    set<shared_ptr<AudioNode>>& visitedNodeOutputNodes = GetNodeOutputNodes(node);
    if (visitedNodeOutputNodes.empty())
    {
        outputNodesSearchResult.insert(node);
    }
    else
    {
        for (shared_ptr<AudioNode> outputNode : visitedNodeOutputNodes)
            SearchOutputNodes(outputNode, outputNodesSearchResult);
    }
}

void AudioGraph::ClearNodesVisitedFlag()
{
    for (shared_ptr<AudioNode> node : _Nodes)
        ClearNodeVisit(node);
}

} // namespace Loom
