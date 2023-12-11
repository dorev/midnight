#pragma once

#include <cmath>

#include "types.h"

namespace Midnight
{
    enum Error : U32
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

    using Decibel = F32;

    Decibel LinearToDecibel(F32 linearVolume)
    {
        if (linearVolume <= 0.0f)
        {
            return -80.0f;
        }
        return 20.0f * std::log10(linearVolume);
    }

    F32 DecibelToLinear(F32 decibel)
    {
        if (decibel <= -80.0f)
        {
            return 0.0f;
        }
        return std::pow(10.0f, decibel / 20.0f);
    }

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

    enum class AudioParameterType
    {
        Unsigned,
        Signed,
        Float,
        Boolean,
        Transform
    };

    struct AudioParameter
    {
        String name;
        AudioParameterType type;
        Variant<U32, S32, F32, Bool, Transform> value;
    };

    enum class AudioNodeState
    {
        Dirty,
        Processing,
        Clean
    };

    class AudioNode
    {
    private:
        friend class AudioGraph;

    public:
        Error ConnectPrevious(SharedPtr<AudioNode>& node)
        {
            return Error::NotYetImplemented;
        }

        Error ConnectNext(SharedPtr<AudioNode>& node)
        {
            return Error::NotYetImplemented;
        }

        Bool ReadyToProcess() const
        {
            return true;
        }

        AudioNodeState GetState() const
        {
            return _State;
        }

    protected:
        virtual Error Process(const AudioBuffer& input, AudioBuffer& output) = 0;

    protected:
        Vector<SharedPtr<AudioParameter>> _Parameters;
        Vector<SharedPtr<AudioNode>> _InputNodes;
        Vector<SharedPtr<AudioNode>> _OutputNodes;
        Atomic<AudioNodeState> _State;
    };

    class AudioNodeConnectionResult
    {
    public:
        SharedPtr<AudioNode> node;
        Error error;

        AudioNodeConnectionResult(SharedPtr<AudioNode> node, Error error)
            : node(node), error(error)
        {
        }

        AudioNodeConnectionResult& operator>>(SharedPtr<AudioNode>& right)
        {
            if (error == Error::Ok)
            {
                error = node->ConnectNext(right);
                if (error == Error::Ok)
                {
                    node = right;
                }
            }
            return *this;
        }
    };

    template <class T>
    AudioNodeConnectionResult operator>>(AudioNodeConnectionResult& result, const SharedPtr<T>& right)
    {
        static_assert(IsDerivedFrom<AudioNode, T>, "Right operand must be derived from AudioNode");

        if (result.error == Error::Ok)
        {
            result.error = result.node->ConnectNext(std::static_pointer_cast<AudioNode>(right));
            if (result.error == Error::Ok)
            {
                result.node = right;
            }
        }
        return result;
    }

    template <typename T, typename U>
    AudioNodeConnectionResult operator>>(const SharedPtr<T>& left, const SharedPtr<U>& right)
    {
        static_assert(IsDerivedFrom<AudioNode, T>, "Left operand must be derived from AudioNode");
        static_assert(IsDerivedFrom<AudioNode, U>, "Right operand must be derived from AudioNode");

        Error error = left->ConnectNext(std::static_pointer_cast<AudioNode>(right));
        return AudioNodeConnectionResult(std::static_pointer_cast<AudioNode>(right), error);
    }

    class AudioGraph
    {
    public:
        template <class AudioNodeType, class... Args>
        EnableIf<IsDerivedFrom<AudioNode, AudioNodeType>, SharedPtr<AudioNodeType>> CreateNode(Args... args);

    private:
        Vector<SharedPtr<AudioNode>> _Nodes;
    };

    class AudioSource
    {
    public:
        Error Play(F32 fadeIn = 0.0f);
        Error Pause(F32 fadeOut = 0.05f);
        Error Stop(F32 fadeOut = 0.05f);
        Error Seek(U32 position);
        U32 samplePosition;
        Bool loop;

    private:
        U32 _Id;
        U32 _Priority;
        SharedPtr<AudioAsset> _AudioAsset;
        SharedPtr<AudioNode> _InputNode;
    };

    class IAudioService
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
    };

    using AudioPlaybackCallback = void(*)(U8* destination, U32 channels, U32 frames, Error error);

    class IAudioDeviceManager : public IAudioService
    {
    public:
        virtual Error RegisterPlaybackCallback(AudioPlaybackCallback callback) = 0;
        virtual Error EnumerateDevices(U32& deviceCount, const AudioDeviceDescription* devices) = 0;
        virtual Error SelectPlaybackDevice(const AudioDeviceDescription& device) = 0;
        virtual Error Start() = 0;
        virtual Error Stop() = 0;
    };

    class IAudioDecoder : public IAudioService
    {
    public:
        virtual Error DecodeFile(const String& filePath, AudioBuffer& destination) = 0;
    };

    class IAudioResampler : public IAudioService
    {
    public:
        virtual Error Resample(const AudioBuffer& source, AudioBuffer& destination) = 0;
    };

    class IAudioChannelRemapper : public IAudioService
    {
    public:
        virtual Error Remap(const AudioBuffer& source, AudioBuffer& destination) = 0;
    };

    struct AudioManagerServices
    {
    public:
        SharedPtr<IAudioDeviceManager> deviceManager;
        SharedPtr<IAudioDecoder> decoder;
        SharedPtr<IAudioResampler> resampler;
        SharedPtr<IAudioChannelRemapper> remapper;
    };

    struct AudioManagerConfig
    {
        U32 maxAudibleSources;
    };

    class AudioManager
    {
    public:
        Error Initialize(const AudioManagerServices& services, const AudioManagerConfig& config);
        Error Shutdown();
        SharedPtr<AudioAsset> LoadAudioAsset(const String& filePath);
        Error UnloadAudioAsset(const SharedPtr<AudioAsset>& audioAsset);
        SharedPtr<AudioSource> CreateAudioSource(const SharedPtr<AudioAsset>& audioAsset, const SharedPtr<AudioNode>& inputNode);
        Error DestroyAudioSource(const SharedPtr<AudioSource>& audioSource);

    private:
        AudioManagerServices _Services;
        Map<SharedPtr<AudioAsset>, Vector<SharedPtr<AudioSource>>> _AudioSources;
        AudioGraph _AudioGraph;
    };
}