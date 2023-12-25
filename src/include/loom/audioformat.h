#pragma once

#include "loom/defines.h"
#include "loom/types.h"

namespace Loom
{

LOOM_DECLARE_FLAG_ENUM(AudioFormat, u32)
{
    NotSpecified = 0,

    // First 4 bits contains the channel count
    ChannelsOffset = 0,
    ChannelsMask = 0x0F,

    SampleFormatOffset = 4,
    LOOM_FLAG(Int16, SampleFormatOffset + 0),
    LOOM_FLAG(Int32, SampleFormatOffset + 1),
    LOOM_FLAG(Float32, SampleFormatOffset + 2),
    SampleFormatMask = Int16 | Int32 | Float32,

    StandardOffset = 8,
    LOOM_FLAG(DirectSpeakers, StandardOffset + 0),
    LOOM_FLAG(AudioObject, StandardOffset + 1),
    LOOM_FLAG(Ambisonic, StandardOffset + 2),
    StandardMask = DirectSpeakers | AudioObject | Ambisonic,

    SamplingRateOffset = 12,
    LOOM_FLAG(Hz44100, SamplingRateOffset + 0),
    LOOM_FLAG(Hz48000, SamplingRateOffset + 1),
    SamplingRateMask = 0x0F << SamplingRateOffset,

    Invalid = UINT32_MAX
};

u32 ParseFormatChannels(AudioFormat audioFormat)
{
    AudioFormat channels = (audioFormat & AudioFormat::ChannelsMask) << AudioFormat::ChannelsOffset;
    return static_cast<u32>(channels);
}

AudioFormat ParseSampleFormat(AudioFormat audioFormat)
{
    AudioFormat samplingRateFormat = audioFormat & AudioFormat::SampleFormatMask;
    switch(samplingRateFormat)
    {
        case AudioFormat::Int16:
        case AudioFormat::Int32:
        case AudioFormat::Float32:
            return samplingRateFormat;
        default:
            return AudioFormat::Invalid;
    }
}

u32 ParseSamplingRate(AudioFormat audioFormat)
{
    AudioFormat samplingRate = (audioFormat & AudioFormat::SamplingRateMask) << AudioFormat::SamplingRateOffset;
    switch(samplingRate)
    {
        case AudioFormat::Hz44100: return 44100;
        case AudioFormat::Hz48000: return 48000;
        default:
            return 0;
    }
}

} // namespace Loom
