#pragma once

#include "loom/nodes/audionode.h"

namespace Loom
{

class AudioAsset;

enum class AudioSourceState
{
    WaitingAssetLoad,
    Stopped,
    Playing,
    Paused
};

using FadeFunction = void(*)(float&, float, u64, u64);

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

class AudioSource : public AudioNode
{
public:
    Result Play(float fadeIn = 0.0f)
    {
        _State = AudioSourceState::Playing;
        SetFade(FadeIn, fadeIn);
    }

    void SetFade(FadeFunction function, float duration)
    {
        if (_FadeFunction != nullptr && function == nullptr)
        {
            _FadeEndTime = Time::Now();
        }
        else
        {
            _FadeFunction = function;
            _FadeEndTime = Time::Now() + Time::SecondsToNanoseconds(duration);
        }
    }

    Result Pause(float fadeOut = 0.05f)
    {
        _State = AudioSourceState::Paused;
        SetFade(FadeOut, fadeOut);
    }

    Result Stop(float fadeOut = 0.05f)
    {
        _State = AudioSourceState::Stopped;
        SetFade(FadeOut, fadeOut);
    }

    Result Seek(u32 sample)
    {
        LOOM_RETURN_RESULT(Result::NotYetImplemented);
    }

    Result Seek(float seconds)
    {
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
        return _Virtual;
    }

private:
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
    atomic<AudioSourceState> _State;
    u32 _SamplePosition;
    u32 _Priority;
    bool _Virtual;
    bool _Loop;

    float _Volume;
    float _FadeGain;
    u64 _FadeEndTime;
    FadeFunction _FadeFunction;

    shared_ptr<AudioAsset> _AudioAsset;
};

} // namespace Loom
