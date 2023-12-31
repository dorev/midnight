#pragma once

#include "loom/interfaces/iaudiosystemcomponent.h"

namespace Loom
{

class AudioBuffer;
class AudioAsset;

class IAudioCodec : public IAudioSystemComponent
{
public:
    IAudioCodec(IAudioSystem& system);
    AudioSystemComponentType GetType() const final override;
    virtual Result LoadAsset(const char* filePath, AudioAsset& audioFile) = 0;
};

class AudioCodecStub : public IAudioCodec
{
public:
    AudioCodecStub();
    static AudioCodecStub& GetInstance();
    const char* GetName() const final override;
    Result LoadAsset(const char* filePath, AudioAsset& audioFile) final override;
};


} // namespace Loom
