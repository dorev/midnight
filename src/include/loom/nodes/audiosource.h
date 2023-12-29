#pragma once

#include "loom/nodes/audionode.h"
#include "loom/time.h"

namespace Loom
{

class AudioAsset;

enum class AudioSourceState
{
    Invalid,
    Initializing,
    Loading,
    Priming,
    Playing,
    Stopping,
    Stopped,
    Unloading
};

using FadeFunction = void(*)(float&, float, u64, u64);
/*
void LinearFade(float& gain, float targetGain, u64 currentTime, u64 endTime)
{
};
void FadeIn(float& gain, float targetGain, u64 currentTime, u64 endTime)
{
    LinearFade(gain, 1.0f, currentTime, endTime);
};
void FadeOut(float& gain, float targetGain, u64 currentTime, u64 endTime)
{
    LinearFade(gain, 0.0f, currentTime, endTime);
};
*/


class AudioSource : public AudioNode
{
public:
    AudioSource(IAudioSystem& system)
        : AudioNode(system)
        , _StateData(Initializing, 0, 0, nullptr)
    {
    }

    Result Play(float fadeIn = 0.0f)
    {
        return _StateData.state.Play(_StateData, fadeIn);
    }

    Result Stop(float fadeOut = 0.05f)
    {
        return _StateData.state.Stop(_StateData, fadeOut);
    }

    Result Seek(u32 sample)
    {
        LOOM_UNUSED(sample);
        LOOM_RETURN_RESULT(Result::NotYetImplemented);
    }

    Result Seek(float seconds)
    {
        LOOM_UNUSED(seconds);
        LOOM_RETURN_RESULT(Result::NotYetImplemented);
    }

    void SetLoop(bool loop)
    {
        _Loop = loop;
    }

    bool IsLooping() const
    {
        return _Loop;
    }

    bool IsVirtual() const
    {
        return Bypass();
    }

public:
    Result Execute(AudioBuffer& destinationBuffer) override
    {
        // make sure that the format specified in the destination buffer is respected!
        LOOM_UNUSED(destinationBuffer);
        LOOM_RETURN_RESULT(Result::NotYetImplemented);
    }

    const char* GetName() const override
    {
        return "AudioSource";
    }

    u64 GetTypeId() const override
    {
        return AudioNodeId::AudioSource;
    }

private:
    u32 _Id;
    u32 _SamplePosition;
    u32 _Priority;
    bool _Loop;
    float _Volume;
    shared_ptr<AudioAsset> _AudioAsset;

    class IState;
    struct StateData
    {
        StateData(IState& state, float fadeGain, u64 fadeEndTime, FadeFunction fadeFunction)
            : state(state)
            , fadeGain(fadeGain)
            , fadeEndTime(fadeEndTime)
            , fadeFunction(fadeFunction)
        {
        }

        IState& state;
        float fadeGain;
        u64 fadeEndTime;
        FadeFunction fadeFunction;
        // load callback
        // unload callback
        // playing callback
        // stopped callback
    }
    _StateData;

    class IState
    {
    public:
        virtual AudioSourceState GetState() = 0;
        virtual Result Play(StateData& context, float fade) = 0;
        virtual Result Stop(StateData& context, float fade) = 0;
    };

    static class InitializingState : public IState
    {
        AudioSourceState GetState() final override
        {
            return AudioSourceState::Initializing;
        }

        Result Play(StateData&, float) final override
        {
            return Result::NotYetImplemented;
        }

        Result Stop(StateData&, float) final override
        {
            return Result::NotYetImplemented;
        }
    }
    Initializing;

    static class PlayingState : public IState
    {
        AudioSourceState GetState() final override
        {
            return AudioSourceState::Playing;
        }

        Result Play(StateData&, float) final override
        {
            return Result::NotYetImplemented;
        }

        Result Stop(StateData&, float) final override
        {
            return Result::NotYetImplemented;
        }
    }
    Playing;
};

} // namespace Loom
