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

Result AudioGraph::InsertNode(AudioNodePtr& node)
{
    _UpdateNodesMutex.lock();
    if (_NodesToAdd.insert(node).second)
        return Result::Ok;
    else
        LOOM_RETURN_RESULT(Result::UnableToAddNode);
}

void AudioGraph::OnNodeInsertSuccess(AudioNodePtr& node)
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

void AudioGraph::OnNodeInsertFailure(AudioNodePtr& node, const Result& result)
{
    LOOM_LOG_WARNING("Unable to add node %s to AudioGraph. Shutting down and deallocating node.", node->GetName());
    LOOM_LOG_RESULT(result);
    node->Shutdown();
    node = nullptr;
    _UpdateNodesMutex.unlock();
}

void AudioGraph::OnNodeCreationFailure(AudioNodePtr& node)
{
    const char* name = node == nullptr ? "{nullptr}" : node->GetName();
    LOOM_LOG_WARNING("Failed to create node %s.", name);
}

Result AudioGraph::RemoveNode(AudioNodePtr& node)
{
    scoped_lock lock(_UpdateNodesMutex);
    if (node == nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);
    if (_NodesToRemove.insert(node).second)
        return Result::Ok;
    else
        LOOM_RETURN_RESULT(Result::CannotFind);
}

Result AudioGraph::ConnectNodes(AudioNodePtr& sourceNode, AudioNodePtr& destinationNode)
{
    _NodesToConnect.emplace_back(sourceNode, destinationNode);
    return Result::Ok;
}

Result AudioGraph::ConnectNodes(initializer_list<AudioNodePtr>&& nodes)
{
    Result result = Result::InvalidParameter;
    if (nodes.size() > 1)
    {
        AudioNodePtr previousNode = nullptr;
        for (auto itr = nodes.begin(); itr != nodes.end(); itr++)
        {
            AudioNodePtr node = *itr;
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
    for (const AudioNodePtr& node : _NodesToRemove)
    {
        node->Shutdown();
        _Nodes.erase(node);
    }
    _NodesToRemove.clear();
    for (const AudioNodePtr& node : _NodesToAdd)
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
    set<AudioNodePtr> outputNodes;
    ClearNodesVisitedFlag();
    for (const AudioNodePtr& node : _Nodes)
        SearchOutputNodes(node, outputNodes);
    bool nodesContainsOutputNode = _OutputNode != nullptr && _Nodes.find(_OutputNode) != _Nodes.end();
    if (outputNodes.size() == 1)
    {
        AudioNodePtr node = *outputNodes.begin();
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
        for (AudioNodePtr node : outputNodes)
            node->AddOutput(_OutputNode);
    }
    return Result::Ok;
}

void AudioGraph::SearchOutputNodes(AudioNodePtr node, set<AudioNodePtr>& outputNodesSearchResult)
{
    if (NodeWasVisited(node))
        return;
    VisitNode(node);
    set<AudioNodePtr>& visitedNodeOutputNodes = GetNodeOutputNodes(node);
    if (visitedNodeOutputNodes.empty())
    {
        outputNodesSearchResult.insert(node);
    }
    else
    {
        for (AudioNodePtr outputNode : visitedNodeOutputNodes)
            SearchOutputNodes(outputNode, outputNodesSearchResult);
    }
}

void AudioGraph::ClearNodesVisitedFlag()
{
    for (AudioNodePtr node : _Nodes)
        ClearNodeVisit(node);
}

} // namespace Loom
