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

Result AudioBuffer::CopyDataFrom(const AudioBuffer& other) const
{
    if (_Data == nullptr || other._Data == nullptr)
        LOOM_RETURN_RESULT(Result::NoData);
    if (_Capacity < other._Size)
        LOOM_RETURN_RESULT(Result::BufferCapacityMismatch);
    memcpy(_Data, other._Data, other._Size);
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

Result AudioBuffer::GetSampleCount(u32& sampleCount) const
{
    if (_Data != nullptr)
    {
        switch (_Format & AudioFormat::SampleFormatMask)
        {
            case AudioFormat::Int16:
                sampleCount = _Size / sizeof (s16);
                return Result::Ok;
            case AudioFormat::Int32:
            case AudioFormat::Float32:
                sampleCount = _Size / sizeof (s32);
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
    return ParseSamplingRate(_Format);
}

AudioFormat AudioBuffer::GetSampleFormat() const
{
    return ParseSampleFormat(_Format);
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
