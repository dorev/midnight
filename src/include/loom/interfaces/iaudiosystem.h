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
    virtual const AudioSystemConfig& GetConfig() const;
    virtual IAudioGraph& GetGraph();
    virtual IAudioCodec& GetCodec() const;
    virtual IAudioDeviceManager& GetDeviceManager() const;
    virtual IAudioResampler& GetResampler() const;
    virtual IAudioChannelRemapper& GetChannelRemapper() const;
    virtual IAudioBufferProvider& GetBufferProvider() const;
};

} // namespace Loom
