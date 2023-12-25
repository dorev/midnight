#pragma once

#include "loom/defines.h"
#include "loom/types.h"

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
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::Decoder;
    }

    virtual Result CreateSampleBuffer(const char* filePath, AudioBuffer& destination) = 0;
    virtual Result OpenFile(const char* filePath, IAudioFile& audioFile) = 0;
};

class AudioCodecStub : public IAudioCodec
{
public:
    static AudioCodecStub& GetInstance()
    {
        static AudioCodecStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioCodec stub";
    }

    virtual Result CreateSampleBuffer(const char*, AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    virtual Result OpenFile(const char*, IAudioFile&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }
};


} // namespace Loom
