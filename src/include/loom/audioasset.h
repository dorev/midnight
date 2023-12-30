#pragma once

#include "loom/audiobuffer.h"
#include "loom/iaudiosystem.h"

namespace Loom
{

enum class AudioAssetState
{
    Invalid,
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
        // Calculate duration
    }

    virtual ~AudioAsset()
    {
    }

    Result Load()
    {
        return Result::NotYetImplemented;
    }

    Result Unload()
    {
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
