#pragma once


namespace Loom
{

//
// File decoding
//
class IAudioFile
{
public:
    virtual ~IAudioFile()
    {
    }

    virtual Result Seek(float seconds) = 0;
    virtual Result Seek(u32 frame) = 0;
    virtual Result GetFramePosition(u32& frame) = 0;
    virtual Result GetTimePosition(float& seconds) = 0;
    virtual Result Read(u32 frameCountRequested, AudioBuffer*& buffer) = 0;
};

class IAudioCodec : public IAudioSubsystem
{
public:
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::Decoder;
    }
    virtual Result CreateSampleBuffer(const char* filePath, AudioBuffer*& destination) = 0;
    virtual Result OpenFile(const char* filePath, IAudioFile*& audioFile) = 0;
};

class AudioDecoderStub : public IAudioCodec
{
public:
    static AudioDecoderStub& GetInstance()
    {
        static AudioDecoderStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioCodec stub";
    }

    virtual Result CreateSampleBuffer(const char*, AudioBuffer*&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    virtual Result OpenFile(const char*, IAudioFile*&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }
};

//
// Resampling
//
class IAudioResampler : public IAudioSubsystem
{
public:
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::Resampler;
    }
    virtual Result Resample(const AudioBuffer& source, AudioBuffer& destination) = 0;
};

class AudioResamplerStub : public IAudioResampler
{
public:
    static AudioResamplerStub& GetInstance()
    {
        static AudioResamplerStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioResampler stub";
    }

    Result Resample(const AudioBuffer&, AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }
};

//
// Channel remapping
//
class IAudioChannelRemapper : public IAudioSubsystem
{
public:
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::ChannelRemapper;
    }
    virtual Result Remap(const AudioBuffer* source, AudioBuffer*& destination) = 0;
};

class AudioChannelRemapperStub : public IAudioChannelRemapper
{
public:
    static AudioChannelRemapperStub& GetInstance()
    {
        static AudioChannelRemapperStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioChannelRemapper stub";
    }

    Result Remap(const AudioBuffer*, AudioBuffer*&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
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
class IAudioSystem;

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
    AudioNode(IAudioSystem& system, const char* name)
        : _System(system)
        , _Name(name)
        , _State(AudioNodeState::Idle)
    {
    }

    virtual ~AudioNode()
    {
    }

    // Method to override for custom node processing
    virtual Result Execute(AudioBuffer& outputBuffer) = 0;

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

    const char* GetName() const
    {
        return _Name.c_str();
    }

    Result AddInput(shared_ptr<AudioNode> node)
    {
        if (node == nullptr)
            LOOM_RETURN_RESULT(Result::Nullptr);
        if (_InputNodes.insert(node).second)
            return Result::Ok;
        else
            LOOM_RETURN_RESULT(Result::UnableToConnect);
    }

    Result AddOutput(shared_ptr<AudioNode> node)
    {
        if (node ==  nullptr)
            LOOM_RETURN_RESULT(Result::Nullptr);
        if (_OutputNodes.insert(node).second)
            return Result::Ok;
        else
            LOOM_RETURN_RESULT(Result::UnableToConnect);
    }

    Result DisconnectNode(shared_ptr<AudioNode> node)
    {
        if (node ==  nullptr)
            LOOM_RETURN_RESULT(Result::Nullptr);
        _InputNodes.erase(node);
        _OutputNodes.erase(node);
        return Result::Ok;
    }

protected:
    void SetName(const char* name)
    {
        _Name = name;
    }

    AudioBuffer& GetNodeBuffer()
    {
        return _Buffer;
    }

    void ReleaseBuffer()
    {
        _Buffer.Release();
    }

    Result PullInputNodes(AudioBuffer& destinationBuffer)
    {
        if (_InputNodes.empty())
            LOOM_RETURN_RESULT(Result::NoData);
        for (auto itr = _InputNodes.begin(); itr != _InputNodes.end(); itr++)
        {
            shared_ptr<AudioNode> node = *itr;
            node->Execute(destinationBuffer);
            if (itr == _InputNodes.begin())
            {
                _Buffer = destinationBuffer;
            }
            else
            {
                Result result = _Buffer.AddSamplesFrom(destinationBuffer);
                LOOM_CHECK_RESULT(result);
            }
            destinationBuffer.Release();
        }
        destinationBuffer = _Buffer;
        return Result::Ok;
    }


private:
    friend class IAudioGraph;

    atomic<AudioNodeState> _State;
    string _Name;
    AudioBuffer _Buffer;
    set<shared_ptr<AudioNode>> _InputNodes;
    set<shared_ptr<AudioNode>> _OutputNodes;

    IAudioSystem& _System;
    bool _Visited;

};


class IAudioGraph : public IAudioSubsystem
{
public:
    AudioSubsystemType GetType() const final override
    {
        return AudioSubsystemType::Graph;
    }

