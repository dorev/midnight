#include "gtest/gtest.h"
#include "midnight.h"

using namespace Midnight;

class Tests : public ::testing::Test
{
};

class TestNode : public AudioNode
{
    virtual Error Process(const AudioBuffer& input, AudioBuffer& output)
    {
        return Ok;
    }
};

TEST_F(Tests, NextTest)
{
    SharedPtr<TestNode> input = MakeShared<TestNode>();
    SharedPtr<TestNode> gain = MakeShared<TestNode>();
    SharedPtr<TestNode> reverb = MakeShared<TestNode>();
    SharedPtr<TestNode> output = MakeShared<TestNode>();

    AudioNodeConnectionResult error = input >> gain >> reverb >> output;
}

