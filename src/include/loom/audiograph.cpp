#include "loom/audiograph.h"
#include "loom/audionode.h"
#include "loom/iaudiosystem.h"
#include "loom/nodes/mixingnode.h"

namespace Loom
{

AudioGraph::AudioGraph(IAudioSystem& system)
    : _System(system)
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

template <class T, class... Args>
shared_ptr<T> AudioGraph::CreateNode(Args&&... args)
{
    static_assert(std::is_base_of_v<AudioNode, T>, "T must be derived from AudioNode");

    shared_ptr<T> node = make_shared<T>(std::forward<Args>(args)...);
    if (node != nullptr)
    {
        shared_ptr<AudioNode> nodeBase = shared_ptr_cast<AudioNode>(node);
        scoped_lock lock(_UpdateNodesMutex);
        if (_NodesToAdd.insert(nodeBase).second)
        {
            LOOM_LOG("Added node %s to AudioGraph.", node->GetName());
            Result result = nodeBase->Initialize();
            if (result != Result::Ok)
                LOOM_LOG_WARNING("Failed node %s initialization.", node->GetName());
        }
        else
        {
            LOOM_LOG_WARNING("Unable to add node %s to AudioGraph. Shutting down and deallocating node.", node->GetName());
            node->Shutdown();
            node = nullptr;
        }
    }
    return node;
}

template <class T>
Result AudioGraph::RemoveNode(T* node)
{
    static_assert(std::is_base_of_v<AudioNode, T>, "T must be derived from AudioNode");

    scoped_lock lock(_UpdateNodesMutex);
    if (node == nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);
    if (_NodesToRemove.insert(shared_ptr_cast<AudioNode>(node)))
        return Result::Ok;
    else
        LOOM_RETURN_RESULT(Result::CannotFind);
}

template <class T, class U>
AudioGraph::NodeConnection::NodeConnection(shared_ptr<T>& sourceNode, shared_ptr<U>& destinationNode)
{
    static_assert(std::is_base_of_v<AudioNode, T>, "Source node must be derived from AudioNode");
    static_assert(std::is_base_of_v<AudioNode, U>, "Destination node must be derived from AudioNode");
    this->sourceNode = shared_ptr_cast<AudioNode>(sourceNode);
    this->destinationNode = shared_ptr_cast<AudioNode>(destinationNode);
}

template <class T, class U>
Result AudioGraph::Connect(shared_ptr<T>& sourceNode, shared_ptr<U>& destinationNode)
{
    static_assert(std::is_base_of_v<AudioNode, T>, "Source node must be derived from AudioNode");
    static_assert(std::is_base_of_v<AudioNode, U>, "Destination node must be derived from AudioNode");

    _NodesToConnect.emplace_back(sourceNode, destinationNode);
    return Result::Ok;
}

template <class T, class... NodeTypes>
Result AudioGraph::Chain(shared_ptr<T>& node, shared_ptr<NodeTypes>&... followingNodes)
{
    static_assert(std::is_base_of_v<AudioNode, T>, "T must be derived from AudioNode");
    static_assert((std::is_base_of_v<AudioNode, NodeTypes> && ...), "All types must be derived from AudioNode");

    if (node == nullptr || ((followingNodes == nullptr) || ...))
        LOOM_RETURN_RESULT(Result::Nullptr);
    scoped_lock lock(_UpdateNodesMutex);
    return ChainNodesImpl(node, followingNodes...);
}

template <class T, class U, class... NodeTypes>
Result AudioGraph::ChainNodesImpl(shared_ptr<T>& leftNode, shared_ptr<U>& rightNode, shared_ptr<NodeTypes>&... followingNodes)
{
    if constexpr (sizeof...(followingNodes) == 0)
    {
        return Connect(leftNode, rightNode);
    }
    else
    {
        Result result = Connect(leftNode, rightNode);
        LOOM_CHECK_RESULT(result);
        return ChainNodesImpl(rightNode, followingNodes...);
    }
}

template <class T>
Result AudioGraph::ChainNodesImpl(shared_ptr<T>& first)
{
    LOOM_UNUSED(first);
    return Result::Ok;
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
            _OutputNode = shared_ptr_cast<AudioNode>(make_shared<MixingNode>(_System));
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
