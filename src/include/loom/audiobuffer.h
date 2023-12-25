#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

#include "loom/audioformat.h"

namespace Loom
{

class IAudioBufferProvider;

class AudioBuffer
{
public:
    u32 size;
    u8* data;
    u32 channels;
    u32 sampleRate;
    AudioFormat format;

public:
    AudioBuffer(IAudioBufferProvider* pool = nullptr, u8* data = nullptr, u32 capacity = 0);
    AudioBuffer(const AudioBuffer& other);
    AudioBuffer& operator=(const AudioBuffer& other);
    virtual ~AudioBuffer();

    template <class T = u8>
    T* GetData() const
    {
        return reinterpret_cast<T*>(data);
    }

    void Release();

    Result GetSampleCount(u32& sampleCount) const;
    Result GetFrameCount(u32& frameCount) const;
    bool FormatMatches(const AudioBuffer& other) const;


    Result AddSamplesFrom(const AudioBuffer& other);
    Result CopyDataFrom(const AudioBuffer& other) const;

    template <class T>
    Result MultiplySamplesBy(T multiplier)
    {
        if (multiplier == 1)
            return Result::Ok;
        if (!SampleFormatMatchHelper<T>())
            LOOM_RETURN_RESULT(Result::BufferFormatMismatch);
        T* samples = GetData<T>();
        u32 sampleCount = 0;
        Result result = GetSampleCount(sampleCount);
        LOOM_CHECK_RESULT(result);
        for (u32 i = 0; i < sampleCount; i++)
            samples[i] *= multiplier;
        return Result::Ok;
    }

private:
    void DecrementRefCount();

    template <class T>
    Result InternalAddSamplesFrom(const AudioBuffer& other)
    {
        T* source = other.GetData<T>();
        T* destination = GetData<T>();
        u32 sampleCount = 0;
        Result result = GetSampleCount(sampleCount);
        LOOM_CHECK_RESULT(result);
        for (u32 i = 0; i < sampleCount; i++)
            destination[i] += source[i];
        return Result::Ok;
    }

    template <class T>
    bool SampleFormatMatchHelper()
    {
        AudioFormat sampleFormat = format & AudioFormat::SampleFormatMask;
        AudioFormat typeFormat = SampleFormatFromType<T>();
        return sampleFormat == typeFormat;
    }

    template <class T>
    constexpr AudioFormat SampleFormatFromType()
    {
        if constexpr (std::is_same_v<T, s16>)
            return AudioFormat::Int16;
        else if constexpr (std::is_same_v<T, s32>)
            return AudioFormat::Int32;
        else if constexpr (std::is_same_v<T, float>)
            return AudioFormat::Float32;
        else
            return AudioFormat::Invalid;
    }

private:
    IAudioBufferProvider* _Pool;
    u32 _Capacity;
    atomic<u32>* _RefCount;
};

} // namespace Loom