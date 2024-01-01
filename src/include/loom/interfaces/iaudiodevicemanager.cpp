#pragma once

#include "loom/interfaces/iaudiodevicemanager.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioDeviceManager::IAudioDeviceManager(IAudioSystem& system)
    : IAudioSystemComponent(system)
{
}

AudioSystemComponentType IAudioDeviceManager::GetType() const
{
    return AudioSystemComponentType::DeviceManager;
}

AudioDeviceManagerStub::AudioDeviceManagerStub()
    : IAudioDeviceManager(IAudioSystem::GetStub())
{
}

AudioDeviceManagerStub& AudioDeviceManagerStub::GetInstance()
{
    static AudioDeviceManagerStub instance;
    return instance;
}

const char* AudioDeviceManagerStub::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioDeviceManager stub";
}

Result AudioDeviceManagerStub::RegisterPlaybackCallback(AudioDevicePlaybackCallback, void*)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioDeviceManagerStub::EnumerateDevices(u32&, const AudioDeviceDescription*&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioDeviceManagerStub::SelectPlaybackDevice(const AudioDeviceDescription*)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioDeviceManagerStub::SelectDefaultPlaybackDevice(AudioDeviceDescription&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioDeviceManagerStub::Start()
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioDeviceManagerStub::Stop()
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
