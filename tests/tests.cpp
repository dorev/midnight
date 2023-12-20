#include "gtest/gtest.h"
#include "loom.h"

using namespace Loom;

class CompilationTests : public ::testing::Test
{
};

class TestNode : public AudioNode
{
public:
    TestNode(IAudioSystem& system)
        : AudioNode(system, "TestNode")
    {
    }

    Result Execute(AudioBuffer& destinationBuffer)
    {
        return Result::Ok;
    }
};

TEST_F(CompilationTests, AudioGraph)
{
    AudioSystem system;
    AudioGraph& graph = system.GetGraph();
    TestNode* input = graph.CreateNode<TestNode>(system.GetInterface());
    TestNode* gain = graph.CreateNode<TestNode>(system.GetInterface());
    TestNode* reverb = graph.CreateNode<TestNode>(system.GetInterface());
    TestNode* output = graph.CreateNode<TestNode>(system.GetInterface());

    Result error = graph.Chain(input, gain, reverb, output);
}

