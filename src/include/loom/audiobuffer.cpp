#include "loom/audiobuffer.h"
#include "loom/interfaces/iaudiobufferprovider.h"

namespace Loom
{

AudioBuffer::AudioBuffer(AudioFormat format, IAudioBufferProvider* pool, u8* data, u32 capacity)
    : _Data(data)
    , _Size(0)
    , _Format(format)
    , _Pool(pool)
    , _Capacity(capacity)
    , _RefCount(nullptr)
{
    if (_Pool != nullptr)
        _RefCount = new atomic<u32>(1);
}

AudioBuffer::~AudioBuffer()
{
    DecrementRefCount();
}

AudioBuffer::AudioBuffer(const AudioBuffer& other)
    : _Data(other._Data)
    , _Size(other._Size)
    , _Format(other._Format)
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
        _Data = other._Data;
        _Size = other._Size;
        _Format = other._Format;
        _Pool = other._Pool;
        _Capacity = other._Capacity;
        _RefCount = other._RefCount;
        if (_Pool != nullptr && _RefCount != nullptr)
            _RefCount->fetch_add(1);
    }
    return *this;
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
    AudioFormat sampleFormat = other._Format & AudioFormat::SampleFormatMask;
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

Result AudioBuffer::GetFrameCount(u32& frameCount) const
{
    Result result = GetSampleCount(frameCount);
    if (!Ok(result))
    {
        frameCount = 0;
        LOOM_RETURN_RESULT(result);
    }
    frameCount /= GetChannels();
    return Result::Ok;
}

bool AudioBuffer::FormatMatches(const AudioBuffer& other) const
{
    return _Format == other._Format;
}

u32 AudioBuffer::GetChannels() const
{
    return ParseChannels(_Format);
}

u32 AudioBuffer::GetSampleRate() const
{
    return ParseFrameRate(_Format);
}

AudioFormat AudioBuffer::GetSampleFormat() const
{
    return ParseSampleFormat(_Format);
}

u32 AudioBuffer::GetSampleSize() const
{
    switch (GetSampleFormat())
    {
        case AudioFormat::Int16: return sizeof(s16);
        case AudioFormat::Int32: return sizeof(s32);
        case AudioFormat::Float32: return sizeof(float);
        default:
            LOOM_LOG_RESULT(Result::InvalidBufferSampleFormat);
            return 0;
    }
}

void AudioBuffer::DecrementRefCount()
{
    if (_Pool != nullptr
        && _RefCount != nullptr
        && _RefCount->fetch_sub(1) == 1)
    {
        _Pool->ReleaseBuffer(*this);
        _Pool = nullptr;
        _Data = nullptr;
        delete _RefCount;
        _RefCount = nullptr;
    }
}

} // namespace Loom
