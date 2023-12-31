#pragma once

#include "loom/audiobuffer.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

enum class AudioAssetState
{
    InvalidPath,
    Unloaded,
    Loading,
    Loaded,
    Unloading
};

using AudioAssetCallback = void(*)();

class AudioAsset
{
public:
    AudioAsset(IAudioSystem& system, const char* name, const char* filePath)
        : _System(system)
        , _Name(name)
        , _FilePath(filePath)
        , _State(AudioAssetState::Unloaded)
    {
    }

    AudioAsset(const AudioAsset& other) = delete;
    AudioAsset& operator=(const AudioAsset& other) = delete;

    virtual ~AudioAsset()
    {
    }

    Result Load()
    {
        _System.LoadAsset(*this);
        return Result::NotYetImplemented;
    }

    Result Unload()
    {
        _System.UnloadAsset(*this);
        return Result::NotYetImplemented;
    }

    const char* GetName() const
    {
        return _Name.c_str();
    }

    float GetDuration() const
    {
        return _Duration;
    }

    u32 GetFrames() const
    {
        u32 frameCount = _Buffer.GetFrameCount();
        if (frameCount == 0)
            LOOM_LOG_RESULT(Result::InvalidBufferFrameRateFormat);
        return frameCount;
    }

    const AudioBuffer& GetBuffer() const
    {
        return _Buffer;
    }

    AudioAssetState GetState() const
    {
        return _State;
    }

private:
    IAudioSystem& _System;
    string _Name;
    string _FilePath;
    AudioAssetState _State;
    float _Duration;
    AudioBuffer _Buffer;
};


} // namespace Loom
