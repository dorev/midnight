#include "loom/iaudiograph.h"
#include "loom/audionode.h"

namespace Loom
{

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

} // namespace Loom
