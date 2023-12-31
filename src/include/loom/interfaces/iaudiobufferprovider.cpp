#include "loom/interfaces/iaudiobufferprovider.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioBufferProvider::IAudioBufferProvider(IAudioSystem& system)
    : IAudioSubsystem(system)
{
}

IAudioBufferProvider& IAudioBufferProvider::GetStub()
{
    static IAudioBufferProvider instance(IAudioSystem::GetStub());
    return instance;
}

IAudioBufferProvider& IAudioBufferProvider::GetInterface()
{
    return *this;
}

AudioSubsystemType IAudioBufferProvider::GetType() const
{
    return AudioSubsystemType::BufferProvider;
}

const char* IAudioBufferProvider::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioBufferProvider stub";
}

Result IAudioBufferProvider::AllocateBuffer(AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

Result IAudioBufferProvider::ReleaseBuffer(AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
