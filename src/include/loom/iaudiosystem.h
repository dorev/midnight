#pragma once

#include "loom/defines.h"
#include "loom/types.h"

#include "loom/audiosystemconfig.h"

namespace Loom
{

class IAudioGraph;
class IAudioCodec;
class IAudioDeviceManager;
class IAudioResampler;
class IAudioChannelRemapper;
class IAudioBufferProvider;

class IAudioSystem
{
public:
    virtual ~IAudioSystem()
    {
    }

    IAudioSystem& GetInterface()
    {
        return *this;
    }

    virtual const AudioSystemConfig& GetConfig() const = 0;
    virtual IAudioGraph& GetGraphInterface() = 0;
    virtual IAudioCodec& GetDecoderInterface() const = 0;
    virtual IAudioDeviceManager& GetDeviceManagerInterface() const = 0;
    virtual IAudioResampler& GetResamplerInterface() const = 0;
    virtual IAudioChannelRemapper& GetChannelRemapperInterface() const = 0;
    virtual IAudioBufferProvider& GetBufferProviderInterface() const = 0;
};

} // namespace Loom
