#pragma once

#include <cmath>

#include "types.h"

namespace Midnight
{
    // This enum is the type used for any result or error code
    enum class Result : U32
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
        NotReady,
        AlreadyProcessing,
        UnableToConnect,
        UnableToAddNode,
        Unknown = U32MAX
    };

    const Char* ResultToString(Result result)
    {
        switch(result)
        {
            case Result::Ok: return "Ok";
            case Result::Nullptr: return "Nullptr";
            case Result::InvalidPosition: return "Invalid Position";
            case Result::UnsupportedFormat: return "Unsupported Format";
            case Result::InvalidFile: return "Invalid File";
            case Result::OutputDeviceDisconnected: return "Output Device Disconnected";
            case Result::WrongParameterType: return "Wrong Parameter Type";
            case Result::CannotFind: return "Cannot Find";
            case Result::NotYetImplemented: return "Not Yet Implemented";
            case Result::UnexpectedState: return "Unexpected State";
            case Result::EndOfFile: return "End Of File";
            case Result::ExceedingLimits: return "Exceeding Limits";
            case Result::NotReady: return "Not Ready";
            case Result::AlreadyProcessing: return "Already Working";
            case Result::UnableToConnect: return "Unable To Connect";
            default:
            case Result::Unknown: return "Unknown";
        }
    }

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

    DECLARE_FLAG_ENUM(AudioFormat, U32)
    {
        NotSpecified = 0,
        FLAG(Int16, 0),
        FLAG(Int24, 1),
        FLAG(Int32, 2),
        FLAG(Float32, 3),

        FLAG(DirectSpeakers, 8),
        FLAG(AudioObject, 9),
        FLAG(Ambisonic, 10),
    };


    // Encapsulates an audio data pointer and the information related
    // to its structure (channels, sample rate, etc.)
    class AudioSampleBuffer
    {
    public:
        U8* _Data;
        U32 _Size;
        U32 _Channels;
        U32 _SampleRate;
        AudioFormat _Format;

        AudioSampleBuffer(U8* data = nullptr, U32 size = 0, U32 channels = 0, U32 sampleRate = 0, AudioFormat format = AudioFormat::NotSpecified)
            : _Data(data)
            , _Size(size)
            , _Channels(channels)
            , _SampleRate(sampleRate)
            , _Format(format)
        {
        }

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
            _Format = AudioFormat::NotSpecified;
        }
    };

    // An AudioAsset refers to the original sound. It contains its data, and
    // could eventually also host metadata and 'global' parameters affecting
    // any process refering to that sound
    class AudioAsset
    {
    public:
        AudioAsset(const String& name, AudioSampleBuffer* buffer)
            : _Name(name)
            , _Buffer(buffer)
            , _dB(0.f)
        {
        }

        const String& GetName() const
        {
            return _Name;
        }

        Decibel GetVolume() const
        {
            return _dB;
        }

        void SetVolume(Decibel volume)
        {
            _dB = volume;
        }

        const AudioSampleBuffer* GetBuffer() const
        {
            return _Buffer;
        }

    private:
        String _Name;
        Decibel _dB;
        AudioSampleBuffer* _Buffer;
    };

    // Possible states of an audio node
    enum class AudioNodeState
    {
        WaitingForDependencies,
        PendingProcessing,
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

        const String& GetName() const
        {
            return _Name;
        }

    protected:
        //
        // Methods to override for custom node processing
        //

        virtual Result Prepare()
        {
            return Result::Ok;
        }

        virtual Result Process(const AudioAsset* asset, AudioSampleBuffer* input, AudioSampleBuffer* output) = 0;

    private:
        // Nodes can only be constructed by AudioGraph
        friend class AudioGraph;
        AudioNode()
        {
        }

        Result Prepare(Passkey<AudioGraph>)
        {
            AudioNodeState processingDoneState = AudioNodeState::ProcessingDone;
            CompareExchange(_State, processingDoneState, AudioNodeState::WaitingForDependencies);
            return Prepare();
        }

        Result ConnectPrevious(AudioNode* node)
        {
            if (node == nullptr)
                return Result::Nullptr;
            if (_InputNodes.Insert(node))
                return Result::Ok;
            else
                return Result::UnableToConnect;
        }

        Result ConnectOutput(AudioNode* node)
        {
            if (node ==  nullptr)
                return Result::Nullptr;
            if (_OutputNodes.Insert(node))
                return Result::Ok;
            else
                return Result::UnableToConnect;
        }

        Result DisconnectNode(AudioNode* node)
        {
            if (node ==  nullptr)
                return Result::Nullptr;
            _InputNodes.Remove(node);
            _OutputNodes.Remove(node);
            return Result::Ok;
        }

    private:
        virtual Result Initialize()
        {
            return Result::Ok;
        }

        virtual Result Shutdown()
        {
            return Result::Ok;
        }

        virtual Result Reset()
        {
            return Result::Ok;
        }

    protected:
        Atomic<AudioNodeState> _State;
        AudioSampleBuffer* _Buffer;
        Set<AudioNode*> _InputNodes;
        Set<AudioNode*> _OutputNodes;

    private:
        String _Name;
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
        Result SetValue(const T& value)
        {
            if constexpr (GetParameterType<T>() == _Type)
            {
                if (_HasLimits && (value > _Max || value < _Min))
                    return Result::ExceedingLimits;
                _Value = value;
                return Result::Ok;
            }
            else
            {
                return Result::WrongParameterType;
            }
        }

        template <class T>
        Result GetValue(T& value) const
        {
            if constexpr (GetParameterType<T>() == _Type)
            {
                value = _Value.Get<T>();
                return Result::Ok;
            }
            else
            {
                return Result::WrongParameterType;
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

    enum class AudioGraphState
    {
        Idle,
        Processing
    };

    // Class storing all the audio nodes of the engine
    class AudioGraph
    {
    public:
        AudioGraph()
            : _State(AudioGraphState::Idle)
        {
        }

        AudioGraphState GetState() const
        {
            return _State;
        }

        Result Process(AudioSampleBuffer outputBuffer)
        {
            AudioGraphState idleState = AudioGraphState::Idle;
            if (!CompareExchange(_State, idleState, AudioGraphState::Processing))
                return Result::AlreadyProcessing;
            Result result = Result::Ok;
            result = UpdateNodes();
            if (result != Result::Ok)
                return result;
            result = ValidateGraph(); // NotImplementedYet
            //if (result != Result::Ok)
            //    return result;
            for (AudioNode* node : _Nodes)
                node->Prepare(_Passkey);

            
        }

        template <class T, class... Args>
        T* CreateNode(Args... args)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");

            T* node = new T(std::forward<Args>(args)...);
            if (node != nullptr)
            {
                ScopedMutex lock(_NodesMutex);
                if (_NodesToAdd.Insert(static_cast<AudioNode*>(node)))
                {
                    LOG("Added node %s to AudioGraph.", node->GetName().CStr());
                }
                else
                {
                    LOG_WARNING("Unable to add node %s to AudioGraph. Destroying and deallocating node.", node->GetName().CStr());
                    node.~T();
                    delete node;
                    node = nullptr;
                }
            }
            return node;
        }

        template <class T>
        Result RemoveNode(T* node)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");

            ScopedMutex lock(_NodesMutex);
            if (node == nullptr)
                return Result::Nullptr;
            if (_NodesToRemove.Insert(static_cast<AudioNode*>(node)))
                return Result::Ok;
            else
                return Result::CannotFind;
        }

        template <class T, class U>
        Result Connect(T* leftNode, U* rightNode)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "Left node must be derived from AudioNode");
            static_assert(IsDerivedFrom<AudioNode, U>, "Right node must be derived from AudioNode");

            return leftNode->ConnectOutput(rightNode);
        }

        template <class T, class... NodeTypes>
        Result Chain(T* node, NodeTypes*... followingNodes)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");
            static_assert((IsDerivedFrom<AudioNode, NodeTypes> && ...), "All types must be derived from AudioNode");

            if (node == Result::Nullptr || ((followingNodes == Result::Nullptr) || ...))
                return Result::Nullptr;
            return ChainNodesImpl(node, followingNodes...);
        }

    private:
        template <class T, class U, class... NodeTypes>
        Result ChainNodesImpl(T* leftNode, U* rightNode, NodeTypes*... followingNodes)
        {
            if constexpr (sizeof...(followingNodes) == 0)
            {
                return Connect(leftNode, rightNode);
            }
            else
            {
                Result result = Connect(leftNode, rightNode);
                if (result != Result::Ok)
                    return result;
                return ChainNodesImpl(rightNode, followingNodes...);
            }
        }

        template <class T>
        Result ConnectNodesImpl(T* first)
        {
            return Result::Ok;
        }

        Result UpdateNodes()
        {
            ScopedMutex lock(_NodesMutex);
            for (AudioNode* node : _NodesToRemove)
                _Nodes.Remove(node);
            _NodesToRemove.Clear();
            for (AudioNode* node : _NodesToAdd)
                _Nodes.Insert(node);
            _NodesToAdd.Clear();
        }

        Result ValidateGraph() const
        {
            // Make sure that all path reach the output
            // Make sure that there are no feedback loops
            return Result::NotYetImplemented;
        }

    private:
        Mutex _NodesMutex;
        Set<AudioNode*> _Nodes;
        Set<AudioNode*> _NodesToAdd;
        Set<AudioNode*> _NodesToRemove;
        AudioNode* _OutputNode;
        Atomic<AudioGraphState> _State;

        static Passkey<AudioGraph> _Passkey;
    };

    class AudioSource : public AudioNode
    {
    public:
        Result Play(F32 fadeIn = 0.0f);
        Result Pause(F32 fadeOut = 0.05f);
        Result Stop(F32 fadeOut = 0.05f);
        Result Seek(U32 position);
        U32 framePosition;
        Bool loop;

    private:
        virtual Result Process(AudioAsset* audioAsset, AudioSampleBuffer* input, AudioSampleBuffer* output)
        {
            return Result::Ok;
        }

        virtual Result Prepare()
        {
            AudioNodeState processingDoneState = AudioNodeState::ProcessingDone;
            CompareExchange(_State, processingDoneState, AudioNodeState::PendingProcessing);
            return Result::Ok;
        }

    private:
        U32 _Id;
        U32 _Priority;
        AudioAsset* _AudioAsset;
    };

    class IAudioService
    {
    public:
        virtual Result Initialize() = 0;
        virtual Result Shutdown() = 0;
        virtual Result Reset() = 0;
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

    using AudioPlaybackCallback = void(*)(U8* destination, U32 channels, U32 frames, Result error);

    class IAudioDeviceManager : public IAudioService
    {
    public:
        virtual Result RegisterPlaybackCallback(AudioPlaybackCallback callback, void* userData) = 0;
        virtual Result EnumerateDevices(U32& deviceCount, const AudioDeviceDescription*& devices) = 0;
        virtual Result SelectPlaybackDevice(const AudioDeviceDescription* device) = 0;
        virtual Result Start() = 0;
        virtual Result Stop() = 0;
    };

    class IAudioFile
    {
    public:
        virtual Result Seek(F32 seconds) = 0;
        virtual Result Seek(U32 frame) = 0;
        virtual Result GetFramePosition(U32& frame) = 0;
        virtual Result GetTimePosition(F32& seconds) = 0;
        virtual Result Read(U32 frameCountRequested, AudioSampleBuffer*& buffer);
    };

    class IAudioDecoder : public IAudioService
    {
    public:
        virtual Result CreateSampleBuffer(const String& filePath, AudioSampleBuffer*& destination) = 0;
        virtual Result OpenFile(const String& filePath, IAudioFile*& audioFile);

    };

    class IAudioResampler : public IAudioService
    {
    public:
        virtual Result Resample(const AudioSampleBuffer* source, AudioSampleBuffer*& destination) = 0;
    };

    class IAudioChannelRemapper : public IAudioService
    {
    public:
        virtual Result Remap(const AudioSampleBuffer* source, AudioSampleBuffer*& destination) = 0;
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
        Result Initialize(const AudioManagerServices& services, const AudioManagerConfig& config);
        Result Shutdown();
        AudioAsset* LoadAudioAsset(const String& filePath);
        Result UnloadAudioAsset(const AudioAsset* audioAsset);
        AudioSource* CreateAudioSource(const AudioAsset* audioAsset, const AudioNode* inputNode);
        Result DestroyAudioSource(const AudioSource* audioSource);

    private:
        AudioManagerServices _Services;
        AudioManagerConfig _Config;
        Map<AudioAsset*, Set<AudioSource*>> _AudioSources;
        AudioGraph _AudioGraph;
    };
}