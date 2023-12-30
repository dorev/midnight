#pragma once

#include "loom/defines.h"
#include "loom/types.h"
#include "loom/result.h"

#include "loom/audiosystemconfig.h"
#include "loom/interfaces/iaudiosystem.h"
#include "loom/interfaces/iaudiobufferprovider.h"
#include "loom/interfaces/iaudiodevicemanager.h"
#include "loom/interfaces/iaudiocodec.h"
#include "loom/interfaces/iaudioresampler.h"
#include "loom/interfaces/iaudiochannelremapper.h"

namespace Loom
{

class AudioNode;
class AudioSource;
class AudioAsset;

class AudioSystem : public IAudioSystem
{
public:
    AudioSystem();

    Result Initialize();

    static void PlaybackCallback(AudioBuffer& destinationBuffer, void* userData);

    Result Shutdown();
    shared_ptr<AudioAsset> LoadAudioAsset(const char* filePath);
    Result UnloadAudioAsset(const shared_ptr<AudioAsset> audioAsset);
    shared_ptr<AudioSource> CreateAudioSource(const shared_ptr<AudioAsset> audioAsset, const AudioNodePtr inputNode);
    Result DestroyAudioSource(const shared_ptr<AudioSource> audioSource);

    const AudioSystemConfig& GetConfig() const override
    {
        return _Config;
    }

    IAudioGraph& GetGraphInterface()
    {
        if (_Graph == nullptr)
            return AudioGraphStub::GetInstance();
        return *_Graph;
    }

    IAudioCodec& GetCodecInterface() const override
    {
        if (_Decoder == nullptr)
            return AudioCodecStub::GetInstance();
        return *_Decoder;
    }

    IAudioDeviceManager& GetDeviceManagerInterface() const override
    {
        if (_DeviceManager == nullptr)
            return AudioDeviceManagerStub::GetInstance();
        return *_DeviceManager;
    }

    IAudioResampler& GetResamplerInterface() const override
    {
        if (_Resampler == nullptr)
            return AudioResamplerStub::GetInstance();
        return *_Resampler;
    }

    IAudioChannelRemapper& GetChannelRemapperInterface() const override
    {
        if (_ChannelRemapper == nullptr)
            return AudioChannelRemapperStub::GetInstance();
        return *_ChannelRemapper;
    }

    IAudioBufferProvider& GetBufferProviderInterface() const override
    {
        if (_BufferProvider == nullptr)
            return AudioBufferProviderStub::GetInstance();
        return *_BufferProvider;
    }

    Result SetService(IAudioSubsystem* service);

private:
    AudioSystemConfig _Config;
    map<shared_ptr<AudioAsset>, set<shared_ptr<AudioSource>>> _AudioSources;
    unique_ptr<IAudioGraph> _Graph;
    unique_ptr<IAudioCodec> _Decoder;
    unique_ptr<IAudioDeviceManager> _DeviceManager;
    unique_ptr<IAudioResampler> _Resampler;
    unique_ptr<IAudioChannelRemapper> _ChannelRemapper;
    unique_ptr<IAudioBufferProvider> _BufferProvider;
};

} // namespace Loom
