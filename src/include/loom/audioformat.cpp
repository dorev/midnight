#pragma once

#include "loom/audioformat.h"

namespace Loom
{

AudioFormat::AudioFormat()
    : channels(0)
    , frameRate(0)
    , sampleFormat(SampleFormat::Invalid)
{
}

AudioFormat::AudioFormat(const AudioFormat& other)
    : channels(other.channels)
    , frameRate(other.frameRate)
    , sampleFormat(other.sampleFormat)
{
}

AudioFormat& AudioFormat::operator=(const AudioFormat& other)
{
    if (&other != this)
    {
        channels = other.channels;
        frameRate = other.frameRate;
        sampleFormat = other.sampleFormat;
    }
    return *this;
}

bool AudioFormat::operator==(const AudioFormat& other) const
{
    return channels == other.channels
        && frameRate == other.frameRate
        && sampleFormat == other.sampleFormat;
}

} // namespace Loom
