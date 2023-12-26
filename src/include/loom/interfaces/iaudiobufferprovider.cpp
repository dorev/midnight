#include "loom/interfaces/iaudiobufferprovider.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioBufferProvider::IAudioBufferProvider(IAudioSystem& system)
    : IAudioSubsystem(system)
{
}

AudioSubsystemType IAudioBufferProvider::GetType() const
{
    return AudioSubsystemType::BufferProvider;
}

AudioBufferProviderStub::AudioBufferProviderStub()
    : IAudioBufferProvider(IAudioSystem::GetStub())
{
}

AudioBufferProviderStub& AudioBufferProviderStub::GetInstance()
{
    static AudioBufferProviderStub instance;
    return instance;
}

const char* AudioBufferProviderStub::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioBufferProvider stub";
}

Result AudioBufferProviderStub::AllocateBuffer(AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result AudioBufferProviderStub::ReleaseBuffer(AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
