#pragma once

#include "loom/audiobuffer.h"

namespace Loom
{

class AudioAsset
{
public:
    AudioAsset(const char* name, AudioBuffer& buffer)
        : _Name(name)
        , _Buffer(buffer)
    {
        // Calculate duration
    }

    virtual ~AudioAsset()
    {
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

private:
    string _Name;
    float _Duration;
    AudioBuffer _Buffer;
};


} // namespace Loom
