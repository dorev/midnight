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
    T* GetData() const;
    void Release();

    Result GetSampleCount(u32& sampleCount) const;
    Result GetFrameCount(u32& frameCount) const;
    bool FormatMatches(const AudioBuffer& other) const;

    template <class T>
    Result MultiplySamplesBy(T multiplier);
    Result AddSamplesFrom(const AudioBuffer& other);
    Result CopyDataFrom(const AudioBuffer& other) const;

private:
    void DecrementRefCount();
    template <class T>
    Result InternalAddSamplesFrom(const AudioBuffer& other);
    template <class T>
    bool SampleFormatMatchHelper();
    template <class T>
    constexpr AudioFormat SampleFormatFromType();

private:
    IAudioBufferProvider* _Pool;
    u32 _Capacity;
    atomic<u32>* _RefCount;
};

} // namespace Loom
