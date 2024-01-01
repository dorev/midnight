#pragma once

#include "loom/audiosystemconfig.h"
#include "loom/interfaces/iaudiosystem.h"
#include "loom/interfaces/iaudiobufferprovider.h"
#include "loom/interfaces/iaudiograph.h"
#include "loom/interfaces/iaudiodevicemanager.h"
#include "loom/interfaces/iaudiocodec.h"
#include "loom/interfaces/iaudioresampler.h"
#include "loom/interfaces/iaudiochannelremapper.h"

namespace Loom
{

class AudioNode;
class AssetReaderNode;
class AudioAsset;

class AudioSystem : public IAudioSystem
{
public:
    AudioSystem();

    static void PlaybackCallback(AudioBuffer& destinationBuffer, void* userData);
    Result Initialize() override;

    shared_ptr<AudioAsset> CreateAudioAsset(const char* filePath)
    {
        // check the current device format
        LOOM_UNUSED(filePath);
        return nullptr;
    }

    Result LoadAudioAsset(AudioAsset& asset)
    {
        // check asset store if already loaded or loading
        // start loading according to 
    }


    Result UnloadAudioAsset(const shared_ptr<AudioAsset> audioAsset);
    shared_ptr<AssetReaderNode> CreateAudioSource(const shared_ptr<AudioAsset> audioAsset, const AudioNodePtr inputNode);
    Result DestroyAudioSource(const shared_ptr<AssetReaderNode> audioSource);

    const AudioSystemConfig& GetConfig() const override;
    IAudioGraph& GetGraph() const override;
    IAudioCodec& GetCodec() const override;
    IAudioDeviceManager& GetDeviceManager() const override;
    IAudioResampler& GetResampler() const override;
    IAudioChannelRemapper& GetChannelRemapper() const override;
    IAudioBufferProvider& GetBufferProvider() const override;

private:
    AudioSystemConfig _Config;
    AudioDeviceDescription _CurrentDevice;
    map<shared_ptr<AudioAsset>, set<shared_ptr<AssetReaderNode>>> _AudioSources;
    unique_ptr<IAudioGraph> _Graph;
    unique_ptr<IAudioCodec> _Decoder;
    unique_ptr<IAudioDeviceManager> _DeviceManager;
    unique_ptr<IAudioResampler> _Resampler;
    unique_ptr<IAudioChannelRemapper> _ChannelRemapper;
    unique_ptr<IAudioBufferProvider> _BufferProvider;
};

} // namespace Loom
