#pragma once

#include "loom/nodes/audionode.h"

namespace Loom
{

class AudioAsset;

class AudioSource : public AudioNode
{
public:
    Result Play(float fadeIn = 0.0f)
    {

    }

    Result Pause(float fadeOut = 0.05f);
    Result Stop(float fadeOut = 0.05f);

    Result Seek(u32 sample)
    {
        return Result::NotYetImplemented;
    }

    Result Seek(float seconds)
    {
        return Result::NotYetImplemented;
    }

    bool IsVirtual() const
    {
        return _Virtual;
    }

    bool loop;

private:
    Result Execute(AudioBuffer& destinationBuffer) override
    {
        // make sure that the format specified in the destination buffer is respected!
        LOOM_UNUSED(destinationBuffer);
        return Result::Ok;
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
    u32 _Position;
    u32 _Priority;
    bool _Virtual;
    shared_ptr<AudioAsset> _AudioAsset;
};

} // namespace Loom
