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
        MissingOutputNode,
        OutOfRange,
        BlockOutOfRange,
        BufferOutOfRange,
        FailedAllocation,
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
            case Result::Unknown: return "Unknown";
            default:
                return "No ResultToString conversion available";
        }
    }

    //
    // Interfaces
    //
    class IAudioSystem
    {
    public:
        virtual IAudioGraph& GetGraph() = 0;
        virtual IAudioDecoder& GetDecoder() = 0;
        virtual IAudioDeviceManager& GetDeviceManager() = 0;
        virtual IAudioResampler& GetResampler() = 0;
        virtual IAudioChannelRemapper& GetChannelRemapper() = 0;
    };

    class IAudioService
    {
    public:
        virtual Result Initialize() = 0;
        virtual Result Shutdown() = 0;
        virtual Result Reset() = 0;
    };

    //
    // AudioBuffer and memory management
    //

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

    class IAudioBufferProvider
    {
    public:
        virtual Result AllocateBuffer(AudioBuffer&) = 0;
        virtual Result ReleaseBuffer(AudioBuffer&) = 0;
    };

    // Encapsulates an audio data pointer and the information related
    // to its structure (channels, sample rate, etc.)
    class AudioBuffer
    {
    public:
        AudioBuffer(IAudioBufferProvider& pool, U8* data, U32 capacity)
            : _Pool(pool)
            , _Data(data)
            , _Size(0)
            , _Capacity(capacity)
            , _Channels(0)
            , _SampleRate(0)
            , _Format(AudioFormat::NotSpecified)
            , _RefCount(new Atomic<U32>(1))
        {
        }

        ~AudioBuffer()
        {
            DecrementRefCount();
        }

        AudioBuffer(const AudioBuffer& other)
            : _Pool(other._Pool)
            , _Data(other._Data)
            , _Size(other._Size)
            , _Capacity(other._Capacity)
            , _Channels(other._Channels)
            , _SampleRate(other._SampleRate)
            , _Format(other._Format)
            , _RefCount(other._RefCount)
        {
            if (_RefCount != nullptr)
                _RefCount->fetch_add(1);
        }

        AudioBuffer& operator=(const AudioBuffer& other)
        {
            if (this != &other)
            {
                _Pool = other._Pool;
                _Data = other._Data;
                _Size = other._Size;
                _Capacity = other._Capacity;
                _Channels = other._Channels;
                _SampleRate = other._SampleRate;
                _Format = other._Format;
                _RefCount = other._RefCount;
                if (_RefCount != nullptr)
                    _RefCount->fetch_add(1);
            }
            return *this;
        }

        U8* GetData()
        {
            return _Data;
        }

    private:
        void DecrementRefCount()
        {
            if (_RefCount != nullptr && _RefCount->fetch_sub(1) == 1)
            {
                _Pool.ReleaseBuffer(*this);
                delete _RefCount;
                _RefCount = nullptr;
                _Data = nullptr;
            }
        }

    private:
        U8* _Data;
        U32 _Size;
        U32 _Capacity;

        U32 _Channels;
        U32 _SampleRate;
        AudioFormat _Format;

        Atomic<U32>* _RefCount;
        IAudioBufferProvider& _Pool;
    };

    template <U32 BlockSize = 32>
    class AudioBufferPool : IAudioBufferProvider
    {
    private:
        static constexpr U32 TailSentinel = U32MAX;

        class Block
        {
        public:
            U32 buffers[BlockSize];

            Block(U32 bufferSize)
                : _Data(new U8[bufferSize * BlockSize])
                , _BufferSize(bufferSize)
            {
                if (_Data == nullptr)
                    LOG_ERROR("Failed to allocate buffer block!");
                else
                    ZeroizeMemory(_Data, bufferSize * BlockSize);
            }

            Block(const Block&) = delete;
            Block& operator=(const Block&) = delete;

            ~Block()
            {
                if (_Data != nullptr)
                    delete[] _Data;
            }

            U8* GetBufferData(U32 index)
            {
                if (_Data != nullptr)
                    return &_Data[_BufferSize * index];
                else
                    return nullptr;
            }

        private:
            U8* _Data;
            U32 _BufferSize;
        };

    public:
        AudioBufferPool(U32 bufferCapacity)
            : _BufferCapacity(bufferCapacity)
            , _Head(0)
        {
            InitializeNewBlock();
        }

        virtual Result AllocateBuffer(AudioBuffer& buffer)
        {
            U32 currentIndex = TailSentinel;
            do
            {
                currentIndex = _Head;
                if (currentIndex == TailSentinel)
                    ExpandPool(currentIndex);
            }
            while (!CompareExchange(_Head, currentIndex, _NextBufferIndex[currentIndex]));

            U32 blockIndex = currentIndex / BlockSize;
            U32 bufferIndex = currentIndex % BlockSize;
            U8* bufferData = _Blocks[blockIndex]->GetBufferData(bufferIndex);
            buffer = AudioBuffer(&this, bufferData, _BufferCapacity);
            return Result::Ok;
        }

        virtual Result ReleaseBuffer(AudioBuffer& buffer)
        {
            U8* data = buffer.GetData();
            if (data == nullptr)
                return Result::Nullptr;
            Block* block = nullptr;
            U32 blockIndex = TailSentinel;
            if (!FindBlockIndex(data, block, blockIndex))
                return Result::BlockOutOfRange;
            U32 bufferIndex = 0;
            if(!FindBufferIndex(data, block, bufferIndex))
                return Result::BufferOutOfRange;
            U32 bufferPoolIndex = blockIndex * BlockSize + bufferIndex;
            U32 currentHead = TailSentinel;
            do
            {
                currentHead = _Head;
                _Blocks[blockIndex]->buffers[bufferIndex] = currentHead;
            }
            while (!CompareExchange(_Head, currentHead, bufferPoolIndex));
            return Result::Ok;
        }

    private:
        void ExpandPool(U32& currentIndex)
        {
            ScopedMutex lock(_ExpansionMutex);
            if (_Head == TailSentinel)
            {
                U32 tailIndexFix = _Blocks.Size() * BlockSize;
                _NextBufferIndex[_NextBufferIndex.Size() - 1] = tailIndexFix;
                _Head = tailIndexFix;
                currentIndex = tailIndexFix;
                InitializeNewBlock();
            }
        }

        Block* InitializeNewBlock()
        {
            Block* block = new Block(_BufferCapacity);
            _Blocks.EmplaceBack(block);

            U32 baseIndex = BlockSize * (_Blocks.Size() - 1);

            for (U32 i = 0; i < BlockSize; ++i)
                _NextBufferIndex.PushBack(baseIndex + i + 1);

            _NextBufferIndex[_NextBufferIndex.Size() - 1] = TailSentinel;

            return block;
        }

        Bool FindBlockIndex(U8* pointer, Block*& block, U32& blockIndex)
        {
            for (blockIndex = 0; blockIndex < m_Blocks.size(); ++blockIndex)
            {
                block= _Blocks[blockIndex]->get();
                U8* blockStart = block->GetBufferData(0);
                U8* blockEnd = blockStart + BlockSize * _BufferCapacity;
                if (pointer >= blockStart && pointer < blockEnd)
                    return true;
            }
            block = nullptr;
            blockIndex = TailSentinel;
            return false;
        }

        Bool FindBufferIndex(U8* pointer, Block* block, U32& bufferIndex)
        {
            for (bufferIndex = 0; bufferIndex < BlockSize; ++bufferIndex)
            {
                if (buffer->GetBufferData(bufferIndex) == pointer)
                    return true;
            }
            bufferIndex = TailSentinel;
            return false;
        }

    private:
        Mutex _ExpansionMutex;
        U32 _BufferCapacity;
        Atomic<U32> _Head;
        Vector<UniquePtr<Block>> _Blocks;
        Vector<U32> _NextBufferIndex;
    };




    //
    // Device management
    //
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

    //
    // File decoding
    //
    class IAudioFile
    {
    public:
        virtual Result Seek(F32 seconds) = 0;
        virtual Result Seek(U32 frame) = 0;
        virtual Result GetFramePosition(U32& frame) = 0;
        virtual Result GetTimePosition(F32& seconds) = 0;
        virtual Result Read(U32 frameCountRequested, AudioBuffer*& buffer);
    };

    class IAudioDecoder : public IAudioService
    {
    public:
        virtual Result CreateSampleBuffer(const String& filePath, AudioBuffer*& destination) = 0;
        virtual Result OpenFile(const String& filePath, IAudioFile*& audioFile);

    };

    //
    // Resampling
    //
    class IAudioResampler : public IAudioService
    {
    public:
        virtual Result Resample(const AudioBuffer* source, AudioBuffer*& destination) = 0;
    };

    //
    // Channel remapping
    //
    class IAudioChannelRemapper : public IAudioService
    {
    public:
        virtual Result Remap(const AudioBuffer* source, AudioBuffer*& destination) = 0;
    };

    //
    // Audio graph
    //
    enum class AudioGraphState
    {
        Idle,
        Busy
    };

    using AudioGraphTask = Result(*)(IAudioGraph&);

    class IAudioGraph
    {
    public:
        virtual Result Execute(AudioGraphTask worker) = 0;
        virtual Result GetNodes(Set<AudioNode*>& nodes) = 0;
        virtual AudioGraphState GetState() const = 0;
    };

    //
    // Audio specific helpers
    //
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

    // An AudioAsset refers to the original sound. It contains its data, and
    // could eventually also host metadata and 'global' parameters affecting
    // any process refering to that sound
    class AudioAsset
    {
    public:
        AudioAsset(const String& name, AudioBuffer* buffer)
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

        const AudioBuffer* GetBuffer() const
        {
            return _Buffer;
        }

    private:
        String _Name;
        Decibel _dB;
        AudioBuffer* _Buffer;
    };

    // Possible states of an audio node
    enum class AudioNodeState
    {
        Waiting,
        Ready,
        Busy,
        Idle,
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
        // Method to override for custom node processing
        virtual Result Execute(Passkey<IAudioGraph>, const AudioAsset* audioAsset, AudioBuffer*& buffer) = 0;

        virtual Result Prepare(Passkey<IAudioGraph>)
        {
            AudioNodeState doneState = AudioNodeState::Idle;
            CompareExchange(_State, doneState, AudioNodeState::Waiting);
            return Ok;
        }

        // Nodes can only be constructed by AudioGraph
        AudioNode()
        {
        }

        Result ConnectPrevious(Passkey<IAudioGraph>, AudioNode* node)
        {
            if (node == nullptr)
                return Result::Nullptr;
            if (_InputNodes.Insert(node))
                return Result::Ok;
            else
                return Result::UnableToConnect;
        }

        Result ConnectOutput(Passkey<IAudioGraph>, AudioNode* node)
        {
            if (node ==  nullptr)
                return Result::Nullptr;
            if (_OutputNodes.Insert(node))
                return Result::Ok;
            else
                return Result::UnableToConnect;
        }

        Result DisconnectNode(Passkey<IAudioGraph>, AudioNode* node)
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
        MemoryPoolReference<AudioBuffer> _Buffer;
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

    // Class storing all the audio nodes of the engine
    class AudioGraph : public IAudioGraph
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

        Result SetupOutputNode(AudioOutput* outputNode)
        {
            return Result::NotYetImplemented;
        }

        Result Execute(AudioBuffer* outputBuffer)
        {
            AudioGraphState idleState = AudioGraphState::Idle;
            if (!CompareExchange(_State, idleState, AudioGraphState::Busy))
                return Result::AlreadyProcessing;
            Result result = Result::Ok;
            result = UpdateNodes();
            if (result != Result::Ok)
                return result;
            result = ValidateGraph();
            if (result != Result::Ok)
                return result;
            for (AudioNode* node : _Nodes)
                node->Prepare();
            return _OutputNode->Execute(nullptr, outputBuffer);
        }

        template <class T, class... Args>
        T* CreateNode(Args... args)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");

            T* node = new T(std::forward<Args>(args)...);
            if (node != nullptr)
            {
                ScopedMutex lock(_UpdateNodesMutex);
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

            ScopedMutex lock(_UpdateNodesMutex);
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
            ScopedMutex lock(_UpdateNodesMutex);
            for (AudioNode* node : _NodesToRemove)
                _Nodes.Remove(node);
            _NodesToRemove.Clear();
            for (AudioNode* node : _NodesToAdd)
                _Nodes.Insert(node);
            _NodesToAdd.Clear();
        }

        Result ValidateGraph() const
        {
            // TODO: Make sure that all path reach the output
            // TODO: Make sure that there are no feedback loops
            if (_OutputNode == nullptr)
                return Result::MissingOutputNode;
            return Result::Ok;
        }

    private:
        Mutex _UpdateNodesMutex;
        Set<AudioNode*> _Nodes;
        Set<AudioNode*> _NodesToAdd;
        Set<AudioNode*> _NodesToRemove;
        AudioNode* _OutputNode;
        Atomic<AudioGraphState> _State;
    };

    class MixerNode : public AudioNode
    {
        virtual Result Execute(const AudioAsset* audioAsset, AudioBuffer*& buffer)
        {

        }
    };

    class AudioOutput : public AudioNode
    {
        virtual Result Execute(const AudioAsset* audioAsset, AudioBuffer*& buffer)
        {
            // This is where everything starts...





            return Result::Ok;
        }
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
        virtual Result Execute(const AudioAsset* audioAsset, AudioBuffer*& buffer)
        {
            return Result::Ok;
        }

        virtual Result Prepare()
        {
            AudioNodeState doneState = AudioNodeState::Idle;
            CompareExchange(_State, doneState, AudioNodeState::Ready);
            return Result::Ok;
        }

    private:
        U32 _Id;
        U32 _Priority;
        AudioAsset* _AudioAsset;
    };

    struct AudioManagerConfig
    {
        U32 maxAudibleSources;
    };

    class AudioSystem
    {
    public:
        Result Initialize(const AudioServices& services, const AudioManagerConfig& config);
        Result Shutdown();
        AudioAsset* LoadAudioAsset(const String& filePath);
        Result UnloadAudioAsset(const AudioAsset* audioAsset);
        AudioSource* CreateAudioSource(const AudioAsset* audioAsset, const AudioNode* inputNode);
        Result DestroyAudioSource(const AudioSource* audioSource);

        AudioGraph& GetAudioGraph()
        {
            return _AudioGraph;
        }

        Result SetAudioDeviceManager(IAudioDeviceManager* deviceManagerImplementation)
        {
            return Result::NotYetImplemented;
        }

    private:
        AudioServices _Services;
        AudioManagerConfig _Config;
        Map<AudioAsset*, Set<AudioSource*>> _AudioSources;
        AudioGraph _AudioGraph;
    };
}