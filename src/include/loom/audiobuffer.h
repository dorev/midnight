#pragma once

#include "loom/audioformat.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

class AudioBuffer
{
public:
    AudioBuffer(IAudioSystem& system = IAudioSystem::GetStub(), AudioFormat format = AudioFormat(), u8* data = nullptr, u32 capacity = 0);
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
    u32 GetFrameRate() const;
    SampleFormat GetSampleFormat() const;
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
        u32 sampleCount = GetSampleCount();
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
        u32 sampleCount = GetSampleCount();
        for (u32 i = 0; i < sampleCount; i++)
            destination[i] += source[i];
        return Result::Ok;
    }

    template <class T>
    bool SampleFormatMatchHelper()
    {
        SampleFormat sampleFormat = _Format.sampleFormat;
        SampleFormat typeFormat = TypeToSampleFormat<T>();
        return sampleFormat == typeFormat;
    }

private:
    IAudioSystem& _System;
    u32 _Capacity;
    u32 _Size;
    u8* _Data;
    AudioFormat _Format;
    atomic<u32>* _RefCount;
};

} // namespace Loom
