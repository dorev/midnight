#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

namespace Loom
{

enum class SampleFormat
{
    Invalid,
    Int16,
    Int32,
    Float32
};

class AudioFormat
{
public:
    u32 channels;
    u32 frameRate;
    SampleFormat sampleFormat;

    AudioFormat();
    AudioFormat(const AudioFormat& other);
    AudioFormat& operator=(const AudioFormat& other);
    bool operator==(const AudioFormat& other) const;
};

template <class T>
constexpr SampleFormat TypeToSampleFormat()
{
    if constexpr (std::is_same_v<T, s16>)
        return SampleFormat::Int16;
    else if constexpr (std::is_same_v<T, s32>)
        return SampleFormat::Int32;
    else if constexpr (std::is_same_v<T, float>)
        return SampleFormat::Float32;
    else
        return SampleFormat::Invalid;
}

template<SampleFormat F> struct SampleFormatToTypeImpl;
template<> struct SampleFormatToTypeImpl<SampleFormat::Int16> { using type = s16; };
template<> struct SampleFormatToTypeImpl<SampleFormat::Int32> { using type = s32; };
template<> struct SampleFormatToTypeImpl<SampleFormat::Float32> { using type = float; };
template <SampleFormat F> using SampleFormatToType = typename SampleFormatToTypeImpl<F>::type;


} // namespace Loom
