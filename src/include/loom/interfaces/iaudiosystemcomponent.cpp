#pragma once

#include "loom/interfaces/iaudiosystemcomponent.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioSystemComponent::IAudioSystemComponent(IAudioSystem& system)
    : _System(system)
{
}

IAudioSystemComponent::~IAudioSystemComponent()
{
}

Result IAudioSystemComponent::Initialize()
{
    return Result::Ok;
}

Result IAudioSystemComponent::Update()
{
    return Result::Ok;
}

void IAudioSystemComponent::Shutdown()
{
}

IAudioSystem& IAudioSystemComponent::GetSystemInterface()
{
    return _System;
}

} // namespace Loom
