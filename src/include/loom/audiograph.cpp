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

const char* AudioGraph::GetName() const
{
    return "AudioGraph";
}

AudioGraphState AudioGraph::GetState() const
{
    return _State;
}

Result AudioGraph::InsertNode(shared_ptr<AudioNode>& node)
{
    _UpdateNodesMutex.lock();
    if (_NodesToAdd.insert(node).second)
        return Result::Ok;
    else
        LOOM_RETURN_RESULT(Result::UnableToAddNode);
}

void AudioGraph::OnNodeInsertSuccess(shared_ptr<AudioNode>& node)
{
    LOOM_LOG("Queuing node %s for insertion to AudioGraph.", node->GetName());
    // TODO: consider moving initialization once
    Result result = node->Initialize();
    if (result != Result::Ok)
    {
        LOOM_LOG_RESULT(result);
        LOOM_LOG_WARNING("Failed node %s (%llu) initialization.", node->GetName(), node->GetId());
    }
    _UpdateNodesMutex.unlock();
}

void AudioGraph::OnNodeInsertFailure(shared_ptr<AudioNode>& node, const Result& result)
{
    LOOM_LOG_WARNING("Unable to add node %s to AudioGraph. Shutting down and deallocating node.", node->GetName());
    LOOM_LOG_RESULT(result);
    node->Shutdown();
    node = nullptr;
    _UpdateNodesMutex.unlock();
}

void AudioGraph::OnNodeCreationFailure(shared_ptr<AudioNode>& node)
{
    const char* name = node == nullptr ? "{nullptr}" : node->GetName();
    LOOM_LOG_WARNING("Failed to create node %s.", name);
}

Result AudioGraph::RemoveNode(shared_ptr<AudioNode>& node)
{
    scoped_lock lock(_UpdateNodesMutex);
    if (node == nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);
    if (_NodesToRemove.insert(node).second)
        return Result::Ok;
    else
        LOOM_RETURN_RESULT(Result::CannotFind);
}

Result AudioGraph::ConnectNodes(shared_ptr<AudioNode>& sourceNode, shared_ptr<AudioNode>& destinationNode)
{
    _NodesToConnect.emplace_back(sourceNode, destinationNode);
    return Result::Ok;
}

Result AudioGraph::ConnectNodes(initializer_list<shared_ptr<AudioNode>>&& nodes)
{
    Result result = Result::InvalidParameter;
    if (nodes.size() > 1)
    {
        shared_ptr<AudioNode> previousNode = nullptr;
        for (auto itr = nodes.begin(); itr != nodes.end(); itr++)
        {
            shared_ptr<AudioNode> node = *itr;
            if (itr != nodes.begin())
                LOOM_CHECK_RESULT(ConnectNodes(previousNode, node));
            previousNode = node;
        }
        result = Result::Ok;
    }
    return result;
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
