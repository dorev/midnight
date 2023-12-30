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
    Virtualizing,
    Virtual,
    Devirtualizing,
    Unloading,
    Unloaded
};

enum class AudioSourceEvent
{
    NoEvent,
    Play,
    Stop,
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
    AudioSource(IAudioSystem& system, shared_ptr<AudioAsset> asset)
        : AudioNode(system)
        , _Asset(asset)
        , _State(AudioSourceState::Initializing)
    {
        Result result = LoadAsset();
    }

    Result Play(float fade = 0.0f)
    {
        _PendingEvent = AudioSourceEvent::Play;
        _FadeDuration = fade;
        return Update();
    }

    Result Stop(float fade = 0.05f)
    {
        _PendingEvent = AudioSourceEvent::Stop;
        _FadeDuration = fade;
        return Update();
    }

    Result LoadAsset()
    {
        if (_Asset != nullptr)
        {
            switch (_State)
            {
            case AudioSourceState::Initializing:
            case AudioSourceState::Loading:
                switch (_Asset->GetState())
                {
                case AudioAssetState::Loaded:
                    return Result::Ok;
                case AudioAssetState::Unloaded:
                    _Asset->Load();
                    [[fallthrough]]
                case AudioAssetState::Loading:
                case AudioAssetState::Unloading:
                    return Result::NotReady;
                default:
                    break;
                }
                break;
            default:
                return Result::Ok;
            }
        }
        _State = AudioSourceState::Invalid;
        LOOM_RETURN_RESULT(Result::InvalidFile);
    }

    Result Update()
    {
        Result result = Result::Ok;
        AudioSourceState state = _State.load();
        switch(state)
        {
        case AudioSourceState::Initializing:
        case AudioSourceState::Loading:
            result = LoadAsset();
            switch (result)
            {
            case Result::Ok:
                switch (_PendingEvent)
                {
                case AudioSourceEvent::Play:
                    _State = AudioSourceState::Priming;
                    break;
                case AudioSourceEvent::NoEvent:
                case AudioSourceEvent::Stop:
                default:
                    _State = AudioSourceState::Stopped;
                    break;
                }
                [[fallthrough]]
            case Result::NotReady:
                return Result::Ok;
            default:
                return result;
            }

        case AudioSourceState::Priming:
        case AudioSourceState::Playing:
            return Result::Ok;

        case AudioSourceState::Stopping:
        case AudioSourceState::Stopped:
            _State = AudioSourceState::Priming;
            return Result::Ok;
    
        case AudioSourceState::Unloading:
        case AudioSourceState::Unloaded:

        case AudioSourceState::Invalid:
        default:
            break;
        }
        return Result::NotYetImplemented;
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

    AudioSourceState GetState() const
    {
        return _State;
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
    shared_ptr<AudioAsset> _Asset;
    atomic<AudioSourceEvent> _PendingEvent;
    atomic<AudioSourceState> _State;
    float _FadeDuration;
    float _FadeGain;
    u64 _FadeEndTime;
    FadeFunction _
    // load callback
    // unload callback
    // playing callback
    // stopped callback
};

} // namespace Loom
