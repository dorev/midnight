#include "loom/interfaces/iaudiochannelremapper.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioChannelRemapper::IAudioChannelRemapper(IAudioSystem& system)
    : IAudioSubsystem(system)
{
}

AudioSubsystemType IAudioChannelRemapper::GetType() const
{
    return AudioSubsystemType::ChannelRemapper;
}

IAudioChannelRemapper& IAudioChannelRemapper::GetStub()
{
    static IAudioChannelRemapper instance(IAudioSystem::GetStub());
    return instance;
}

IAudioChannelRemapper& IAudioChannelRemapper::GetInterface()
{
    return *this;
}

const char* IAudioChannelRemapper::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioChannelRemapper stub";
}

Result IAudioChannelRemapper::Remap(const AudioBuffer&, AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