    virtual Result Execute(AudioBuffer& outputBuffer) = 0;
    virtual AudioGraphState GetState() const = 0;

protected:
    void VisitNode(shared_ptr<AudioNode> node)
    {
        node->_Visited = true;
    }

    void ClearNodeVisit(shared_ptr<AudioNode> node)
    {
        node->_Visited = false;
    }

    bool NodeWasVisited(shared_ptr<AudioNode> node)
    {
        return node->_Visited;
    }

    set<shared_ptr<AudioNode>>& GetNodeOutputNodes(shared_ptr<AudioNode> node)
    {
        return node->_OutputNodes;
    }

    set<shared_ptr<AudioNode>>& GetNodeInputNodes(shared_ptr<AudioNode> node)
    {
        return node->_InputNodes;
    }
};

class AudioGraphStub : public IAudioGraph
{
public:
    static AudioGraphStub& GetInstance()
    {
        static AudioGraphStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return "IAudioGraph stub";
    }

    Result Execute(AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    AudioGraphState GetState() const final override
    {
        LOOM_LOG_RESULT(Result::CallingStub);
        return AudioGraphState::Busy;
    }
};

//
// System
//

struct AudioSystemConfig
{
    u32 maxAudibleSources;
};

class IAudioSystem
{
public:
    virtual ~IAudioSystem()
    {
    }

    IAudioSystem& GetInterface()
    {
        return *this;
    }

    virtual const AudioSystemConfig& GetConfig() const = 0;
    virtual IAudioGraph& GetGraphInterface() = 0;
    virtual IAudioCodec& GetDecoderInterface() const = 0;
    virtual IAudioDeviceManager& GetDeviceManagerInterface() const = 0;
    virtual IAudioResampler& GetResamplerInterface() const = 0;
    virtual IAudioChannelRemapper& GetChannelRemapperInterface() const = 0;
    virtual IAudioBufferProvider& GetBufferProviderInterface() const = 0;
};

//
// Audio specific helpers
//
// An AudioAsset refers to the original sound. It contains its data, and
// could eventually also host metadata and 'global' parameters affecting
// any process refering to that sound
class AudioAsset
{
public:
    AudioAsset(const string& name, AudioBuffer* buffer)
        : _Name(name)
        , _Buffer(buffer)
        , _dB(0.f)
    {
    }

    virtual ~AudioAsset()
    {
    }

    const string& GetName() const
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
    string _Name;
    Decibel _dB;
    AudioBuffer* _Buffer;
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
    using ValueType = variant<u32, s32, float, bool, Vector3, Transform>;

    AudioNodeParameter(const char* name, AudioNodeParameterType type, ValueType initialValue = 0, bool hasLimits = false, ValueType min = 0, ValueType max = 0)
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
        if (GetParameterType<T>() == _Type)
        {
            unique_lock lock(_ValueAccessMutex);
            if (_HasLimits && (value > _Max || value < _Min))
                std::clamp(value, std::get<T>(_Min), std::get<T>(_Max));
            _Value = value;
            return Result::Ok;
        }
        else
        {
            LOOM_RETURN_RESULT(Result::WrongParameterType);
        }
    }

    template <class T>
    Result GetValue(T& value) const
    {
        if (GetParameterType<T>() == _Type)
        {
            shared_lock lock(_ValueAccessMutex);
            value = std::get<T>(_Value);
            return Result::Ok;
        }
        else
        {
            LOOM_RETURN_RESULT(Result::WrongParameterType);
        }
    }

    const char* GetName() const
    {
        return _Name.c_str();
    }

