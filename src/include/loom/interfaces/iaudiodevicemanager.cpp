#pragma once

#include "loom/interfaces/iaudiodevicemanager.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioDeviceManager::IAudioDeviceManager(IAudioSystem& system)
    : IAudioSubsystem(system)
{
}
IAudioDeviceManager& IAudioDeviceManager::GetStub()
{
    static IAudioDeviceManager instance(IAudioSystem::GetStub());
    return instance;
}

IAudioDeviceManager& IAudioDeviceManager::GetInterface()
{
    return *this;
}

AudioSubsystemType IAudioDeviceManager::GetType() const
{
    return AudioSubsystemType::DeviceManager;
}

const char* IAudioDeviceManager::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioDeviceManager stub";
}

Result IAudioDeviceManager::RegisterPlaybackCallback(AudioDevicePlaybackCallback, void*)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result IAudioDeviceManager::EnumerateDevices(u32&, const AudioDeviceDescription*&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result IAudioDeviceManager::SelectPlaybackDevice(const AudioDeviceDescription*)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result IAudioDeviceManager::SelectDefaultPlaybackDevice()
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result IAudioDeviceManager::Start()
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result IAudioDeviceManager::Stop()
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
