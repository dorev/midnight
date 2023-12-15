#pragma once

#include <cmath>

#include "types.h"

namespace Midnight
{
    // This enum is the type used for any result or error code
    enum Error : U32
    {
        Ok = 0,
        Nullptr,
        InvalidPosition,
        UnsupportedFormat,
        InvalidFile,
        OutputDeviceDisconnected,
        WrongParameterType,
        CannotFind,
        NotYetImplemented,
        UnexpectedState,
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

    // This struct encapsulates an audio data pointer and the information related
    // to its structure (channels, sample rate, etc.)
    struct AudioBuffer
    {
        U8* samplesData;
        U32 channels;
        U32 sampleRate;
        U32 samples;
    };

    // An AudioAsset refers to the original sound. It contains its data, and
    // could eventually also host metadata and 'global' parameters affecting
    // any process refering to that sound
    class AudioAsset
    {
    public:
        Decibel dB;

    public:
        AudioAsset(const String& name, AudioBuffer* buffer)
            : _Name(name)
            , _Buffer(buffer)
            , dB(0.f)
        {
        }

        const String& GetName() const
        {
            return _Name;
        }

    private:
        String _Name;
        AudioBuffer* _Buffer;
    };

    // Supported node parameter types
    enum class AudioNodeParameterType
    {
        Unsigned32,
        Signed32,
        Float32,
        Boolean,
        Point,
        Vector3,
        Quaternion,
        Transform,
    };

    // Object encapsulating the value of an audio node parameter
    class AudioNodeParameter
    {
    public:
        AudioNodeParameter(const String& name, AudioNodeParameterType type)
            : _Name(name)
            , _Type(type)
            , _Value(0)
        {
        }

        template <class T>
        Error SetValue(const T& value)
        {
            if (_Value.Contains<T>())
            {
                _Value = value;
                return Ok;
            }
            else
            {
                return WrongParameterType;
            }
        }

        template <class T>
        Error GetValue(T& value) const
        {
            if (_Value.Contains<T>())
            {
                value = _Value.Get<T>();
                return Ok;
            }
            else
            {
                return WrongParameterType;
            }
        }

        const String& GetName() const
        {
            return _Name;
        }

        AudioNodeParameterType GetType() const
        {
            return _Type;
        }

    private:
        String _Name;
        AudioNodeParameterType _Type;
        Variant<U32, S32, F32, Bool, Point, Vector3, Quaternion, Transform> _Value;
    };

    // Possible states of an audio node
    enum class AudioNodeState
    {
        WaitingForDependencies,
        PendingProcessing,
        Processing,
        ProcessingDone,
    };

    // Base class of processing nodes of the audio graph
    class AudioNode
    {
    protected:
        // Nodes can only be constructed by AudioGraph
        friend class AudioGraph;
        AudioNode()
        {
        }

    public:
        Mutex mutex;

        virtual Error Initialize()
        {
            return Ok;
        }

        virtual Error Shutdown()
        {
            return Ok;
        }

        virtual Error Reset()
        {
            return Ok;
        }

    public:
        Error ConnectPrevious(AudioNode* node)
        {
            ScopedMutex lock(mutex);
            _InputNodes.Insert(node);
            return Ok;
        }

        Error ConnectNext(AudioNode* node)
        {
            ScopedMutex lock(mutex);
            _OutputNodes.Insert(node);
            return Ok;
        }

        Error DisconnectNode(AudioNode* node)
        {
            ScopedMutex lock(mutex);
            _InputNodes.Remove(node);
            _OutputNodes.Remove(node);

            return Ok;
        }

        AudioNodeState GetState() const
        {
            return _State;
        }

    protected:
        virtual Error Process(AudioBuffer* input, AudioBuffer* output)
        {
            ScopedMutex lock(mutex);
            if (_State != AudioNodeState::PendingProcessing)
            {
                return UnexpectedState;
            }
            output = input;
            return Ok;
        }

    protected:
        Set<AudioNode*> _InputNodes;
        Set<AudioNode*> _OutputNodes;
        AudioNodeState _State;
    };

    // Class storing all the audio nodes of the engine
    class AudioGraph
    {
    public:
        template <class T, class... Args>
        T* CreateNode(Args... args)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");

            T* node = new T(std::forward<Args>(args)...);
            if (_Nodes.Insert(static_cast<AudioNode*>(node)))
            {
                return node;
            }
            else
            {
                return nullptr;
            }
        }

        template <class T>
        Error RemoveNode(const T* node)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");

            if (node == nullptr)
            {
                return Nullptr;
            }
            
            if (_Nodes.Remove(SharedPtrCast<AudioNode>(node)))
            {
                return Ok;
            }
            else
            {
                return CannotFind;
            }
        }

        template <class T, class... NodeTypes>
        Error ChainNodes(T* node, NodeTypes*... followingNodes)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");
            static_assert((IsDerivedFrom<AudioNode, NodeTypes> && ...), "All types must be derived from AudioNode");

            if (node == nullptr || ((followingNodes == nullptr) || ...))
            {
                return Nullptr;
            }

            return ChainNodesImpl(node, followingNodes...);
        }

    private:
        // Helper function for chaining nodes
        template <class T, class U, class... NodeTypes>
        Error ChainNodesImpl(T* leftNode, U* rightNode, NodeTypes*... followingNodes)
        {
            if constexpr (sizeof...(followingNodes) == 0)
            {
                return leftNode->ConnectNext(rightNode);
            }
            else
            {
                Error error = leftNode->ConnectNext(rightNode);
                if (error != Ok)
                {
                    return error;
                }
                return ChainNodesImpl(rightNode, followingNodes...);
            }
        }

        template <class T>
        Error ChainNodesImpl(T* first)
        {
            return Ok;
        }

    private:
        Set<AudioNode*> _Nodes;
    };

    class IAudioNodeController
    {
    public:
        const String& GetName();
        Error GetParameter(U32 parameterIndex, AudioNodeParameter* parameter) const;
        U32 GetParametersCount() const;

    private:
        AudioNode* _Node;
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
        Set<IAudioNodeController*> controls;

    private:
        U32 _Id;
        U32 _Priority;
        AudioAsset* _AudioAsset;
        AudioNode* _InputNode;
    };

    class IAudioService
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
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

    using AudioPlaybackCallback = void(*)(U8* destination, U32 channels, U32 frames, Error error);

    class IAudioDeviceManager : public IAudioService
    {
    public:
        virtual Error RegisterPlaybackCallback(AudioPlaybackCallback callback, void* userData) = 0;
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
        AudioAsset* LoadAudioAsset(const String& filePath);
        Error UnloadAudioAsset(const AudioAsset* audioAsset);
        AudioSource* CreateAudioSource(const AudioAsset* audioAsset, const AudioNode* inputNode);
        Error DestroyAudioSource(const AudioSource* audioSource);

    private:
        AudioManagerServices _Services;
        AudioManagerConfig _Config;
        Map<AudioAsset*, Set<AudioSource*>> _AudioSources;
        AudioGraph _AudioGraph;
    };
}