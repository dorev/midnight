#include "loom/audiosystem.h"
#include "loom/audiograph.h"
#include "loom/audiobufferpool.h"

namespace Loom
{

AudioSystem::AudioSystem()
    : _Graph(new AudioGraph(GetInterface()))
{
}

Result AudioSystem::Initialize()
{
    IAudioDeviceManager& deviceManager = GetDeviceManager();
    Result result = Result::Unknown;
    result = deviceManager.Initialize();
    LOOM_CHECK_RESULT(result);
    AudioDeviceDescription deviceDescription;
    result = deviceManager.SelectDefaultPlaybackDevice(deviceDescription);
    LOOM_CHECK_RESULT(result);
    AudioFormat outputFormat = deviceDescription.bufferTemplate.GetFormat();
    u32 outputBufferSize = deviceDescription.bufferTemplate.GetSize();
    _BufferProvider.reset(new AudioBufferPool(GetInterface(), outputFormat, outputBufferSize));
    result = deviceManager.RegisterPlaybackCallback(PlaybackCallback, this);
    LOOM_CHECK_RESULT(result);


    // TODO: store the device format
    //      * buffer size
    //      * sample format
    //      * channels
    //      * frame rate

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

Result AudioSystem::SetService(IAudioSubsystem* service)
{
    if (service == nullptr)
        LOOM_RETURN_RESULT(Result::Nullptr);

    switch(service->GetType())
    {
        case AudioSubsystemType::Codec:
            if (_Decoder != nullptr)
                _Decoder->Shutdown();
            _Decoder.reset(static_cast<IAudioCodec*>(service));
            return _Decoder->Initialize();

        case AudioSubsystemType::BufferProvider:
            if (_BufferProvider != nullptr)
                _BufferProvider->Shutdown();
            _BufferProvider.reset(static_cast<IAudioBufferProvider*>(service));
            return _BufferProvider->Initialize();

        case AudioSubsystemType::Resampler:
            if (_Resampler != nullptr)
                _Resampler->Shutdown();
            _Resampler.reset(static_cast<IAudioResampler*>(service));
            return _Resampler->Initialize();

        case AudioSubsystemType::ChannelRemapper:
            if (_ChannelRemapper != nullptr)
                _ChannelRemapper->Shutdown();
            _ChannelRemapper.reset(static_cast<IAudioChannelRemapper*>(service));
            return _ChannelRemapper->Initialize();

        case AudioSubsystemType::DeviceManager:
            if (_DeviceManager != nullptr)
                _DeviceManager->Shutdown();
            _DeviceManager.reset(static_cast<IAudioDeviceManager*>(service));
            return _DeviceManager->Initialize();
        default:
            LOOM_RETURN_RESULT(Result::InvalidEnumValue);
    }
}

} // namespace Loom
