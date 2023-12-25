#pragma once

#include "loom/types.h"

namespace Loom
{

float LinearToDB(float volume)
{
    if (volume <= 0.0f)
        return -80.0f;
    return 20.0f * std::log10(volume);
}

float DBToLinear(float dB)
{
    if (dB <= -80.0f)
        return 0.0f;
    return std::pow(10.0f, dB / 20.0f);
}

}
