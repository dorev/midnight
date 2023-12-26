#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

#include "loom/audionode.h"

namespace Loom
{

class AudioAsset;

class AudioSource : public AudioNode
{
public:
    Result Play(float fadeIn = 0.0f);
    Result Pause(float fadeOut = 0.05f);
    Result Stop(float fadeOut = 0.05f);
    Result Seek(u32 sample);
    Result Seek(float seconds);
    bool IsVirtual();
    bool loop;

private:
    Result Execute(AudioBuffer& destinationBuffer) override
    {
        // make sure that the format specified in the destination buffer is respected!
        LOOM_UNUSED(destinationBuffer);
        return Result::Ok;
    }

    const char* GetName() const
    {
        return "AudioSource";
    }

    u64 GetTypeId() const
    {
        return AudioNodeId::AudioSource;
    }

private:
    u32 _Id;
    u32 _Priority;
    bool _Virtual;
    shared_ptr<AudioAsset> _AudioAsset;
};

} // namespace Loom
