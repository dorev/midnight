#pragma once

#include <cmath>

#include "types.h"

namespace Midnight
{
    using Decibel = F32;

    F32 LinearToDecibel(F32 linearVolume)
    {
        if (linearVolume <= 0.0f)
        {
            return -80.0f;
        }
        return 20.0f * std::log10(linearVolume);
    }

    enum class Error : U32
    {
        Ok = 0,
        Nullptr,
        InvalidPosition,
        UnsupportedFormat,
        InvalidFile,
        OutputDeviceDisconnected,
        NotYetImplemented,
        Unknown = U32MAX
    };

    enum class AudioDeviceType
    {
        Playback,
        Recording
    };

    struct AudioDeviceDescription
    {
        String name;
        U32 channels;
        U32 sampleRate;
        Bool defaultDevice;
        AudioDeviceType deviceType;
    };

    struct AudioBuffer
    {
        SharedPtr<F32[]> samplesData;
        SharedPtr<U8[]> metadata;
        U32 channels;
        U32 sampleRate;
        U32 samples;
    };

    class AudioAsset
    {
        String _Name;
        Decibel _GlobalVolumeAdjustment;
        SharedPtr<AudioBuffer> _AudioBuffer;
    };

    using AudioPlaybackCallback = void(*)(U8* destination, U32 channels, U32 frames, Error error);

    class IAudioDeviceManager
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
        virtual Error RegisterPlaybackCallback(AudioPlaybackCallback callback) = 0;
        virtual Error EnumerateDevices(U32& deviceCount, const AudioDeviceDescription* devices) = 0;
        virtual Error SelectPlaybackDevice(const AudioDeviceDescription& device) = 0;
        virtual Error Start() = 0;
        virtual Error Stop() = 0;
    };

    class IAudioDecoder
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
        virtual Error DecodeFile(const String& filePath, AudioBuffer& audioData) = 0;
    };

    class IAudioResampler
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
        virtual Error Resample(const AudioBuffer& input, AudioBuffer& output) = 0;
    };

    class IAudioChannelRemapper
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
        virtual Error Remap(const AudioBuffer& input, AudioBuffer& output) = 0;
    };

    class MixerNode
    {
    private:
        friend class Mixer;

    public:
        Error ConnectPrevious(SharedPtr<MixerNode> node);
        Error ConnectNext(SharedPtr<MixerNode> node);
        Bool ReadyToProcess() const;

        enum MixerNodeState
        {
            NeedsProcessing,
            Processing,
            DoneProcessing
        };

        MixerNodeState GetState() const { return _State; }

    protected:
        virtual Error Process(const AudioBuffer& input, AudioBuffer& output) = 0;

    private:
        SharedPtr<MixerNode> _Previous;
        SharedPtr<MixerNode> _Next;
        MixerNodeState _State;
    };

    enum class AudioParameterType
    {
        Unsigned,
        Signed,
        Float,
        Boolean
    };

    class AudioParameter
    {
    private:
        AudioParameterType _Type;
        Variant<U32, S32, F32, Bool> _Value;
        String _Name;
    };

    class MixerEffectNodeBase : public MixerNode
    {
    public:
        Error Process(const AudioBuffer& input, AudioBuffer& output);

    protected:
        Vector<SharedPtr<AudioParameter>> _Parameters;
    };

    class MixerOutputNode : public MixerNode
    {
    public:
        Error Process(const AudioBuffer& input, AudioBuffer& output);

    private:
        using MixerNode::ConnectNext;
    };

    class Mixer
    {
    public:
        template <class MixerNodeType, class... Args>
        EnableIf<IsDerivedFrom<MixerNode, MixerNodeType>, SharedPtr<MixerNodeType>> CreateNode(Args... args);

    private:
        Vector<SharedPtr<MixerNode>> _Nodes;
    };

    class AudioSource
    {
    public:
        Error Play(F32 fadeIn = 0.0f);
        Error Pause(F32 fadeOut = 0.05f);
        Error Stop(F32 fadeOut = 0.05f);
        Error Seek(U32 position);

    private:
        U32 _Id;
        U32 _Position;
        SharedPtr<const AudioBuffer> _AudioBuffer;
        SharedPtr<MixerNode> _MixerInput;
    };

    struct AudioManagerServices
    {
        SharedPtr<IAudioDeviceManager> deviceManager;
        SharedPtr<IAudioDecoder> decoder;
        SharedPtr<IAudioResampler> resampler;
        SharedPtr<IAudioChannelRemapper> remapper;
    };

    class AudioManager
    {
    public:
        AudioManager(const AudioManagerServices& services)
            : _Services(services)
        {
        }

        Error Initialize();
        SharedPtr<AudioAsset> CreateAudioAsset(const String& filePath);
        SharedPtr<AudioSource> CreateAudioSource(const SharedPtr<AudioAsset>& audioAsset);

    private:
        AudioManagerServices _Services;
    };
}