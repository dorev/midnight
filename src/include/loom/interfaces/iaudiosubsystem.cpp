#pragma once

#include "loom/interfaces/iaudiosubsystem.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioSubsystem::IAudioSubsystem(IAudioSystem& system)
    : _System(system)
{
}

IAudioSubsystem::~IAudioSubsystem()
{
}

Result IAudioSubsystem::Initialize()
{
    return Result::Ok;
}

void IAudioSubsystem::Shutdown()
{
}

IAudioSystem& IAudioSubsystem::GetSystemInterface()
{
    return _System;
}

} // namespace Loom
