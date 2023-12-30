#pragma once

#include "loom/nodes/audionode.h"
#include "loom/audioasset.h"
#include "loom/time.h"

namespace Loom
{


using FadeFunction = void(*)(float&, float, u64, u64);

void LinearFade(float& gain, float targetGain, u64 startTime, u64 endTime)
{
    u64 now = Now();
    if (now >= endTime)
    {
        gain = targetGain;
        return;
    }
    else if (now <= startTime)
    {
        return;
    }
    float fadeRange = endTime - startTime;
    float fadeProgress = now - startTime;
    float fadeRatio = fadeProgress / fadeRange;
    float gainRange = targetGain - gain;
    gain += gainRange * fadeRatio;

};
void FadeIn(float& gain, float targetGain, u64 startTime, u64 endTime)
{
    LinearFade(gain, 1.0f, startTime, endTime);
};
void FadeOut(float& gain, float targetGain, u64 startTime, u64 endTime)
{
    LinearFade(gain, 0.0f, startTime, endTime);
};



class AudioSource : public AudioNode
{
public:
    static constexpr float VirtualFadeDuration = 0.05f;

    enum State
    {
        Invalid,
        Initializing,
        Loading,
        Playing,
        ToStop,
        Stopping,
        Stopped,
        Virtualizing,
        Virtual,
        Devirtualizing,
        Unloading,
        Unloaded
    };

    enum Event
    {
        NoEvent,
        Play,
        Stop,
    };

    AudioSource(IAudioSystem& system, shared_ptr<AudioAsset> asset)
        : AudioNode(system)
        , _Asset(asset)
        , _State(Initializing)
        , _PendingEvent(NoEvent)
    {
    }

    Result Play(float fade = 0.0f)
    {
        _PendingEvent = Play;
        _FadeInDuration = fade;
        return Update();
    }

    Result Stop(float fade = 0.05f)
    {
        _PendingEvent = Stop;
        _FadeOutDuration = fade;
        return Update();
    }

    Result LoadAsset(bool& loaded)
    {
        if (_Asset != nullptr)
        {
            switch (_Asset->GetState())
            {
            case AudioAssetState::Loaded:
                loaded = true;
                return Result::Ok;
            case AudioAssetState::Unloaded:
                _Asset->Load();
                [[fallthrough]]
            case AudioAssetState::Loading:
            case AudioAssetState::Unloading:
                loaded = false;
                return Result::Ok;
            default:
                break;
            }
        }
        loaded = false;
        LOOM_RETURN_RESULT(Result::InvalidFile);
    }

    // Update should be the only method altering the state
    Result Update()
    {
        switch(_State)
        {
        case Initializing:
        case Loading:
            {
                bool assetIsLoaded = false;
                Result result = LoadAsset(assetIsLoaded);
                LOOM_CHECK_RESULT(result);
                if (assetIsLoaded)
                {
                    if (PlayIsRequested())
                    {
                        ConfigureFade(FadeIn, _FadeInDuration);
                        _State = Playing;
                    }
                    else
                        _State = Stopped;
                }
                else
                {
                    _State = Loading;
                }
            }
            return Result::Ok;
        case Playing:
            if (IsVirtual())
            {
                ConfigureFade(FadeOut, VirtualFadeDuration)
                _State = Virtualizing;
            }
            if (StopIsRequested())
            {
                ConfigureFade(FadeOut, _FadeOutDuration);
                _State = Stopping;
            }
            return Result::Ok;
        case Stopping:
        case Stopped:
            if (PlayIsRequested())
            {
                ConfigureFade(FadeIn, _FadeInDuration);
                _State = Playing;
            }
            return Result::Ok;
        case Virtualizing:
        case Virtual:
            if (!IsVirtual())
                _State = Devirtualizing;
            return Result::Ok;
        case Devirtualizing:
            if (StopIsRequested())
            {
                ConfigureFade(FadeOut, _FadeOutDuration);
                _State = Stopping;
            }
            return Result::Ok;
        case Unloading:
        case Unloaded:
            LOOM_RETURN_RESULT(Result::NotYetImplemented);
        case Invalid:
        default:
            LOOM_RETURN_RESULT(Result::InvalidState);
        }
    }

    Result Execute(AudioBuffer& destinationBuffer) override
    {
        switch(_State)
        {
        case Initializing:
        case Loading:
            // check buffer template requirements!!
            return Result::NotReady;
        case Playing:
        {
            
        }
        case Stopping:
            if (_FadeGain == 0.0f)
                _State = Stopped;
        case Stopped:
        case Virtualizing:
            if (_FadeGain == 0.0f)
                _State = Virtual;
        case Virtual;
        case Devirtualizing:
        case Unloading:
        case Unloaded:
            LOOM_RETURN_RESULT(Result::NotYetImplemented);

        case Invalid:
        default:
            LOOM_RETURN_RESULT(Result::InvalidState);
        }
        return Result::Ok;

        // make sure that the format specified in the destination buffer is respected!
        LOOM_UNUSED(destinationBuffer);
        LOOM_RETURN_RESULT(Result::NotYetImplemented);
    }

    void ConfigureFade(FadeFunction function, float duration)
    {
        _FadeFunction = function;
        if (function != nullptr)
        {
            _FadeStartTime = Now();
            _FadeEndTime = _FadeStartTime + static_cast<u64>(duration * NanosecondsPerSecond);
        }
    }

    const char* GetName() const override
    {
        return "AudioSource";
    }

    u64 GetTypeId() const override
    {
        return AudioNodeId::AudioSource;
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

private:
    bool PlayIsRequested() const
    {
        return _PendingEvent.load() == Play;
    }

    bool StopIsRequested() const
    {
        return _PendingEvent.load() == Stop;
    }

    bool AssetIsLoaded() const
    {
        return _Asset != nullptr && _Asset->GetState() == AudioAssetState::Loaded;
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

    float _FadeGain;
    float _FadeInDuration;
    float _FadeOutDuration;
    u64 _FadeStartTime;
    u64 _FadeEndTime;
    FadeFunction _FadeFunction;
};

} // namespace Loom
