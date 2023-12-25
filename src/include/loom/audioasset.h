#pragma once

#include "loom/defines.h"
#include "loom/types.h"

#include "loom/audiobuffer.h"

namespace Loom
{

class AudioAsset
{
public:
    AudioAsset(const string& name, AudioBuffer& buffer)
        : _Name(name)
        , _Buffer(buffer)
        , _dB(0.f)
    {
    }

    virtual ~AudioAsset()
    {
    }

    const string& GetName() const
    {
        return _Name;
    }

    float GetVolume() const
    {
        return _dB;
    }

    void SetVolume(float volume)
    {
        _dB = volume;
    }

    const AudioBuffer& GetBuffer() const
    {
        return _Buffer;
    }

private:
    string _Name;
    float _dB;
    AudioBuffer _Buffer;
};


} // namespace Loom
