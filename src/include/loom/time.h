#pragma once

#include <chrono>

#include "loom/types.h"

namespace Loom
{

u64 Now()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

u64 SecondsToNanoseconds(double seconds)
{
    return static_cast<u64>(seconds / 1000000000.0);
}

}
