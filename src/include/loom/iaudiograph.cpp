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

void IAudioGraph::VisitNode(shared_ptr<AudioNode> node)
{
    node->_Visited = true;
}

void IAudioGraph::ClearNodeVisit(shared_ptr<AudioNode> node)
{
    node->_Visited = false;
}

bool IAudioGraph::NodeWasVisited(shared_ptr<AudioNode> node)
{
    return node->_Visited;
}

set<shared_ptr<AudioNode>>& IAudioGraph::GetNodeOutputNodes(shared_ptr<AudioNode> node)
{
    return node->_OutputNodes;
}

set<shared_ptr<AudioNode>>& IAudioGraph::GetNodeInputNodes(shared_ptr<AudioNode> node)
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
    return AudioGraphState::Busy;
}

} // namespace Loom
