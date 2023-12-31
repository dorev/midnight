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
        : AudioNode(system)
    {
    }

    Result Execute(AudioBuffer&)
    {
        return Result::Ok;
    }

    const char* GetName() const
    {
        return "TestNode";
    }

    u64 GetTypeId() const
    {
        return 0;
    }
};

TEST_F(CompilationTests, AudioGraph)
{
    AudioSystem system;
    IAudioGraph& graph = system.GetGraph();
    AudioNodePtr input = graph.CreateNode<TestNode>();
    AudioNodePtr gain = graph.CreateNode<TestNode>();
    AudioNodePtr reverb = graph.CreateNode<TestNode>();
    AudioNodePtr output = graph.CreateNode<TestNode>();

    Result result = graph.ConnectNodes({input, gain, reverb, output});
    LOOM_UNUSED(result);
}
