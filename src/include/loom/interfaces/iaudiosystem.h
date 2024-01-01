#pragma once

#include "loom/audiosystemconfig.h"
#include "loom/interfaces/iaudiosystemcomponent.h"

namespace Loom
{

class IAudioGraph;
class IAudioCodec;
class IAudioDeviceManager;
class IAudioResampler;
class IAudioChannelRemapper;
class IAudioBufferProvider;

class IAudioSystem : public IAudioSystemComponent
{
public:
    IAudioSystem();
    virtual ~IAudioSystem();
    static IAudioSystem& GetStub();
    IAudioSystem& GetInterface();
    AudioSystemComponentType GetType() const override;
    const char* GetName() const override;

    virtual const AudioSystemConfig& GetConfig() const = 0;
    virtual IAudioGraph& GetGraph() const = 0;
    virtual IAudioCodec& GetCodec() const = 0;
    virtual IAudioDeviceManager& GetDeviceManager() const = 0;
    virtual IAudioResampler& GetResampler() const = 0;
    virtual IAudioChannelRemapper& GetChannelRemapper() const = 0;
    virtual IAudioBufferProvider& GetBufferProvider() const = 0;
};

class AudioSystemStub : public IAudioSystem
{
public:
    static IAudioSystem& GetInstance();
    const AudioSystemConfig& GetConfig() const;
    IAudioGraph& GetGraph() const final override;
    IAudioCodec& GetCodec() const final override;
    IAudioDeviceManager& GetDeviceManager() const final override;
    IAudioResampler& GetResampler() const final override;
    IAudioChannelRemapper& GetChannelRemapper() const final override;
    IAudioBufferProvider& GetBufferProvider() const final override;
};

} // namespace Loom
