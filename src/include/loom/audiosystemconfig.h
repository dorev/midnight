#pragma once

#include "loom/defines.h"
#include "loom/types.h"

namespace Loom
{

struct AudioSystemConfig
{
    AudioSystemConfig()
        : maxAudibleSources(0)
    {
    }

    u32 maxAudibleSources;
};

} // namespace Loom
