#include "loom/audiobuffer.h"
#include "loom/interfaces/iaudiobufferprovider.h"

namespace Loom
{

AudioBuffer::AudioBuffer(IAudioSystem& system, AudioFormat format, u8* data, u32 capacity)
    : _System(system)
    , _Data(data)
    , _Size(0)
    , _Format(format)
    , _Capacity(capacity)
    , _RefCount(nullptr)
{
    if (_Data != nullptr)
        _RefCount = new atomic<u32>(1);
}

AudioBuffer::~AudioBuffer()
{
    DecrementRefCount();
}

AudioBuffer::AudioBuffer(const AudioBuffer& other)
    : _System(other._System)
    , _Data(other._Data)
    , _Size(other._Size)
    , _Format(other._Format)
    , _Capacity(other._Capacity)
    , _RefCount(other._RefCount)
{
    if (_RefCount != nullptr)
        _RefCount->fetch_add(1);
}

AudioBuffer& AudioBuffer::operator=(const AudioBuffer& other)
{
    if (this != &other)
    {
        DecrementRefCount();
        _System = other._System;
        _Data = other._Data;
        _Size = other._Size;
        _Format = other._Format;
        _Capacity = other._Capacity;
        _RefCount = other._RefCount;
        if (_RefCount != nullptr)
            _RefCount->fetch_add(1);
    }
    return *this;
}

void AudioBuffer::Release()
{
    DecrementRefCount();
    if (_RefCount != nullptr && _RefCount->load() > 0)
        _RefCount = nullptr;
}

Result AudioBuffer::CloneDataFrom(const AudioBuffer& other)
{
    if (_Data == nullptr || other._Data == nullptr)
        LOOM_RETURN_RESULT(Result::NoData);
    if (_Capacity < other._Size)
        LOOM_RETURN_RESULT(Result::BufferCapacityMismatch);
    memcpy(_Data, other._Data, other._Size);
    _Size = other._Size;
    return Result::Ok;
}

Result AudioBuffer::CopyDataFrom(const AudioBuffer& other, u32 offset, u32 size)
{
    if (_Data == nullptr || other._Data == nullptr)
        LOOM_RETURN_RESULT(Result::NoData);
    if (size == 0)
        LOOM_RETURN_RESULT(Result::InvalidParameter);
    if (size > _Capacity)
        LOOM_RETURN_RESULT(Result::BufferCapacityMismatch);
    if ((offset + size) > other._Size)
        LOOM_RETURN_RESULT(Result::ExceedingLimits);
    memcpy(_Data, other._Data + offset, size);
    _Size = size;
    return Result::Ok;
}

Result AudioBuffer::AddSamplesFrom(const AudioBuffer& other)
{
    if (!FormatMatches(other))
        LOOM_RETURN_RESULT(Result::BufferFormatMismatch);
    SampleFormat sampleFormat = other.GetSampleFormat();
    switch (sampleFormat)
    {
        case SampleFormat::Int16:
            return InternalAddSamplesFrom<s16>(other);
        case SampleFormat::Int32:
            return InternalAddSamplesFrom<s32>(other);
        case SampleFormat::Float32:
            return InternalAddSamplesFrom<float>(other);
        default:
            LOOM_RETURN_RESULT(Result::InvalidBufferSampleFormat);
    }
}

u32 AudioBuffer::GetSampleCount() const
{
    u32 sampleCount = 0;
    if (_Data == nullptr)
    {
        LOOM_LOG_RESULT(Result::NoData);
    }
    else
    {
        u32 sampleSize = GetSampleSize();
        if (sampleSize > 0)
            sampleCount = _Size / sampleSize;
        else
            LOOM_LOG_RESULT(Result::InvalidBufferSampleFormat);
    }
    return sampleCount;
}

u32 AudioBuffer::GetFrameCount() const
{
    u32 frameCount = GetSampleCount();
    u32 channels = GetChannels();
    if (channels > 0)
        return frameCount /= channels;
    else
        return 0;

}

bool AudioBuffer::FormatMatches(const AudioBuffer& other) const
{
    return _Format == other._Format;
}

u32 AudioBuffer::GetChannels() const
{
    return _Format.channels;
}

u32 AudioBuffer::GetFrameRate() const
{
    return _Format.frameRate;
}

SampleFormat AudioBuffer::GetSampleFormat() const
{
    return _Format.sampleFormat;
}

u32 AudioBuffer::GetSampleSize() const
{
    switch (GetSampleFormat())
    {
        case SampleFormat::Int16: return sizeof(s16);
        case SampleFormat::Int32: return sizeof(s32);
        case SampleFormat::Float32: return sizeof(float);
        default:
            LOOM_LOG_RESULT(Result::InvalidBufferSampleFormat);
            return 0;
    }
}

void AudioBuffer::DecrementRefCount()
{
    if (_RefCount != nullptr && _RefCount->fetch_sub(1) == 1)
    {
        _System.GetBufferProvider().ReleaseBuffer(*this);
        _Data = nullptr;
        delete _RefCount;
        _RefCount = nullptr;
    }
}

} // namespace Loom
