#include "loom/interfaces/iaudiochannelremapper.h"
#include "loom/interfaces/iaudiosystem.h"

namespace Loom
{

IAudioChannelRemapper::IAudioChannelRemapper(IAudioSystem& system)
    : IAudioSystemComponent(system)
{
}

AudioSystemComponentType IAudioChannelRemapper::GetType() const
{
    return AudioSystemComponentType::ChannelRemapper;
}

AudioChannelRemapperStub::AudioChannelRemapperStub()
    : IAudioChannelRemapper(IAudioSystem::GetStub())
{
}

AudioChannelRemapperStub& AudioChannelRemapperStub::GetInstance()
{
    static AudioChannelRemapperStub instance;
    return instance;
}

const char* AudioChannelRemapperStub::GetName() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return "IAudioChannelRemapper stub";
}

Result AudioChannelRemapperStub::Remap(const AudioBuffer&, AudioBuffer&)
{
    LOOM_RETURN_RESULT(Result::CallingStub);
}

} // namespace Loom
