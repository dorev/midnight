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
    static IAudioSystem instance;
    return instance;
}

IAudioSystem& IAudioSystem::GetInterface()
{
    return *this;
}

const AudioSystemConfig& IAudioSystem::GetConfig() const
{
    static const AudioSystemConfig dummyConfig;
    LOOM_LOG_RESULT(Result::CallingStub);
    return dummyConfig;
}

IAudioGraph& IAudioSystem::GetGraph()
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return IAudioGraph::GetStub();
}

IAudioCodec& IAudioSystem::GetCodec() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return IAudioCodec::GetStub();
}

IAudioDeviceManager& IAudioSystem::GetDeviceManager() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return IAudioDeviceManager::GetStub();
}

IAudioResampler& IAudioSystem::GetResampler() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return IAudioResampler::GetStub();
}

IAudioChannelRemapper& IAudioSystem::GetChannelRemapper() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return IAudioChannelRemapper::GetStub();
}

IAudioBufferProvider& IAudioSystem::GetBufferProvider() const
{
    LOOM_LOG_RESULT(Result::CallingStub);
    return IAudioBufferProvider::GetStub();
}

} // namespace Loom
