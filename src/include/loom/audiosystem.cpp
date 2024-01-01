#include "loom/audiosystem.h"
#include "loom/audiograph.h"
#include "loom/audiobufferpool.h"

namespace Loom
{

AudioSystem::AudioSystem()
    : _Graph(new AudioGraph(GetInterface()))
    //, _DeviceManager(new MiniAudioDeviceManager(GetInterface()))
{
    Result result = GetGraph().Initialize();
    if (!Ok(result))
        LOOM_LOG_RESULT(result);
}

Result AudioSystem::Initialize()
{
    // Initialize device manager to get hardware audio format and buffer size
    IAudioDeviceManager& deviceManager = GetDeviceManager();
    Result result = Result::Ok;
    result = deviceManager.Initialize();
    LOOM_CHECK_RESULT(result);
    result = deviceManager.SelectDefaultPlaybackDevice(_CurrentDevice);
    LOOM_CHECK_RESULT(result);

    // Setup buffer provider
    _BufferProvider.reset(new AudioBufferPool(GetInterface(), _CurrentDevice.audioFormat, _CurrentDevice.bufferSize));
    result = deviceManager.RegisterPlaybackCallback(PlaybackCallback, this);
    LOOM_CHECK_RESULT(result);



    return result;
}

void AudioSystem::PlaybackCallback(AudioBuffer& destinationBuffer, void* userData)
{
    IAudioSystem* system = reinterpret_cast<IAudioSystem*>(userData);
    if (system == nullptr)
    {
        LOOM_LOG_ERROR("IAudioSystem not available in PlaybackCallback.");
        return;
    }
    IAudioGraph& graph = system->GetGraph();
    graph.Execute(destinationBuffer);
}

const AudioSystemConfig& AudioSystem::GetConfig() const
{
    return _Config;
}

IAudioGraph& AudioSystem::GetGraph() const
{
    if (_Graph == nullptr)
        return AudioGraphStub::GetInstance();
    return *_Graph;
}

IAudioCodec& AudioSystem::GetCodec() const
{
    if (_Decoder == nullptr)
        return AudioCodecStub::GetInstance();
    return *_Decoder;
}

IAudioDeviceManager& AudioSystem::GetDeviceManager() const
{
    if (_DeviceManager == nullptr)
        return AudioDeviceManagerStub::GetInstance();
    return *_DeviceManager;
}

IAudioResampler& AudioSystem::GetResampler() const
{
    if (_Resampler == nullptr)
        return AudioResamplerStub::GetInstance();
    return *_Resampler;
}

IAudioChannelRemapper& AudioSystem::GetChannelRemapper() const
{
    if (_ChannelRemapper == nullptr)
        return AudioChannelRemapperStub::GetInstance();
    return *_ChannelRemapper;
}

IAudioBufferProvider& AudioSystem::GetBufferProvider() const
{
    if (_BufferProvider == nullptr)
        return AudioBufferProviderStub::GetInstance();
    return *_BufferProvider;
}

} // namespace Loom
