#pragma once

#include "loom/nodes/audionode.h"
#include "loom/audioasset.h"
#include "loom/time.h"
#include "loom/fade.h"

namespace Loom
{

class AssetReadingNode : public AudioNode
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

    AssetReadingNode(IAudioSystem& system, shared_ptr<AudioAsset> asset)
        : AudioNode(system)
        , _Asset(asset)
        , _State(Initializing)
        , _PendingEvent(NoEvent)
        , _FadeGain(0.0f)
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
                ConfigureFade(FadeOut, VirtualFadeDuration);
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
            return Result::NotReady;
        case Playing:
        case Devirtualizing:
            break;
        case Stopping:
            if (_FadeGain == 0.0f)
            {
                _State = Stopped;
                return Result::Ignore;
            }
            break;
        case Virtualizing:
            if (_FadeGain == 0.0f)
            {
                _State = Virtual;
                return Result::Ignore;
            }
            break;
        case Stopped:
        case Virtual:
            return Result::Ignore;
        case Unloading:
        case Unloaded:
            LOOM_RETURN_RESULT(Result::NotYetImplemented);
        case Invalid:
        default:
            LOOM_RETURN_RESULT(Result::InvalidState);
        }
        AudioBuffer assetBuffer = _Asset->GetBuffer();
        if (!assetBuffer.FormatMatches(destinationBuffer))
            LOOM_RETURN_RESULT(Result::BufferFormatMismatch);
        u32 offset = _FramePosition * assetBuffer.GetChannels() * assetBuffer.GetSampleSize();
        u32 sizeBeforeWrapAround = destinationBuffer.GetSize();
        u32 sizeAfterWrapAround = 0;
        u32 sizeLeftToRead = assetBuffer.GetSize() - offset;
        if (sizeBeforeWrapAround > sizeLeftToRead)
        {
            sizeAfterWrapAround = sizeBeforeWrapAround - sizeLeftToRead;
            sizeBeforeWrapAround = sizeLeftToRead;
        }
        switch (destinationBuffer.GetSampleFormat())
        {
            case SampleFormat::Int16:
                TransferBuffer<s16>(destinationBuffer, offset, sizeBeforeWrapAround, sizeAfterWrapAround);
                break;
            case SampleFormat::Int32:
                TransferBuffer<s32>(destinationBuffer, offset, sizeBeforeWrapAround, sizeAfterWrapAround);
                break;
            case SampleFormat::Float32:
                TransferBuffer<float>(destinationBuffer, offset, sizeBeforeWrapAround, sizeAfterWrapAround);
                break;
            default:
                LOOM_RETURN_RESULT(Result::InvalidBufferSampleFormat);
        }
        _FramePosition += destinationBuffer.GetFrameCount();
        return Result::Ok;
    }

    template <class T>
    void TransferBuffer(AudioBuffer& destinationBuffer, u32 offset, u32 sizeBeforeWrapAround, u32 sizeAfterWrapAround)
    {
        T* destinationData = destinationBuffer.GetData<T>();
        T* assetData = _Asset->GetBuffer().GetData<T>();
        sizeBeforeWrapAround /= sizeof(T);
        sizeAfterWrapAround /= sizeof(T);
        if (_FadeFunction != nullptr)
        {
            _FadeFunction(_FadeGain, _FadeStartTime, _FadeEndTime);
            if ((_FadeFunction == FadeIn && _FadeGain == 1.0f) || (_FadeFunction == FadeOut && _FadeGain == 0.0f))
                _FadeFunction = nullptr;
            if (_FadeGain == 0.0f)
            {
                memset(destinationData, 0, destinationBuffer.GetSize());
            }
            else
            {
                for (u32 i = 0; i < sizeBeforeWrapAround; i++)
                    destinationData[i] = assetData[offset + i] * _FadeGain;
                for (u32 i = 0; i < sizeAfterWrapAround; i++)
                    destinationData[sizeBeforeWrapAround + i] = assetData[i] * _FadeGain;
            }
        }
        else
        {
            for (u32 i = 0; i < sizeBeforeWrapAround; i++)
                destinationData[i] = assetData[offset + i];
            for (u32 i = 0; i < sizeAfterWrapAround; i++)
                destinationData[sizeBeforeWrapAround + i] = assetData[i];
        }
    }

    void ConfigureFade(FadeFunction function, float duration)
    {
        _FadeFunction = function;
        if (function != nullptr)
        {
            _FadeStartTime = Now();
            _FadeEndTime = _FadeStartTime + static_cast<u64>(duration * NanosecondsPerSecond);
            if (function == FadeIn)
                _FadeGain = 0.0f;
            else if (function == FadeOut)
                _FadeGain = 1.0f;
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

    Result SeekFrame(u32 frame)
    {
        LOOM_UNUSED(frame);
        LOOM_RETURN_RESULT(Result::NotYetImplemented);
    }

    Result SeekTime(float seconds)
    {
        LOOM_UNUSED(seconds);
        LOOM_RETURN_RESULT(Result::NotYetImplemented);
    }

    u32 GetFramePosition() const
    {
        return _FramePosition;
    }

    float GetTimePosition() const
    {
        return GetFramePosition() / _Asset->GetFrames() * _Asset->GetDuration();
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

    AssetReadingNode::State GetState() const
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
    u32 _FramePosition;
    u32 _Priority;
    bool _Loop;
    float _Volume;
    shared_ptr<AudioAsset> _Asset;

    atomic<AssetReadingNode::Event> _PendingEvent;
    atomic<AssetReadingNode::State> _State;

    float _FadeGain;
    float _FadeInDuration;
    float _FadeOutDuration;
    u64 _FadeStartTime;
    u64 _FadeEndTime;
    FadeFunction _FadeFunction;
};

} // namespace Loom
