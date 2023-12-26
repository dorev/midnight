#include "loom/iaudiograph.h"
#include "loom/audionode.h"
#include "loom/iaudiosystem.h"

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

void IAudioGraph::VisitNode(const shared_ptr<AudioNode>& node)
{
    node->_Visited = true;
}

void IAudioGraph::ClearNodeVisit(const shared_ptr<AudioNode>& node)
{
    node->_Visited = false;
}

bool IAudioGraph::NodeWasVisited(const shared_ptr<AudioNode>& node)
{
    return node->_Visited;
}

set<shared_ptr<AudioNode>>& IAudioGraph::GetNodeOutputNodes(const shared_ptr<AudioNode>& node)
{
    return node->_OutputNodes;
}

set<shared_ptr<AudioNode>>& IAudioGraph::GetNodeInputNodes(const shared_ptr<AudioNode>& node)
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

Result AudioGraphStub::InsertNode(shared_ptr<AudioNode>&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

void AudioGraphStub::OnNodeInsertSuccess(shared_ptr<AudioNode>&)
{
    LOOM_LOG_RESULT(Result::CallingStub);
}

void AudioGraphStub::OnNodeInsertFailure(shared_ptr<AudioNode>&, const Result&)
{
    LOOM_LOG_RESULT(Result::CallingStub);
}

void AudioGraphStub::OnNodeCreationFailure(shared_ptr<AudioNode>&)
{
    LOOM_LOG_RESULT(Result::CallingStub);
}

Result AudioGraphStub::RemoveNode(shared_ptr<AudioNode>&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioGraphStub::ConnectNodes(shared_ptr<AudioNode>&, shared_ptr<AudioNode>&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioGraphStub::ConnectNodes(initializer_list<shared_ptr<AudioNode>>&&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}


} // namespace Loom
