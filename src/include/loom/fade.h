#pragma once

#include "loom/types.h"
#include "loom/time.h"

namespace Loom
{

using FadeFunction = void(*)(float&, u64, u64);

void LinearFade(float& gain, float targetGain, u64 startTime, u64 endTime)
{
    u64 now = Now();
    if (now >= endTime)
    {
        gain = targetGain;
        return;
    }
    else if (now <= startTime)
    {
        return;
    }
    float fadeRange = static_cast<float>(endTime - startTime);
    float fadeProgress = static_cast<float>(now - startTime);
    float fadeRatio = fadeProgress / fadeRange;
    float gainRange = targetGain - gain;
    gain += gainRange * fadeRatio;

};

void FadeIn(float& gain, u64 startTime, u64 endTime)
{
    LinearFade(gain, 1.0f, startTime, endTime);
};

void FadeOut(float& gain, u64 startTime, u64 endTime)
{
    LinearFade(gain, 0.0f, startTime, endTime);
};

}
