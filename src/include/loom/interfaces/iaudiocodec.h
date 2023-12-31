#pragma once

#include "loom/interfaces/iaudiosubsystem.h"

namespace Loom
{

class AudioBuffer;
class AudioAsset;

class IAudioCodec : public IAudioSubsystem
{
public:
    IAudioCodec(IAudioSystem& system);
    static IAudioCodec& GetStub();
    IAudioCodec& GetInterface();
    const char* GetName() const override;
    AudioSubsystemType GetType() const final override;
    virtual Result LoadAsset(const char* filePath, AudioAsset& audioFile);
};

} // namespace Loom
