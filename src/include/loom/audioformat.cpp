#include "loom/audioformat.h"

namespace Loom
{

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
