#include "loom/audioformat.h"

namespace Loom
{

u32 ParseChannels(AudioFormat audioFormat)
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

Result SetChannels(AudioFormat& audioFormat, u32 channels)
{
    if (channels <= 0 || channels > static_cast<u32>(AudioFormat::MaxChannels))
        return Result::InvalidParameter;

    // Clear channels bits
    audioFormat &= static_cast<AudioFormat>(~(static_cast<u32>(AudioFormat::ChannelsMask)));

    // Set channels bits
    audioFormat |= static_cast<AudioFormat>(channels << static_cast<u32>(AudioFormat::ChannelsOffset));
    return Result::Ok;
}

} // namespace Loom