    AudioNodeParameterType GetType() const
    {
        return _Type;
    }

private:
    mutable shared_mutex _ValueAccessMutex;
    const string _Name;
    const AudioNodeParameterType _Type;
    ValueType _Value;
    bool _HasLimits;
    const ValueType _Min;
    const ValueType _Max;

private:
    template <class T>
    static constexpr AudioNodeParameterType GetParameterType()
    {
        if constexpr (std::is_same_v<T, u32>)
            return AudioNodeParameterType::Unsigned32;
        else if constexpr (std::is_same_v<T, s32>)
            return AudioNodeParameterType::Signed32;
        else if constexpr (std::is_same_v<T, bool>)
            return AudioNodeParameterType::Boolean;
        else if constexpr (std::is_same_v<T, Vector3>)
            return AudioNodeParameterType::Vector3;
        else if constexpr (std::is_same_v<T, Transform>)
            return AudioNodeParameterType::Transform;
        return AudioNodeParameterType::NotSupported;
    };
};

class MixingNode : public AudioNode
{
public:
    MixingNode(IAudioSystem& system)
        : AudioNode(system, "MixingNode")
        , _Gain("Gain", AudioNodeParameterType::Float32, 1.0f, true, 0.0f, 10.0f)
    {
    }

    Result Execute(AudioBuffer& destinationBuffer) override
    {
        Result result = PullInputNodes(destinationBuffer);
        LOOM_CHECK_RESULT(result);
        float gain = 1.0f;
        _Gain.GetValue<float>(gain);
        destinationBuffer.MultiplySamplesBy<float>(gain);
        return Result::Ok;
    }

private:
    AudioNodeParameter _Gain;
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

    const char* GetName() const override
    {
        return "AudioGraph";
    }

    Result Execute(AudioBuffer& destinationBuffer)
    {
        AudioGraphState idleState = AudioGraphState::Idle;
        if (!_State.compare_exchange_strong(idleState, AudioGraphState::Busy))
            LOOM_RETURN_RESULT(Result::Busy);
        Result result = Result::Ok;
        result = UpdateNodes();
        LOOM_CHECK_RESULT(result);
        return _OutputNode->Execute(destinationBuffer);
    }

    template <class T, class... Args>
    shared_ptr<T> CreateNode(Args&&... args)
    {
        static_assert(std::is_base_of_v<AudioNode, T>, "T must be derived from AudioNode");

        shared_ptr<T> node = make_shared<T>(std::forward<Args>(args)...);
        if (node != nullptr)
        {
            shared_ptr<AudioNode> nodeBase = shared_ptr_cast<AudioNode>(node);
            scoped_lock lock(_UpdateNodesMutex);
            if (_NodesToAdd.insert(nodeBase).second)
            {
                LOOM_LOG("Added node %s to AudioGraph.", node->GetName());
                Result result = nodeBase->Initialize();
                if (result != Result::Ok)
                    LOOM_LOG_WARNING("Failed node %s initialization.", node->GetName());
            }
            else
            {
                LOOM_LOG_WARNING("Unable to add node %s to AudioGraph. Shutting down and deallocating node.", node->GetName());
                node->Shutdown();
                node = nullptr;
            }
        }
        return node;
    }

    template <class T>
    Result RemoveNode(T* node)
    {
        static_assert(std::is_base_of_v<AudioNode, T>, "T must be derived from AudioNode");

        scoped_lock lock(_UpdateNodesMutex);
        if (node == nullptr)
            LOOM_RETURN_RESULT(Result::Nullptr);
        if (_NodesToRemove.insert(shared_ptr_cast<AudioNode>(node)))
            return Result::Ok;
        else
            LOOM_RETURN_RESULT(Result::CannotFind);
    }

    struct NodeConnection
    {
        template <class T, class U>
        NodeConnection(shared_ptr<T>& sourceNode, shared_ptr<U>& destinationNode)
        {
            static_assert(std::is_base_of_v<AudioNode, T>, "Source node must be derived from AudioNode");
            static_assert(std::is_base_of_v<AudioNode, U>, "Destination node must be derived from AudioNode");
            this->sourceNode = shared_ptr_cast<AudioNode>(sourceNode);
            this->destinationNode = shared_ptr_cast<AudioNode>(destinationNode);
        }
        shared_ptr<AudioNode> sourceNode;
        shared_ptr<AudioNode> destinationNode;
    };

