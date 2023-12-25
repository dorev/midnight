#pragma once

#include "loom/iaudiosubsystem.h"

namespace Loom
{

class AudioBuffer;

class IAudioFile
{
public:
    virtual ~IAudioFile()
    {
    }

    virtual Result Seek(float seconds) = 0;
    virtual Result Seek(u32 frame) = 0;
    virtual Result GetFramePosition(u32& frame) = 0;
    virtual Result GetTimePosition(float& seconds) = 0;
    virtual Result Read(u32 frameCountRequested, AudioBuffer& buffer) = 0;
};

class IAudioCodec : public IAudioSubsystem
{
public:
    IAudioCodec(IAudioSystem& system);
    AudioSubsystemType GetType() const final override;
    virtual Result CreateSampleBuffer(const char* filePath, AudioBuffer& destination) = 0;
    virtual Result OpenFile(const char* filePath, IAudioFile& audioFile) = 0;
};

class AudioCodecStub : public IAudioCodec
{
public:
    AudioCodecStub();
    static AudioCodecStub& GetInstance();
    const char* GetName() const final override;
    virtual Result CreateSampleBuffer(const char*, AudioBuffer&) final override;
    virtual Result OpenFile(const char*, IAudioFile&) final override;
};


} // namespace Loom
