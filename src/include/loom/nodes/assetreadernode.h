#pragma once

#include "loom/nodes/audionode.h"
#include "loom/audioasset.h"
#include "loom/time.h"
#include "loom/fade.h"

namespace Loom
{

class AssetReaderNode : public AudioNode
{
public:
    static constexpr float VirtualFadeDuration = 0.05f;

    enum State
    {
        Invalid,
        Initializing,
        Loading,
        Playing,
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
        PlayRequest,
        StopRequest,
    };

    AssetReaderNode(IAudioSystem& system, shared_ptr<AudioAsset> asset);
    u64 GetTypeId() const override;
    const char* GetName() const override;
    Result Execute(AudioBuffer& destinationBuffer) override;

    Result Play(float fade = 0.0f);
    Result Stop(float fade = 0.05f);
    Result Update();
    Result LoadAsset(bool& loaded);
    void ConfigureFade(FadeFunction function, float duration);
    Result SeekFrame(u32 frame);
    Result SeekTime(float seconds);
    u32 GetFramePosition() const;
    float GetTimePosition() const;
    void SetLoop(bool loop);
    bool IsLooping() const;
    bool IsVirtual() const;

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
                    destinationData[i] = static_cast<T>(static_cast<float>(assetData[offset + i]) * _FadeGain);
                for (u32 i = 0; i < sizeAfterWrapAround; i++)
                    destinationData[sizeBeforeWrapAround + i] = static_cast<T>(static_cast<float>(assetData[i]) * _FadeGain);
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

private:
    bool PlayIsRequested() const;
    bool StopIsRequested() const;
    bool AssetIsLoaded() const;

private:
    u32 _Id;
    u32 _FramePosition;
    u32 _Priority;
    bool _Loop;
    float _Volume;
    shared_ptr<AudioAsset> _Asset;

    atomic<AssetReaderNode::Event> _PendingEvent;
    atomic<AssetReaderNode::State> _State;

    float _FadeGain;
    float _FadeInDuration;
    float _FadeOutDuration;
    u64 _FadeStartTime;
    u64 _FadeEndTime;
    FadeFunction _FadeFunction;
};

} // namespace Loom