    template <class T, class U>
    Result Connect(shared_ptr<T>& sourceNode, shared_ptr<U>& destinationNode)
    {
        static_assert(std::is_base_of_v<AudioNode, T>, "Source node must be derived from AudioNode");
        static_assert(std::is_base_of_v<AudioNode, U>, "Destination node must be derived from AudioNode");

        _NodesToConnect.emplace_back(sourceNode, destinationNode);
        return Result::Ok;
    }

    template <class T, class... NodeTypes>
    Result Chain(shared_ptr<T>& node, shared_ptr<NodeTypes>&... followingNodes)
    {
        static_assert(std::is_base_of_v<AudioNode, T>, "T must be derived from AudioNode");
        static_assert((std::is_base_of_v<AudioNode, NodeTypes> && ...), "All types must be derived from AudioNode");

        if (node == nullptr || ((followingNodes == nullptr) || ...))
            LOOM_RETURN_RESULT(Result::Nullptr);
        scoped_lock lock(_UpdateNodesMutex);
        return ChainNodesImpl(node, followingNodes...);
    }

private:
    template <class T, class U, class... NodeTypes>
    Result ChainNodesImpl(shared_ptr<T>& leftNode, shared_ptr<U>& rightNode, shared_ptr<NodeTypes>&... followingNodes)
    {
        if constexpr (sizeof...(followingNodes) == 0)
        {
            return Connect(leftNode, rightNode);
        }
        else
        {
            Result result = Connect(leftNode, rightNode);
            LOOM_CHECK_RESULT(result);
            return ChainNodesImpl(rightNode, followingNodes...);
        }
    }

    template <class T>
    Result ChainNodesImpl(shared_ptr<T>& first)
    {
        LOOM_UNUSED(first);
        return Result::Ok;
    }

