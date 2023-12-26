#include "loom/interfaces/iaudiograph.h"
#include "loom/audionode.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioGraph::IAudioGraph(IAudioSystem& system)
    : IAudioSubsystem(system)
{
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

AudioGraphStub::AudioGraphStub()
    : IAudioGraph(IAudioSystem::GetStub())
{
}

AudioGraphStub& AudioGraphStub::GetInstance()
{
    static AudioGraphStub instance;
    return instance;
}

const char* AudioGraphStub::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioGraph stub";
}

Result AudioGraphStub::Execute(AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

AudioGraphState AudioGraphStub::GetState() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return AudioGraphState::Invalid;
}

Result AudioGraphStub::InsertNode(AudioNodePtr&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

void AudioGraphStub::OnNodeInsertSuccess(AudioNodePtr&)
{
    LOOM_LOG_RESULT(Result::CallingStub);
}

void AudioGraphStub::OnNodeInsertFailure(AudioNodePtr&, const Result&)
{
    LOOM_LOG_RESULT(Result::CallingStub);
}

void AudioGraphStub::OnNodeCreationFailure(AudioNodePtr&)
{
    LOOM_LOG_RESULT(Result::CallingStub);
}

Result AudioGraphStub::RemoveNode(AudioNodePtr&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioGraphStub::ConnectNodes(AudioNodePtr&, AudioNodePtr&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioGraphStub::ConnectNodes(initializer_list<AudioNodePtr>&&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}


} // namespace Loom
