#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

namespace Loom
{

LOOM_DECLARE_FLAG_ENUM(AudioFormat, u32)
{
    NotSpecified = 0,

    // First 4 bits contains the channel count
    ChannelsOffset = 0,
    ChannelsMax = 0xFF,
    ChannelsMask = ChannelsMax << ChannelsOffset,

    SampleFormatOffset = 8,
    Int16 = 1 << SampleFormatOffset,
    Int32 = 2 << SampleFormatOffset,
    Float32 = 3 << SampleFormatOffset,
    SampleFormatMax = 0x0F,
    SampleFormatMask = SampleFormatMax << SampleFormatOffset,

    FrameRateOffset = 16,
    Hz44100 = 1 << FrameRateOffset,
    Hz48000 = 2 << FrameRateOffset,
    FrameRateMax = 0x0F,
    FrameRateMask = FrameRateMax << FrameRateOffset,

    Invalid = UINT32_MAX
};

u32 ParseChannels(AudioFormat audioFormat);
AudioFormat ParseSampleFormat(AudioFormat audioFormat);
u32 ParseFrameRate(AudioFormat audioFormat);
Result SetChannels(AudioFormat& audioFormat, u32 channels);

} // namespace Loom
