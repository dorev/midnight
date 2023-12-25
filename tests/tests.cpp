#include "gtest/gtest.h"
#include "loom/loom.h"

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

    Result Execute(AudioBuffer&)
    {
        return Result::Ok;
    }
};

TEST_F(CompilationTests, AudioGraph)
{
    AudioSystem system;
    AudioGraph& graph = static_cast<AudioGraph&>(system.GetGraphInterface());
    shared_ptr<TestNode> input = graph.CreateNode<TestNode>(system.GetInterface());
    shared_ptr<TestNode> gain = graph.CreateNode<TestNode>(system.GetInterface());
    shared_ptr<TestNode> reverb = graph.CreateNode<TestNode>(system.GetInterface());
    shared_ptr<TestNode> output = graph.CreateNode<TestNode>(system.GetInterface());

    Result result = graph.Chain(input, gain, reverb, output);
    EXPECT_EQ(result, Result::Ok);
}
