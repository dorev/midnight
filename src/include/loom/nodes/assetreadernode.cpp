#pragma once

#include "loom/nodes/assetreadernode.h"
#include "loom/interfaces/iaudiosystem.h"
#include "loom/audioasset.h"

namespace Loom
{
    AssetReaderNode::AssetReaderNode(IAudioSystem& system, shared_ptr<AudioAsset> asset)
        : AudioNode(system)
        , _Asset(asset)
        , _State(Initializing)
        , _PendingEvent(NoEvent)
        , _FadeGain(0.0f)
    {
    }

    Result AssetReaderNode::Play(float fade)
    {
        _PendingEvent = PlayRequest;
        _FadeInDuration = fade;
        return Update();
    }

    Result AssetReaderNode::Stop(float fade)
    {
        _PendingEvent = StopRequest;
        _FadeOutDuration = fade;
        return Update();
    }

    Result AssetReaderNode::LoadAsset(bool& loaded)
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
                [[fallthrough]];
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

    Result AssetReaderNode::Update()
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

    Result AssetReaderNode::Execute(AudioBuffer& destinationBuffer)
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
                return Result::NodeIsVirtual;
            }
            break;
        case Virtualizing:
            if (_FadeGain == 0.0f)
            {
                _State = Virtual;
                return Result::NodeIsVirtual;
            }
            break;
        case Stopped:
        case Virtual:
            return Result::NodeIsVirtual;
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

    void AssetReaderNode::ConfigureFade(FadeFunction function, float duration)
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

    const char* AssetReaderNode::GetName() const
    {
        return "AudioSource";
    }

    u64 AssetReaderNode::GetTypeId() const
    {
        return AudioNodeId::AudioSource;
    }

    Result AssetReaderNode::SeekFrame(u32 frame)
    {
        LOOM_UNUSED(frame);
        LOOM_RETURN_RESULT(Result::NotYetImplemented);
    }

    Result AssetReaderNode::SeekTime(float seconds)
    {
        LOOM_UNUSED(seconds);
        LOOM_RETURN_RESULT(Result::NotYetImplemented);
    }

    u32 AssetReaderNode::GetFramePosition() const
    {
        return _FramePosition;
    }

    float AssetReaderNode::GetTimePosition() const
    {
        return GetFramePosition() / _Asset->GetFrames() * _Asset->GetDuration();
    }

    void AssetReaderNode::SetLoop(bool loop)
    {
        _Loop = loop;
    }

    bool AssetReaderNode::IsLooping() const
    {
        return _Loop;
    }

    bool AssetReaderNode::IsVirtual() const
    {
        return BypassNode();
    }

    bool AssetReaderNode::PlayIsRequested() const
    {
        return _PendingEvent.load() == PlayRequest;
    }

    bool AssetReaderNode::StopIsRequested() const
    {
        return _PendingEvent.load() == StopRequest;
    }

    bool AssetReaderNode::AssetIsLoaded() const
    {
        return _Asset != nullptr && _Asset->GetState() == AudioAssetState::Loaded;
    }

} // namespace Loom
