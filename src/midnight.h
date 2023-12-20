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
        InvalidEnumValue,
        CallingStub,
        ServiceUnavailable,
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
    enum class AudioServiceType
    {
        Graph,
        Decoder,
        Resampler,
        ChannelRemapper,
        DeviceManager,
        BufferProvider
    };

    class IAudioService
    {
    public:
        virtual AudioServiceType GetType() const = 0;
        virtual const Char* GetName() const = 0;

        IAudioService()
        {
        }

        IAudioService(const IAudioService&) = delete;
        IAudioService& operator=(const IAudioService&) = delete;

        virtual ~IAudioService()
        {
        }

        virtual Result Initialize()
        {
            return Result::Ok;
        }

        virtual void Shutdown()
        {
        }
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

    class AudioBuffer;
    class IAudioBufferProvider : public IAudioService
    {
    public:
        AudioServiceType GetType() const override
        {
            return AudioServiceType::BufferProvider;
        }

        virtual Result AllocateBuffer(AudioBuffer& buffer) = 0;
        virtual Result ReleaseBuffer(AudioBuffer& buffer) = 0;
    };

    class AudioBufferProviderStub : public IAudioBufferProvider
    {
    public:
        const Char* GetName() const final override
        {
            return "IAudioBufferProvider stub";
        }

        Result AllocateBuffer(AudioBuffer& buffer) final override
        {
            return Result::CallingStub;
        }

        Result ReleaseBuffer(AudioBuffer& buffer) final override
        {
            return Result::CallingStub;
        }
    };

    // Encapsulates an audio data pointer and the information related
    // to its structure (channels, sample rate, etc.)
    class AudioBuffer
    {
    public:
        U32 size;
        U8* data;
        U32 channels;
        U32 sampleRate;
        AudioFormat format;

    public:
        AudioBuffer(IAudioBufferProvider* pool = nullptr, U8* data = nullptr, U32 capacity = 0)
            : data(data)
            , size(0)
            , channels(0)
            , sampleRate(0)
            , format(AudioFormat::NotSpecified)
            , _Pool(pool)
            , _Capacity(capacity)
            , _RefCount(nullptr)
        {
            if (_Pool != nullptr)
                _RefCount = new Atomic<U32>(1);
        }

        virtual ~AudioBuffer()
        {
            DecrementRefCount();
        }

        AudioBuffer(const AudioBuffer& other)
            : data(other.data)
            , size(other.size)
            , channels(other.channels)
            , sampleRate(other.sampleRate)
            , format(other.format)
            , _Pool(other._Pool)
            , _Capacity(other._Capacity)
            , _RefCount(other._RefCount)
        {
            if (_Pool != nullptr && _RefCount != nullptr)
                _RefCount->fetch_add(1);
        }

        AudioBuffer& operator=(const AudioBuffer& other)
        {
            if (this != &other)
            {
                DecrementRefCount();
                data = other.data;
                size = other.size;
                channels = other.channels;
                sampleRate = other.sampleRate;
                format = other.format;
                _Pool = other._Pool;
                _Capacity = other._Capacity;
                _RefCount = other._RefCount;
                if (_Pool != nullptr && _RefCount != nullptr)
                    _RefCount->fetch_add(1);
            }
            return *this;
        }

        template <class T = U8>
        T* GetData()
        {
            return reinterpret_cast<T*>(data);
        }

    private:
        void DecrementRefCount()
        {
            if (_Pool != nullptr
                && _RefCount != nullptr
                && _RefCount->fetch_sub(1) == 1)
            {
                _Pool->ReleaseBuffer(*this);
                delete _RefCount;
                _RefCount = nullptr;
                data = nullptr;
            }
        }

    private:
        IAudioBufferProvider* _Pool;
        U32 _Capacity;
        Atomic<U32>* _RefCount;
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

    // The device calls back using an AudioBuffer to provide all the information on the format
    // of the buffer it requires.
    using AudioDevicePlaybackCallback = void(*)(AudioBuffer& outputBuffer, void* userData);

    class IAudioDeviceManager : public IAudioService
    {
    public:
        AudioServiceType GetType() const final override
        {
            return AudioServiceType::DeviceManager;
        }

        virtual Result RegisterPlaybackCallback(AudioDevicePlaybackCallback callback, void* userData) = 0;
        virtual Result EnumerateDevices(U32& deviceCount, const AudioDeviceDescription*& devices) = 0;
        virtual Result SelectPlaybackDevice(const AudioDeviceDescription* device) = 0;
        virtual Result SelectDefaultPlaybackDevice() = 0;
        virtual Result Start() = 0;
        virtual Result Stop() = 0;
    };

    class AudioDeviceManagerStub : public IAudioDeviceManager
    {
    public:
        const Char* GetName() const final override
        {
            return "IAudioDeviceManager stub";
        }

        Result RegisterPlaybackCallback(AudioDevicePlaybackCallback callback, void* userData) final override
        {
            return Result::CallingStub;
        }

        Result EnumerateDevices(U32& deviceCount, const AudioDeviceDescription*& devices) final override
        {
            return Result::CallingStub;
        }

        Result SelectPlaybackDevice(const AudioDeviceDescription* device) final override
        {
            return Result::CallingStub;
        }

        Result SelectDefaultPlaybackDevice() final override
        {
            return Result::CallingStub;
        }

        Result Start() final override
        {
            return Result::CallingStub;
        }

        Result Stop() final override
        {
            return Result::CallingStub;
        }

    };

    //
    // File decoding
    //
    class IAudioFile
    {
    public:
        virtual ~IAudioFile()
        {
        }

        virtual Result Seek(F32 seconds) = 0;
        virtual Result Seek(U32 frame) = 0;
        virtual Result GetFramePosition(U32& frame) = 0;
        virtual Result GetTimePosition(F32& seconds) = 0;
        virtual Result Read(U32 frameCountRequested, AudioBuffer*& buffer) = 0;
    };

    class IAudioDecoder : public IAudioService
    {
    public:
        AudioServiceType GetType() const final override
        {
            return AudioServiceType::Decoder;
        }
        virtual Result CreateSampleBuffer(const String& filePath, AudioBuffer*& destination) = 0;
        virtual Result OpenFile(const String& filePath, IAudioFile*& audioFile) = 0;
    };

    class AudioDecoderStub : public IAudioDecoder
    {
    public:
        const Char* GetName() const final override
        {
            return "IAudioDecoder stub";
        }

        virtual Result CreateSampleBuffer(const String& filePath, AudioBuffer*& destination) final override
        {
            return Result::CallingStub;
        }

        virtual Result OpenFile(const String& filePath, IAudioFile*& audioFile) final override
        {
            return Result::CallingStub;
        }
    };

    //
    // Resampling
    //
    class IAudioResampler : public IAudioService
    {
    public:
        AudioServiceType GetType() const final override
        {
            return AudioServiceType::Resampler;
        }
        virtual Result Resample(const AudioBuffer* source, AudioBuffer*& destination) = 0;
    };

    class AudioResamplerStub : public IAudioResampler
    {
    public:
        const Char* GetName() const final override
        {
            return "IAudioResampler stub";
        }

        Result Resample(const AudioBuffer* source, AudioBuffer*& destination) final override
        {
            return Result::CallingStub;
        }
    };

    //
    // Channel remapping
    //
    class IAudioChannelRemapper : public IAudioService
    {
    public:
        AudioServiceType GetType() const final override
        {
            return AudioServiceType::ChannelRemapper;
        }
        virtual Result Remap(const AudioBuffer* source, AudioBuffer*& destination) = 0;
    };

    class AudioChannelRemapperStub : public IAudioChannelRemapper
    {
    public:
        const Char* GetName() const final override
        {
            return "IAudioChannelRemapper stub";
        }

        Result Remap(const AudioBuffer* source, AudioBuffer*& destination) final override
        {
            return Result::CallingStub;
        }
    };

    //
    // Audio graph
    //
    enum class AudioGraphState
    {
        Idle,
        Busy
    };

    class IAudioGraph;
    class AudioNode;

    class IAudioGraph : public IAudioService
    {
    public:
        AudioServiceType GetType() const final override
        {
            return AudioServiceType::Graph;
        }

        virtual Result Execute(AudioBuffer& outputBuffer) = 0;
        virtual AudioGraphState GetState() const = 0;

    protected:
        void VisitNode(AudioNode* node)
        {
            node->_Visited = true;
        }

        void ClearNodeVisit(AudioNode* node)
        {
            node->_Visited = false;
        }

        Bool NodeWasVisited(AudioNode* node)
        {
            return node->_Visited;
        }

        Set<AudioNode*>& GetNodeOutputNodes(AudioNode* node)
        {
            return node->_OutputNodes;
        }

        Set<AudioNode*>& GetNodeInputNodes(AudioNode* node)
        {
            return node->_InputNodes;
        }
    };

    class AudioGraphStub : public IAudioGraph
    {
        const Char* GetName() const final override
        {
            return "IAudioGraph stub";
        }

        Result Execute(AudioBuffer& buffer) final override
        {
            return Result::CallingStub;
        }

        AudioGraphState GetState() const final override
        {
            return AudioGraphState::Busy;
        }
    };

    //
    // System
    //

    struct AudioSystemConfig
    {
        U32 maxAudibleSources;
    };

    class IAudioSystem
    {
    public:
        virtual ~IAudioSystem()
        {
        }

        IAudioSystem& GetSystemInterface()
        {
            return *this;
        }

        const AudioSystemConfig& GetConfig() const
        {
            return _Config;
        }

        IAudioGraph& GetGraph()
        {
            if (_Graph == nullptr)
                return _GraphStub;
            return *_Graph;
        }

        IAudioDecoder& GetDecoder()
        {
            if (_Decoder == nullptr)
                return _DecoderStub;
            return *_Decoder;
        }

        IAudioDeviceManager& GetDeviceManager()
        {
            if (_DeviceManager == nullptr)
                return _DeviceManagerStub;
            return *_DeviceManager;
        }

        IAudioResampler& GetResampler()
        {
            if (_Resampler == nullptr)
                return _ResamplerStub;
            return *_Resampler;
        }

        IAudioChannelRemapper& GetChannelRemapper()
        {
            if (_ChannelRemapper == nullptr)
                return _ChannelRemapperStub;
            return *_ChannelRemapper;
        }

        IAudioBufferProvider& GetBufferProvider()
        {
            if (_BufferProvider == nullptr)
                return _BufferProviderStub;
            return *_BufferProvider;
        }

    protected:
        void Configure(const AudioSystemConfig& config)
        {
            _Config = config;
        }

        // Once a service is set this way, IAudioSystem will manage its lifetime
        Result SetService(IAudioService* service)
        {
            if (service == nullptr)
                return Result::Nullptr;

            switch(service->GetType())
            {
                case AudioServiceType::Graph:
                    if (_Graph != nullptr)
                        _Graph->Shutdown();
                    _Graph.Reset(static_cast<IAudioGraph*>(service));
                    return _Graph->Initialize();

                case AudioServiceType::Decoder:
                    if (_Decoder != nullptr)
                        _Decoder->Shutdown();
                    _Decoder.Reset(static_cast<IAudioDecoder*>(service));
                    return _Decoder->Initialize();

                case AudioServiceType::BufferProvider:
                    if (_BufferProvider != nullptr)
                        _BufferProvider->Shutdown();
                    _BufferProvider.Reset(static_cast<IAudioBufferProvider*>(service));
                    return _BufferProvider->Initialize();

                case AudioServiceType::Resampler:
                    if (_Resampler != nullptr)
                        _Resampler->Shutdown();
                    _Resampler.Reset(static_cast<IAudioResampler*>(service));
                    return _Resampler->Initialize();

                case AudioServiceType::ChannelRemapper:
                    if (_ChannelRemapper != nullptr)
                        _ChannelRemapper->Shutdown();
                    _ChannelRemapper.Reset(static_cast<IAudioChannelRemapper*>(service));
                    return _ChannelRemapper->Initialize();

                case AudioServiceType::DeviceManager:
                    if (_DeviceManager != nullptr)
                        _DeviceManager->Shutdown();
                    _DeviceManager.Reset(static_cast<IAudioDeviceManager*>(service));
                    return _Graph->Initialize();

                default:
                    return Result::InvalidEnumValue;
            }
        }

        AudioGraphStub _GraphStub;
        AudioDecoderStub _DecoderStub;
        AudioDeviceManagerStub _DeviceManagerStub;
        AudioResamplerStub _ResamplerStub;
        AudioChannelRemapperStub _ChannelRemapperStub;
        AudioBufferProviderStub _BufferProviderStub;

        AudioSystemConfig _Config;
        UniquePtr<IAudioGraph> _Graph;
        UniquePtr<IAudioDecoder> _Decoder;
        UniquePtr<IAudioDeviceManager> _DeviceManager;
        UniquePtr<IAudioResampler> _Resampler;
        UniquePtr<IAudioChannelRemapper> _ChannelRemapper;
        UniquePtr<IAudioBufferProvider> _BufferProvider;
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

        virtual ~AudioAsset()
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
        AudioNode()
            : _State(AudioNodeState::Idle)
        {
        }

        virtual ~AudioNode()
        {
        }

        // Method to override for custom node processing
        virtual Result Execute(AudioBuffer& destinationBuffer) = 0;

        // Called when the graph creates the node
        virtual Result Initialize()
        {
            return Result::Ok;
        }

        // Called when the node is removed
        virtual Result Shutdown()
        {
            return Result::Ok;
        }

        AudioNodeState GetState() const
        {
            return _State;
        }

        const Char* GetName() const
        {
            return _Name.CStr();
        }

        Result AddInput(AudioNode* node)
        {
            if (node == nullptr)
                return Result::Nullptr;
            if (_InputNodes.Insert(node))
                return Result::Ok;
            else
                return Result::UnableToConnect;
        }

        Result AddOutput(AudioNode* node)
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

    protected:
        void SetName(const Char* name)
        {
            _Name = name;
        }

    protected:
        Atomic<AudioNodeState> _State;
        AudioBuffer _Buffer;
        Set<AudioNode*> _InputNodes;
        Set<AudioNode*> _OutputNodes;

    private:
        friend class IAudioGraph;
        Bool _Visited;

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

        const Char* GetName() const
        {
            return _Name.CStr();
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
        AudioGraph(IAudioSystem& system)
            : _System(system)
            , _State(AudioGraphState::Idle)
        {
        }

        AudioGraphState GetState() const override
        {
            return _State;
        }

        const Char* GetName() const override
        {
            return "AudioGraph";
        }

        Result Execute(AudioBuffer& outputBuffer)
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




            return Result::Ok;
        }

        template <class T, class... Args>
        T* CreateNode(Args... args)
        {
            static_assert(IsDerivedFrom<AudioNode, T>, "T must be derived from AudioNode");

            T* node = new T(std::forward<Args>(args)...);
            if (node != nullptr)
            {
                ScopedMutex lock(_UpdateNodesMutex);
                AudioNode* nodeBase = static_cast<AudioNode*>(node);
                if (_NodesToAdd.Insert(nodeBase))
                {
                    LOG("Added node %s to AudioGraph.", node->GetName());
                    Result result = nodeBase->Initialize();
                    if (result != Result::Ok)
                        LOG_WARNING("Failed node %s initialization.", node->GetName());
                }
                else
                {
                    LOG_WARNING("Unable to add node %s to AudioGraph. Shutting down and deallocating node.", node->GetName());
                    node->Shutdown();
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

            if (node == nullptr || ((followingNodes == nullptr) || ...))
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
            {
                _Nodes.Remove(node);
                node->Shutdown();
                delete node;
            }
            _NodesToRemove.Clear();
            for (AudioNode* node : _NodesToAdd)
                _Nodes.Insert(node);
            _NodesToAdd.Clear();

            if (_Nodes.Empty())
            {
                _OutputNode = nullptr; // Just in case
                return Result::MissingOutputNode;
            }

            Set<AudioNode*> outputNodes;
            ClearNodesVisitedFlag();
            for (AudioNode* node : _Nodes)
                SearchOutputNodes(node, outputNodes);

            if (outputNodes.Size() == 1)
            {
                AudioNode* node = *outputNodes.begin();
                if (node == _OutputNode)
                {
                    // The only leaf is the current output node
                    return Result::Ok;
                }
                else
                {
                    if (_Nodes.Contains(_OutputNode))
                    {
                        // This means the output node has an output node behind it...
                        // That's not normal!
                        return Result::UnexpectedState;
                    }
                    else
                    {
                        // Update the node found as the official output node
                        _OutputNode = node;
                    }
                }
            }
            else
            {
                if (_Nodes.Contains(_OutputNode))
                {
                    // All the output nodes that are not the official output node should be connected to it
                    outputNodes.Remove(_OutputNode);
                }
                else
                {
                    // A new output node must be created
                    _OutputNode = static_cast<AudioNode*>(new MixingNode);
                    _Nodes.Insert(_OutputNode);
                }
                for (AudioNode* node : outputNodes)
                    node->AddOutput(_OutputNode);
            }
            return Result::Ok;
        }

        void SearchOutputNodes(AudioNode* node, Set<AudioNode*>& outputNodes)
        {
            if (NodeWasVisited(node))
                return;

            VisitNode(node);

            Set<AudioNode*>& outputNodes = GetNodeOutputNodes(node);
            if (outputNodes.Empty())
            {
                outputNodes.Insert(node);
            }
            else
            {
                for (AudioNode* nodeOutput : GetNodeOutputNodes(node))
                    SearchOutputNodes(nodeOutput, outputNodes);
            }
        }

        void ClearNodesVisitedFlag()
        {
            for (AudioNode* node : _Nodes)
                ClearNodeVisit(node);
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
        IAudioSystem& _System;
        Mutex _UpdateNodesMutex;
        Set<AudioNode*> _Nodes;
        Set<AudioNode*> _NodesToAdd;
        Set<AudioNode*> _NodesToRemove;
        AudioNode* _OutputNode;
        Atomic<AudioGraphState> _State;
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
        virtual Result Execute(AudioBuffer& buffer)
        {
            return Result::Ok;
        }

    private:
        U32 _Id;
        U32 _Priority;
        AudioAsset* _AudioAsset;
    };

    class AudioSystem : public IAudioSystem
    {
    public:
        using IAudioSystem::Configure;
        using IAudioSystem::SetService;

        Result Initialize()
        {
            IAudioDeviceManager& deviceManager = GetDeviceManager();
            Result result = Result::Unknown;
            result = deviceManager.Initialize();
            if (result != Result::Ok)
            {
                LOG_ERROR("Failed to initialize device manager.");
                return result;
            }
            result = deviceManager.SelectDefaultPlaybackDevice();
            if (result != Result::Ok)
            {
                LOG_ERROR("Failed to select default device.");
                return result;
            }
            result = deviceManager.RegisterPlaybackCallback(PlaybackCallback, this);
        }

        static void PlaybackCallback(AudioBuffer& outputBuffer, void* userData)
        {
            IAudioSystem* system = reinterpret_cast<IAudioSystem*>(userData);
            if (system == nullptr)
            {
                LOG_ERROR("IAudioSystem not available in PlaybackCallback.");
                return;
            }

            IAudioGraph& graph = system->GetGraph();
            graph.Execute(outputBuffer);
        }

        Result Shutdown();
        AudioAsset* LoadAudioAsset(const String& filePath);
        Result UnloadAudioAsset(const AudioAsset* audioAsset);
        AudioSource* CreateAudioSource(const AudioAsset* audioAsset, const AudioNode* inputNode);
        Result DestroyAudioSource(const AudioSource* audioSource);

    private:
        AudioSystemConfig _Config;
        Map<AudioAsset*, Set<AudioSource*>> _AudioSources;
        IAudioGraph* _AudioGraph;
    };
}