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

    const char* GetName() const override
    {
        return "AudioGraph";
    }

    AudioGraphState GetState() const override
    {
        return _State;
    }

    Result Execute(AudioBuffer& destinationBuffer) override;

    template <class T, class... Args>
    shared_ptr<T> CreateNode(Args&&... args);
    template <class T>
    Result RemoveNode(T* node);
    template <class T, class U>
    Result Connect(shared_ptr<T>& sourceNode, shared_ptr<U>& destinationNode);
    template <class T, class... NodeTypes>
    Result Chain(shared_ptr<T>& node, shared_ptr<NodeTypes>&... followingNodes);

private:
    struct NodeConnection
    {
        template <class T, class U>
        NodeConnection(shared_ptr<T>& sourceNode, shared_ptr<U>& destinationNode);

        shared_ptr<AudioNode> sourceNode;
        shared_ptr<AudioNode> destinationNode;
    };

    template <class T, class U, class... NodeTypes>
    Result ChainNodesImpl(shared_ptr<T>& leftNode, shared_ptr<U>& rightNode, shared_ptr<NodeTypes>&... followingNodes);
    template <class T>
    Result ChainNodesImpl(shared_ptr<T>& first);

    Result UpdateNodes();
    void SearchOutputNodes(shared_ptr<AudioNode> node, set<shared_ptr<AudioNode>>& outputNodesSearchResult);
    void ClearNodesVisitedFlag();

private:
    IAudioSystem& _System;
    atomic<AudioGraphState> _State;
    shared_ptr<AudioNode> _OutputNode;
    set<shared_ptr<AudioNode>> _Nodes;
    set<shared_ptr<AudioNode>> _NodesToAdd;
    set<shared_ptr<AudioNode>> _NodesToRemove;
    vector<NodeConnection> _NodesToConnect;
    mutex _UpdateNodesMutex;
};


} // namespace Loom
