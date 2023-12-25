#pragma once

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
    virtual ~IAudioSystem();
    static IAudioSystem& GetStub();
    IAudioSystem& GetInterface();
    virtual const AudioSystemConfig& GetConfig() const = 0;
    virtual IAudioGraph& GetGraphInterface() = 0;
    virtual IAudioCodec& GetCodecInterface() const = 0;
    virtual IAudioDeviceManager& GetDeviceManagerInterface() const = 0;
    virtual IAudioResampler& GetResamplerInterface() const = 0;
    virtual IAudioChannelRemapper& GetChannelRemapperInterface() const = 0;
    virtual IAudioBufferProvider& GetBufferProviderInterface() const = 0;
};

class AudioSystemStub : public IAudioSystem
{
public:
    static IAudioSystem& GetInstance();
    const AudioSystemConfig& GetConfig() const;
    IAudioGraph& GetGraphInterface() final override;
    IAudioCodec& GetCodecInterface() const final override;
    IAudioDeviceManager& GetDeviceManagerInterface() const final override;
    IAudioResampler& GetResamplerInterface() const final override;
    IAudioChannelRemapper& GetChannelRemapperInterface() const final override;
    IAudioBufferProvider& GetBufferProviderInterface() const final override;
};

} // namespace Loom
