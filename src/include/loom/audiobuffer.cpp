#include "loom/audioformat.h"

namespace Loom
{

AudioBuffer::AudioBuffer(IAudioBufferProvider* pool, u8* data, u32 capacity)
    : data(data)
    , size(0)
    , channels(0)
    , sampleRate(0)
    , format(AudioFormat::NotSpecified)
    , _Pool(pool)
    , _Capacity(capacity)
    , _RefCount(nullptr)
{
    if (_Pool != nullptr)
        _RefCount = new atomic<u32>(1);
}

virtual AudioBuffer::~AudioBuffer()
{
    DecrementRefCount();
}

AudioBuffer::AudioBuffer(const AudioBuffer& other)
    : data(other.data)
    , size(other.size)
    , channels(other.channels)
    , sampleRate(other.sampleRate)
    , format(other.format)
    , _Pool(other._Pool)
    , _Capacity(other._Capacity)
    , _RefCount(other._RefCount)
{
    if (_Pool != nullptr && _RefCount != nullptr)
        _RefCount->fetch_add(1);
}

AudioBuffer& AudioBuffer::operator=(const AudioBuffer& other)
{
    if (this != &other)
    {
        DecrementRefCount();
        data = other.data;
        size = other.size;
        channels = other.channels;
        sampleRate = other.sampleRate;
        format = other.format;
        _Pool = other._Pool;
        _Capacity = other._Capacity;
        _RefCount = other._RefCount;
        if (_Pool != nullptr && _RefCount != nullptr)
            _RefCount->fetch_add(1);
    }
    return *this;
}

template <class T = u8>
T* AudioBuffer::GetData() const
{
    return reinterpret_cast<T*>(data);
}

void AudioBuffer::Release()
{
    if (_Pool == nullptr)
        return;
    DecrementRefCount();
    _Pool = nullptr;
    if (_RefCount != nullptr && _RefCount->load() > 0)
        _RefCount = nullptr;
}

Result AudioBuffer::CopyDataFrom(const AudioBuffer& other) const
{
    if (data == nullptr || other.data == nullptr)
        LOOM_RETURN_RESULT(Result::NoData);
    if (_Capacity < other.size)
        LOOM_RETURN_RESULT(Result::BufferCapacityMismatch);
    memcpy(data, other.data, other.size);
    return Result::Ok;
}

Result AudioBuffer::AddSamplesFrom(const AudioBuffer& other)
{
    if (!FormatMatches(other))
        LOOM_RETURN_RESULT(Result::BufferFormatMismatch);
    AudioFormat sampleFormat = other.format & AudioFormat::SampleFormatMask;
    switch (sampleFormat)
    {
        case AudioFormat::Int16:
            return InternalAddSamplesFrom<s16>(other);
        case AudioFormat::Int32:
            return InternalAddSamplesFrom<s32>(other);
        case AudioFormat::Float32:
            return InternalAddSamplesFrom<float>(other);
        default:
            LOOM_RETURN_RESULT(Result::InvalidBufferSampleFormat);
    }
}

template <class T>
Result AudioBuffer::MultiplySamplesBy(T multiplier)
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

Result AudioBuffer::GetSampleCount(u32& sampleCount) const
{
    if (data != nullptr)
    {
        switch (format & AudioFormat::SampleFormatMask)
        {
            case AudioFormat::Int16:
                sampleCount = size / sizeof (s16);
                return Result::Ok;
            case AudioFormat::Int32:
            case AudioFormat::Float32:
                sampleCount = size / sizeof (s32);
                return Result::Ok;
            default:
                sampleCount = 0;
                LOOM_RETURN_RESULT(Result::InvalidBufferSampleFormat);
        }
    }
    LOOM_RETURN_RESULT(Result::NoData);
}

Result AudioBuffer::GetFrameCount(u32& frameCount) const
{
    Result result = GetSampleCount(frameCount);
    LOOM_CHECK_RESULT(result);
    frameCount /= channels;
    return Result::Ok;
}

bool AudioBuffer::FormatMatches(const AudioBuffer& other) const
{
    return format == other.format
        && sampleRate == other.sampleRate
        && channels == other.channels;
}

template <class T>
Result AudioBuffer::InternalAddSamplesFrom(const AudioBuffer& other)
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

void AudioBuffer::DecrementRefCount()
{
    if (_Pool != nullptr
        && _RefCount != nullptr
        && _RefCount->fetch_sub(1) == 1)
    {
        _Pool->ReleaseBuffer(*this);
        _Pool = nullptr;
        data = nullptr;
        delete _RefCount;
        _RefCount = nullptr;
    }
}

template <class T>
bool AudioBuffer::SampleFormatMatchHelper()
{
    AudioFormat sampleFormat = format & AudioFormat::SampleFormatMask;
    AudioFormat typeFormat = SampleFormatFromType<T>();
    return sampleFormat == typeFormat;
}

template <class T>
constexpr AudioBuffer::AudioFormat SampleFormatFromType()
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

} // namespace Loom
