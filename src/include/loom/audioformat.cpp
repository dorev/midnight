#include "loom/audioformat.h"

namespace Loom
{

u32 ParseChannels(AudioFormat audioFormat)
{
    u32 channels = static_cast<u32>((audioFormat & AudioFormat::ChannelsMask) >> AudioFormat::ChannelsOffset);
    if (channels == 0)
        LOOM_LOG_RESULT(Result::InvalidBufferChannelFormat);
    return channels;
}

AudioFormat ParseSampleFormat(AudioFormat audioFormat)
{
    AudioFormat sampleFormat = audioFormat & AudioFormat::SampleFormatMask;
    switch(sampleFormat)
    {
        case AudioFormat::Int16:
        case AudioFormat::Int32:
        case AudioFormat::Float32:
            return sampleFormat;
        default:
            LOOM_LOG_RESULT(Result::InvalidBufferSampleFormat);
            return AudioFormat::Invalid;
    }
}

u32 ParseFrameRate(AudioFormat audioFormat)
{
    AudioFormat frameRate = audioFormat & AudioFormat::FrameRateMask;
    switch(frameRate)
    {
        case AudioFormat::Hz44100: return 44100;
        case AudioFormat::Hz48000: return 48000;
        default:
            LOOM_LOG_RESULT(Result::InvalidBufferFrameRateFormat);
            return 0;
    }
}

Result SetChannels(AudioFormat& audioFormat, u32 channels)
{
    if (channels <= 0 || channels > static_cast<u32>(AudioFormat::ChannelsMax))
        LOOM_RETURN_RESULT(Result::InvalidParameter);

    // Clear channels bits
    audioFormat &= static_cast<AudioFormat>(~(static_cast<u32>(AudioFormat::ChannelsMask)));

    // Set channels bits
    audioFormat |= static_cast<AudioFormat>(channels << static_cast<u32>(AudioFormat::ChannelsOffset));
    return Result::Ok;
}

} // namespace Loom