    Result UpdateNodes()
    {
        scoped_lock lock(_UpdateNodesMutex);
        for (const shared_ptr<AudioNode>& node : _NodesToRemove)
        {
            node->Shutdown();
            _Nodes.erase(node);
        }
        _NodesToRemove.clear();
        for (const shared_ptr<AudioNode>& node : _NodesToAdd)
            _Nodes.insert(node);
        _NodesToAdd.clear();
        if (_Nodes.empty())
        {
            _OutputNode = nullptr; // Just in case
            LOOM_RETURN_RESULT(Result::MissingOutputNode);
        }
        for (const NodeConnection& connection : _NodesToConnect)
            connection.sourceNode->AddOutput(connection.destinationNode);
        _NodesToConnect.clear();

        // Evaluate output node
        set<shared_ptr<AudioNode>> outputNodes;
        ClearNodesVisitedFlag();
        for (shared_ptr<AudioNode> node : _Nodes)
            SearchOutputNodes(node, outputNodes);
        bool nodesContainsOutputNode = _OutputNode != nullptr && _Nodes.find(_OutputNode) != _Nodes.end();
        if (outputNodes.size() == 1)
        {
            shared_ptr<AudioNode> node = *outputNodes.begin();
            if (node == _OutputNode)
            {
                // The only leaf is the current output node
                return Result::Ok;
            }
            else
            {
                if (nodesContainsOutputNode)
                {
                    // This means the output node has an output node behind it...
                    // That's not normal!
                    LOOM_RETURN_RESULT(Result::UnexpectedState);
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
            if (nodesContainsOutputNode)
            {
                // All the output nodes that are not the official output node should be connected to it
                outputNodes.erase(_OutputNode);
            }
            else
            {
                // A new output node must be created
                _OutputNode = shared_ptr_cast<AudioNode>(make_shared<MixingNode>(_System));
                _Nodes.insert(_OutputNode);
            }
            for (shared_ptr<AudioNode> node : outputNodes)
                node->AddOutput(_OutputNode);
        }
        return Result::Ok;
    }

    void SearchOutputNodes(shared_ptr<AudioNode> node, set<shared_ptr<AudioNode>>& outputNodesSearchResult)
    {
        if (NodeWasVisited(node))
            return;
        VisitNode(node);
        set<shared_ptr<AudioNode>>& visitedNodeOutputNodes = GetNodeOutputNodes(node);
        if (visitedNodeOutputNodes.empty())
        {
            outputNodesSearchResult.insert(node);
        }
        else
        {
            for (shared_ptr<AudioNode> outputNode : visitedNodeOutputNodes)
                SearchOutputNodes(outputNode, outputNodesSearchResult);
        }
    }

    void ClearNodesVisitedFlag()
    {
        for (shared_ptr<AudioNode> node : _Nodes)
            ClearNodeVisit(node);
    }

private:
    IAudioSystem& _System;
    atomic<AudioGraphState> _State;
    shared_ptr<AudioNode> _OutputNode;
    set<shared_ptr<AudioNode>> _Nodes;
    set<shared_ptr<AudioNode>> _NodesToAdd;
    set<shared_ptr<AudioNode>> _NodesToRemove;
    vector<NodeConnection> _NodesToConnect;
    mutex _UpdateNodesMutex;
};

class AudioSource : public AudioNode
{
public:
    Result Play(float fadeIn = 0.0f);
    Result Pause(float fadeOut = 0.05f);
    Result Stop(float fadeOut = 0.05f);
    Result Seek(u32 position);
    u32 framePosition;
    bool loop;

private:
    virtual Result Execute(AudioBuffer& destinationBuffer)
    {
        // make sure that the format specified in the destination buffer is respected!
        LOOM_UNUSED(destinationBuffer);
        return Result::Ok;
    }

private:
    u32 _Id;
    u32 _Priority;
    bool _Virtual;
    AudioAsset* _AudioAsset;
};

class AudioSystem : public IAudioSystem
{
public:
    AudioSystem()
        : _Graph(GetInterface())
    {
    }

    Result Initialize()
    {
        IAudioDeviceManager& deviceManager = GetDeviceManagerInterface();
        Result result = Result::Unknown;
        result = deviceManager.Initialize();
        LOOM_CHECK_RESULT(result);
        result = deviceManager.SelectDefaultPlaybackDevice();
        LOOM_CHECK_RESULT(result);
        result = deviceManager.RegisterPlaybackCallback(PlaybackCallback, this);
    }

    AudioGraph& GetGraph()
    {
        return _Graph;
    }

    static void PlaybackCallback(AudioBuffer& destinationBuffer, void* userData)
    {
        IAudioSystem* system = reinterpret_cast<IAudioSystem*>(userData);
        if (system == nullptr)
        {
            LOOM_LOG_ERROR("IAudioSystem not available in PlaybackCallback.");
            return;
        }
        IAudioGraph& graph = system->GetGraphInterface();
        graph.Execute(destinationBuffer);
    }

    Result Shutdown();
    AudioAsset* LoadAudioAsset(const string& filePath);
    Result UnloadAudioAsset(const AudioAsset* audioAsset);
    AudioSource* CreateAudioSource(const AudioAsset* audioAsset, const shared_ptr<AudioNode> inputNode);
    Result DestroyAudioSource(const AudioSource* audioSource);

    const AudioSystemConfig& GetConfig() const override
    {
        return _Config;
    }

    IAudioGraph& GetGraphInterface()
    {
        return *static_cast<IAudioGraph*>(&_Graph);
    }

    IAudioCodec& GetDecoderInterface() const override
    {
        if (_Decoder == nullptr)
            return AudioDecoderStub::GetInstance();
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

    Result SetService(IAudioSubsystem* service)
    {
        if (service == nullptr)
            LOOM_RETURN_RESULT(Result::Nullptr);

        switch(service->GetType())
        {
            case AudioSubsystemType::Decoder:
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

private:
    AudioSystemConfig _Config;
    AudioGraph _Graph;
    map<AudioAsset*, set<AudioSource*>> _AudioSources;
    unique_ptr<IAudioCodec> _Decoder;
    unique_ptr<IAudioDeviceManager> _DeviceManager;
    unique_ptr<IAudioResampler> _Resampler;
    unique_ptr<IAudioChannelRemapper> _ChannelRemapper;
    unique_ptr<IAudioBufferProvider> _BufferProvider;
};

} // namespace Loom
