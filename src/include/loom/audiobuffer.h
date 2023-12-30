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
    AudioBuffer(AudioFormat format = AudioFormat::NotSpecified, IAudioBufferProvider* pool = nullptr, u8* data = nullptr, u32 capacity = 0);
    AudioBuffer(const AudioBuffer& other);
    AudioBuffer& operator=(const AudioBuffer& other);
    virtual ~AudioBuffer();

    template <class T = u8>
    T* GetData() const
    {
        return reinterpret_cast<T*>(_Data);
    }

    void Release();
    u32 GetSampleCount() const;
    u32 GetFrameCount() const;
    bool FormatMatches(const AudioBuffer& other) const;
    u32 GetChannels() const;
    u32 GetSampleRate() const;
    AudioFormat GetSampleFormat() const;
    u32 GetSampleSize() const;
    u32 GetSize() const;
    AudioFormat GetFormat() const;

    Result AddSamplesFrom(const AudioBuffer& other);
    Result CloneDataFrom(const AudioBuffer& other);
    Result CopyDataFrom(const AudioBuffer& other, u32 offset, u32 size);

    template <class T>
    Result MultiplySamplesBy(T multiplier)
    {
        if (multiplier == 1)
            return Result::Ok;
        if (multiplier == 0)
        {
            memset(_Data, 0, _Size);
            return Result::Ok;
        }
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
        AudioFormat sampleFormat = _Format & AudioFormat::SampleFormatMask;
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
    u32 _Capacity;
    u32 _Size;
    u8* _Data;
    AudioFormat _Format;
    IAudioBufferProvider* _Pool;
    atomic<u32>* _RefCount;
};

} // namespace Loom
