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

    const char* GetName() const override
    {
        return "AudioGraph";
    }

    AudioGraphState GetState() const override
    {
        return _State;
    }

    template <class T, class... Args>
    shared_ptr<T> CreateNode(Args&&... args)
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
    Result RemoveNode(T* node)
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
    Result Connect(shared_ptr<T>& sourceNode, shared_ptr<U>& destinationNode)
    {
        static_assert(std::is_base_of_v<AudioNode, T>, "Source node must be derived from AudioNode");
        static_assert(std::is_base_of_v<AudioNode, U>, "Destination node must be derived from AudioNode");

        _NodesToConnect.emplace_back(sourceNode, destinationNode);
        return Result::Ok;
    }

    template <class T, class... NodeTypes>
    Result Chain(shared_ptr<T>& node, shared_ptr<NodeTypes>&... followingNodes)
    {
        static_assert(std::is_base_of_v<AudioNode, T>, "T must be derived from AudioNode");
        static_assert((std::is_base_of_v<AudioNode, NodeTypes> && ...), "All types must be derived from AudioNode");

        if (node == nullptr || ((followingNodes == nullptr) || ...))
            LOOM_RETURN_RESULT(Result::Nullptr);
        scoped_lock lock(_UpdateNodesMutex);
        return ChainNodesImpl(node, followingNodes...);
    }

private:
    struct NodeConnection
    {
        shared_ptr<AudioNode> sourceNode;
        shared_ptr<AudioNode> destinationNode;

        template <class T, class U>
        NodeConnection(shared_ptr<T>& sourceNode, shared_ptr<U>& destinationNode)
        {
            static_assert(std::is_base_of_v<AudioNode, T>, "Source node must be derived from AudioNode");
            static_assert(std::is_base_of_v<AudioNode, U>, "Destination node must be derived from AudioNode");
            this->sourceNode = shared_ptr_cast<AudioNode>(sourceNode);
            this->destinationNode = shared_ptr_cast<AudioNode>(destinationNode);
        }
    };

    Result UpdateNodes();
    void SearchOutputNodes(shared_ptr<AudioNode> node, set<shared_ptr<AudioNode>>& outputNodesSearchResult);
    void ClearNodesVisitedFlag();

    template <class T, class U, class... NodeTypes>
    Result ChainNodesImpl(shared_ptr<T>& leftNode, shared_ptr<U>& rightNode, shared_ptr<NodeTypes>&... followingNodes)
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
    Result ChainNodesImpl(shared_ptr<T>& first)
    {
        LOOM_UNUSED(first);
        return Result::Ok;
    }

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
