#include "gtest/gtest.h"
#include "midnight.h"

using namespace Midnight;

class CompilationTests : public ::testing::Test
{
};

class TestNode : public AudioNode
{
};

TEST_F(CompilationTests, AudioGraph)
{
    AudioSystem system;

    AudioGraph graph(static_cast<IAudioSystem&>(system));
    TestNode* input = graph.CreateNode<TestNode>();
    TestNode* gain = graph.CreateNode<TestNode>();
    TestNode* reverb = graph.CreateNode<TestNode>();
    TestNode* output = graph.CreateNode<TestNode>();

    Result error = graph.Chain(input, gain, reverb, output);
}

