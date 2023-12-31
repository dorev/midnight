#include "loom/interfaces/iaudiograph.h"
#include "loom/nodes/audionode.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioGraph::IAudioGraph(IAudioSystem& system)
    : IAudioSubsystem(system)
{
}

IAudioGraph& IAudioGraph::GetStub()
{
    static IAudioGraph instance(IAudioSystem::GetStub());
    return instance;
}

IAudioGraph& IAudioGraph::GetInterface()
{
    return *this;
}

AudioSubsystemType IAudioGraph::GetType() const
{
    return AudioSubsystemType::Graph;
}

void IAudioGraph::VisitNode(const AudioNodePtr& node)
{
    node->_Visited = true;
}

void IAudioGraph::ClearNodeVisit(const AudioNodePtr& node)
{
    node->_Visited = false;
}

bool IAudioGraph::NodeWasVisited(const AudioNodePtr& node)
{
    return node->_Visited;
}

set<AudioNodePtr>& IAudioGraph::GetNodeOutputNodes(const AudioNodePtr& node)
{
    return node->_OutputNodes;
}

set<AudioNodePtr>& IAudioGraph::GetNodeInputNodes(const AudioNodePtr& node)
{
    return node->_InputNodes;
}

const char* IAudioGraph::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioGraph stub";
}

Result IAudioGraph::Execute(AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

AudioGraphState IAudioGraph::GetState() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return AudioGraphState::Invalid;
}

Result IAudioGraph::InsertNode(AudioNodePtr&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

void IAudioGraph::OnNodeInsertSuccess(AudioNodePtr&)
{
    LOOM_LOG_RESULT(Result::CallingStub);
}

void IAudioGraph::OnNodeInsertFailure(AudioNodePtr&, const Result&)
{
    LOOM_LOG_RESULT(Result::CallingStub);
}

void IAudioGraph::OnNodeCreationFailure(AudioNodePtr&)
{
    LOOM_LOG_RESULT(Result::CallingStub);
}

Result IAudioGraph::RemoveNode(AudioNodePtr&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result IAudioGraph::ConnectNodes(AudioNodePtr&, AudioNodePtr&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result IAudioGraph::ConnectNodes(initializer_list<AudioNodePtr>&&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}


} // namespace Loom
