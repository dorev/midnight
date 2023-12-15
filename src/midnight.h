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
        EndOfFile,
        ExceedingLimits,
        Unknown = U32MAX
    };

    using Decibel = F32;

    Decibel LinearToDecibel(F32 linearVolume)
    {
        if (linearVolume <= 0.0f)
            return -80.0f;
        return 20.0f * std::log10(linearVolume);
    }

    F32 DecibelToLinear(F32 decibel)
    {
        if (decibel <= -80.0f)
            return 0.0f;
        return std::pow(10.0f, decibel / 20.0f);
    }

    enum class AudioFormat
    {
        NotSupported,
        Int16,
        Int32,
        Float32
    };

    // Encapsulates an audio data pointer and the information related
    // to its structure (channels, sample rate, etc.)
    class AudioSampleBuffer
    {
    public:
        U8* _Data;
        U32 _Channels;
        U32 _SampleRate;
        U32 _Size;
        AudioFormat _Format;

        ~AudioSampleBuffer()
        {
            // Data is not freed when this class is destructed
            // TODO: Log a warning?
        }

        AudioSampleBuffer(const AudioSampleBuffer& other)
        {
            _Data = other._Data;
            _Channels = other._Channels;
            _SampleRate = other._SampleRate;
            _Size = other._Size;
            _Format = other._Format;
        }

        AudioSampleBuffer& operator=(const AudioSampleBuffer& other)
        {
            if (this != &other)
            {
                _Data = other._Data;
                _Channels = other._Channels;
                _SampleRate = other._SampleRate;
                _Size = other._Size;
                _Format = other._Format;
            }
            return *this;
        }

        AudioSampleBuffer(AudioSampleBuffer&& other) noexcept
            : _Data(other._Data)
            , _Channels(other._Channels)
            , _SampleRate(other._SampleRate)
            , _Size(other._Size)
            , _Format(other._Format)
        {
            other._Data = nullptr;
            other._Channels = 0;
            other._SampleRate = 0;
            other._Size = 0;
            _Format = other._Format;
        }

        AudioSampleBuffer& operator=(AudioSampleBuffer&& other) noexcept
        {
            if (this != &other)
            {
                _Data = other._Data;
                _Channels = other._Channels;
                _SampleRate = other._SampleRate;
                _Size = other._Size;
                _Format = other._Format;
                other.Clear();
            }
            return *this;
        }

        void Free()
        {
            if (_Data != nullptr)
                delete[] _Data;
            Clear();
        }

        void Clear()
        {
            _Data = nullptr;
            _Channels = 0;
            _SampleRate = 0;
            _Size = 0;
            _Format = AudioFormat::NotSupported;
        }
    };

    // An AudioAsset refers to the original sound. It contains its data, and
    // could eventually also host metadata and 'global' parameters affecting
    // any process refering to that sound
    class AudioAsset
    {
    public:
        Decibel dB;

    public:
        AudioAsset(const String& name, AudioSampleBuffer* buffer)
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
        AudioSampleBuffer* _Buffer;
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
    public:

        AudioNodeState GetState() const
        {
            return _State;
        }

    private:
        // Nodes can only be constructed by AudioGraph
        friend class AudioGraph;
        AudioNode()
        {
        }

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

        Error ConnectPrevious(AudioNode* node)
        {
            if (node == nullptr)
                return Nullptr;
            ScopedMutex lock(_Mutex);
            _InputNodes.Insert(node);
            return Ok;
        }

        Error ConnectNext(AudioNode* node)
        {
            if (node == nullptr)
                return Nullptr;
            ScopedMutex lock(_Mutex);
            _OutputNodes.Insert(node);
            return Ok;
        }

        Error DisconnectNode(AudioNode* node)
        {
            if (node == nullptr)
                return Nullptr;
            ScopedMutex lock(_Mutex);
            _InputNodes.Remove(node);
            _OutputNodes.Remove(node);
            return Ok;
        }

        virtual Error Process()
        {
            ScopedMutex lock(_Mutex);
            if (_State != AudioNodeState::PendingProcessing)
                return UnexpectedState;
            for (AudioNode* node : _InputNodes)
            {
            }
            for (AudioNode* node : _OutputNodes)
            {
            }
            return Ok;
        }

    protected:
        Mutex _Mutex;
        AudioSampleBuffer* _Buffer;
        Set<AudioNode*> _InputNodes;
        Set<AudioNode*> _OutputNodes;
        Atomic<AudioNodeState> _State;
    };

    // Supported node parameter types
    enum class AudioNodeParameterType
    {
        NotSupported,
        Unsigned32,
        Signed32,
        Float32,
        Boolean,
        Vector3,
        Transform,
    };

    // Object encapsulating the value of an audio node parameter
    class AudioNodeParameter
    {
    public:
        using ValueType = Variant<U32, S32, F32, Bool, Vector3, Transform>;

        AudioNodeParameter(const String& name, AudioNodeParameterType type, ValueType initialValue = 0, Bool hasLimits = false, ValueType min = 0, ValueType max = 0)
            : _Name(name)
            , _Type(type)
            , _Value(initialValue)
            , _HasLimits(hasLimits)
            , _Min(min)
            , _Max(max)
        {
        }

        template <class T>
        Error SetValue(const T& value)
        {
            if constexpr (GetParameterType<T>() == _Type)
            {
                if (_HasLimits && (value > _Max || value < _Min))
                    return ExceedingLimits;
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
            if constexpr (GetParameterType<T>() == _Type)
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
        const String _Name;
        const AudioNodeParameterType _Type;
        ValueType _Value;
        Bool _HasLimits;
        ValueType _Min;
        ValueType _Max;

    private:
        template <class T>
        constexpr AudioNodeParameterType GetParameterType()
        {
            if constexpr (IsSame<T, U32>)
                return AudioNodeParameterType::Unsigned32;
            else if constexpr (IsSame<T, 3>)
                return AudioNodeParameterType::Signed32;
            else if constexpr (IsSame<T, Bool>)
                return AudioNodeParameterType::Boolean;
            else if constexpr (IsSame<T, Vector3>)
                return AudioNodeParameterType::Vector3;
            else if constexpr (IsSame<T, Transform>)
                return AudioNodeParameterType::Transform;
            return AudioNodeParameterType::NotSupported;
        };
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
            if (node != nullptr)
            {
                if (_Nodes.Insert(static_cast<AudioNode*>(node)))
                    return node;
                else
                    delete node;
            }
            return nullptr;
        }

        template <class T>
        Error RemoveNode(const T* node)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");

            if (node == nullptr)
                return Nullptr;
            if (_Nodes.Remove(SharedPtrCast<AudioNode>(node)))
                return Ok;
            else
                return CannotFind;
        }

        template <class T, class... NodeTypes>
        Error Connect(T* node, NodeTypes*... followingNodes)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");
            static_assert((IsDerivedFrom<AudioNode, NodeTypes> && ...), "All types must be derived from AudioNode");

            if (node == nullptr || ((followingNodes == nullptr) || ...))
                return Nullptr;
            return ConnectNodesImpl(node, followingNodes...);
        }

    private:
        template <class T, class U, class... NodeTypes>
        Error ConnectNodesImpl(T* leftNode, U* rightNode, NodeTypes*... followingNodes)
        {
            if constexpr (sizeof...(followingNodes) == 0)
            {
                return leftNode->ConnectNext(rightNode);
            }
            else
            {
                Error error = leftNode->ConnectNext(rightNode);
                if (error != Ok)
                    return error;
                return ConnectNodesImpl(rightNode, followingNodes...);
            }
        }

        template <class T>
        Error ConnectNodesImpl(T* first)
        {
            return Ok;
        }

    private:
        Set<AudioNode*> _Nodes;
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
        AudioAsset* _AudioAsset;
        AudioNode* _InputNode;
    };

    class IAudioService
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
        virtual Error Reset() = 0;
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
        virtual Error EnumerateDevices(U32& deviceCount, const AudioDeviceDescription*& devices) = 0;
        virtual Error SelectPlaybackDevice(const AudioDeviceDescription* device) = 0;
        virtual Error Start() = 0;
        virtual Error Stop() = 0;
    };

    class IAudioFile
    {
    public:
        virtual Error Seek(F32 seconds) = 0;
        virtual Error Seek(U32 sample) = 0;
        virtual Error GetSamplePosition(U32& sample) = 0;
        virtual Error GetTimePosition(F32& seconds) = 0;
        virtual Error Read(U32 frameCountRequested, AudioSampleBuffer*& buffer);
    };

    class IAudioDecoder : public IAudioService
    {
    public:
        virtual Error CreateSampleBuffer(const String& filePath, AudioSampleBuffer*& destination) = 0;
        virtual Error OpenFile(const String& filePath, IAudioFile*& audioFile);

    };

    class IAudioResampler : public IAudioService
    {
    public:
        virtual Error Resample(const AudioSampleBuffer* source, AudioSampleBuffer*& destination) = 0;
    };

    class IAudioChannelRemapper : public IAudioService
    {
    public:
        virtual Error Remap(const AudioSampleBuffer* source, AudioSampleBuffer*& destination) = 0;
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