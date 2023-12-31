#include "loom/audiosystem.h"
#include "loom/interfaces/iaudiosystem.h"
#include "loom/interfaces/iaudiograph.h"
#include "loom/interfaces/iaudiocodec.h"
#include "loom/interfaces/iaudioresampler.h"
#include "loom/interfaces/iaudiobufferprovider.h"
#include "loom/interfaces/iaudiodevicemanager.h"
#include "loom/interfaces/iaudiochannelremapper.h"

namespace Loom
{

IAudioSystem::~IAudioSystem()
{
}

IAudioSystem& IAudioSystem::GetStub()
{
    return AudioSystemStub::GetInstance().GetInterface();
}

IAudioSystem& IAudioSystem::GetInterface()
{
    return *this;
}

IAudioSystem& AudioSystemStub::GetInstance()
{
    static AudioSystemStub instance;
    return instance.GetInterface();
}

const AudioSystemConfig& AudioSystemStub::GetConfig() const
{
    static AudioSystemConfig dummyConfig;
    LOOM_LOG_RESULT(Result::CallingStub);
    return dummyConfig;
}

IAudioGraph& AudioSystemStub::GetGraph()
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return AudioGraphStub::GetInstance();
}

IAudioCodec& AudioSystemStub::GetCodec() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return AudioCodecStub::GetInstance();
}

IAudioDeviceManager& AudioSystemStub::GetDeviceManager() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return AudioDeviceManagerStub::GetInstance();
}

IAudioResampler& AudioSystemStub::GetResampler() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return AudioResamplerStub::GetInstance();
}

IAudioChannelRemapper& AudioSystemStub::GetChannelRemapper() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return AudioChannelRemapperStub::GetInstance();
}

IAudioBufferProvider& AudioSystemStub::GetBufferProvider() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return AudioBufferProviderStub::GetInstance();
}

} // namespace Loom
