#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace Loom
{

///////////////////////////////////////////////////////////////////////////////
// Aliases
///////////////////////////////////////////////////////////////////////////////

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s16 = int16_t;
using s32 = int32_t;
using string = std::string;
using mutex = std::mutex;
using scoped_lock = std::lock_guard<mutex>;
using shared_mutex = std::shared_mutex;
using shared_lock = std::shared_lock<shared_mutex>;
using unique_lock = std::unique_lock<shared_mutex>;
template <class... T> using unique_ptr = std::unique_ptr<T...>;
template <class... T> using shared_ptr = std::shared_ptr<T...>;
template <class... T> using atomic = std::atomic<T...>;
template <class... T> using vector = std::vector<T...>;
template <class... T> using set = std::set<T...>;
template <class... T> using map = std::map<T...>;
template <class... T> using variant = std::variant<T...>;

struct Vector3
{
    float x,y,z;
};

struct Quaternion
{
    float w,x,y,z;
};

struct Transform
{
    Vector3 point;
    Quaternion rotation;
};

///////////////////////////////////////////////////////////////////////////////
// Logging
///////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
    #define LOOM_DEBUG_BREAK() __debugbreak()
    #define LOOM_FUNCTION __FUNCSIG__
#elif defined(__GNUC__) || defined(__clang__)
    #define LOOM_DEBUG_BREAK() __builtin_trap()
    #define LOOM_FUNCTION __PRETTY_FUNCTION__
#else
    #define LOOM_DEBUG_BREAK() assert(false)
    #define LOOM_FUNCTION __FUNCTION__
#endif

#define LOOM_LOG(format, ...) printf(format "\n", ##__VA_ARGS__);
#define LOOM_LOG_WARNING(format, ...) printf("[WARNING] {%s}" format "\n", LOOM_FUNCTION, ##__VA_ARGS__);
#define LOOM_LOG_ERROR(format, ...) printf("[ERROR] {%s}" format " [%s l.%d]\n", LOOM_FUNCTION, ##__VA_ARGS__, __FILE__, __LINE__);

#define LOOM_LOG_BAD_RESULT(result) LOOM_LOG_WARNING("Returned %s (%d).", ResultToString(result), static_cast<u32>(result))
#define LOOM_RETURN_RESULT(result) { LOOM_LOG_BAD_RESULT(result); return result; }
#define LOOM_CHECK_RESULT(result) if (result != Result::Ok) { LOOM_RETURN_RESULT(result); }

#define LOOM_DEBUG_ASSERT(condition, format, ...) \
    if (!(condition)) \
    { \
        LOOM_LOG("[ASSERT] " format "\n", ##__VA_ARGS__); \
        LOOM_DEBUG_BREAK(); \
    }

#define LOOM_UNUSED(variable) (void)(variable)
///////////////////////////////////////////////////////////////////////////////
// Flags declaration helper
///////////////////////////////////////////////////////////////////////////////

#define DECLARE_FLAG_ENUM(EnumName, UnderlyingType) \
    enum class EnumName : UnderlyingType; \
    constexpr EnumName operator|(EnumName a, EnumName b) \
    { \
        return static_cast<EnumName>(static_cast<UnderlyingType>(a) | static_cast<UnderlyingType>(b)); \
    } \
    constexpr EnumName operator&(EnumName a, EnumName b) \
    { \
        return static_cast<EnumName>(static_cast<UnderlyingType>(a) & static_cast<UnderlyingType>(b)); \
    } \
    inline EnumName& operator|=(EnumName& a, EnumName b) \
    { \
        return a = a | b; \
    } \
    inline EnumName& operator&=(EnumName& a, EnumName b) \
    { \
        return a = a & b; \
    } \
    enum class EnumName : UnderlyingType

#define FLAG(name, shift) name = 1 << shift

// This enum is the type used for any result or error code
enum class Result : u32
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
    Busy,
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
    BufferCapacityMismatch,
    BufferFormatMismatch,
    NoData,
    InvalidBufferSampleFormat,
    Unknown = UINT32_MAX
};

const char* ResultToString(Result result)
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
        case Result::Busy: return "Already Working";
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
    virtual const char* GetName() const = 0;

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

DECLARE_FLAG_ENUM(AudioFormat, u32)
{
    NotSpecified = 0,
    FLAG(Int16, 0),
    FLAG(Int32, 2),
    FLAG(Float32, 3),
    SampleFormatMask = Int16 | Int32 | Float32,

    FLAG(DirectSpeakers, 8),
    FLAG(AudioObject, 9),
    FLAG(Ambisonic, 10),

    Invalid = UINT32_MAX
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
    static AudioBufferProviderStub& GetInstance()
    {
        static AudioBufferProviderStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_BAD_RESULT(Result::CallingStub);
        return "IAudioBufferProvider stub";
    }

    Result AllocateBuffer(AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result ReleaseBuffer(AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }
};

// Encapsulates an audio data pointer and the information related
// to its structure (channels, sample rate, etc.)
class AudioBuffer
{
public:
    u32 size;
    u8* data;
    u32 channels;
    u32 sampleRate;
    AudioFormat format;

public:
    AudioBuffer(IAudioBufferProvider* pool = nullptr, u8* data = nullptr, u32 capacity = 0)
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
            _RefCount = new atomic<u32>(1);
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

    template <class T = u8>
    T* GetData() const
    {
        return reinterpret_cast<T*>(data);
    }

    void Release()
    {
        if (_Pool == nullptr)
            return;
        DecrementRefCount();
        _Pool = nullptr;
        if (_RefCount != nullptr && _RefCount->load() > 0)
            _RefCount = nullptr;
    }

    Result CopyDataFrom(const AudioBuffer& other) const
    {
        if (data == nullptr || other.data == nullptr)
            LOOM_RETURN_RESULT(Result::NoData);
        if (_Capacity < other.size)
            LOOM_RETURN_RESULT(Result::BufferCapacityMismatch);
        memcpy(data, other.data, other.size);
        return Result::Ok;
    }

    Result MixInBuffer(const AudioBuffer& other)
    {
        if (!FormatMatches(other))
            LOOM_RETURN_RESULT(Result::BufferFormatMismatch);
        AudioFormat sampleFormat = other.format & AudioFormat::SampleFormatMask;
        switch (sampleFormat)
        {
            case AudioFormat::Int16:
                return AddSamplesFrom<s16>(other);
            case AudioFormat::Int32:
                return AddSamplesFrom<s32>(other);
            case AudioFormat::Float32:
                return AddSamplesFrom<float>(other);
            default:
                LOOM_RETURN_RESULT(Result::InvalidBufferSampleFormat);
        }
    }

    template <class T>
    Result MultiplySamplesBy(T multiplier)
    {
        if (!SampleFormatMatches<T>())
            LOOM_RETURN_RESULT(Result::BufferFormatMismatch);
        T* samples = GetData<T>();
        u32 sampleCount = 0;
        Result result = GetSampleCount(sampleCount);
        LOOM_CHECK_RESULT(result);
        for (u32 i = 0; i < sampleCount; i++)
            samples[i] *= multiplier;
        return Result::Ok;
    }

    Result GetSampleCount(u32& sampleCount) const
    {
        if (data != nullptr)
        {
            switch (format & AudioFormat::SampleFormatMask)
            {
                case AudioFormat::Int16:
                    sampleCount = size / sizeof (s16);
                    return Result::Ok;
                case AudioFormat::Int32:
                case AudioFormat::Float32:
                    sampleCount = size / sizeof (s32);
                    return Result::Ok;
                default:
                    sampleCount = 0;
                    LOOM_RETURN_RESULT(Result::InvalidBufferSampleFormat);
            }
        }
        LOOM_RETURN_RESULT(Result::NoData);
    }

    Result GetFrameCount(u32& frameCount) const
    {
        Result result = GetSampleCount(frameCount);
        LOOM_CHECK_RESULT(result);
        frameCount /= channels;
        return Result::Ok;
    }

    bool FormatMatches(const AudioBuffer& other) const
    {
        return format == other.format
            && sampleRate == other.sampleRate
            && channels == other.channels;
    }

private:
    template <class T>
    Result AddSamplesFrom(const AudioBuffer& other)
    {
        T* source = other.GetData<T>();
        T* destination = GetData<T>();
        u32 sampleCount = 0;
        Result result = GetSampleCount(sampleCount);
        LOOM_CHECK_RESULT(result);
        for (u32 i = 0; i < sampleCount; i++)
            destination[i] += source[i];
        return Result::Ok;
    }

    void DecrementRefCount()
    {
        if (_Pool != nullptr
            && _RefCount != nullptr
            && _RefCount->fetch_sub(1) == 1)
        {
            _Pool->ReleaseBuffer(*this);
            _Pool = nullptr;
            data = nullptr;
            delete _RefCount;
            _RefCount = nullptr;
        }
    }

    template <class T>
    bool SampleFormatMatches()
    {
        AudioFormat sampleFormat = format & AudioFormat::SampleFormatMask;
        AudioFormat typeFormat = GetSampleTypeAudioFormat<T>();
        return sampleFormat == typeFormat;
    }

    template <class T>
    constexpr AudioFormat GetSampleTypeAudioFormat()
    {
        if constexpr (std::is_same_v<T, s16>)
            return AudioFormat::Int16;
        else if constexpr (std::is_same_v<T, s32>)
            return AudioFormat::Int32;
        else if constexpr (std::is_same_v<T, float>)
            return AudioFormat::Float32;
        else
            return AudioFormat::Invalid;
    }

private:
    IAudioBufferProvider* _Pool;
    u32 _Capacity;
    atomic<u32>* _RefCount;
};

template <u32 BlockSize = 32>
class AudioBufferPool : IAudioBufferProvider
{
private:
    static constexpr u32 TailSentinel = U32MAX;

    class Block
    {
    public:
        u32 buffers[BlockSize];

        Block(u32 bufferSize)
            : _Data(new u8[bufferSize * BlockSize])
            , _BufferSize(bufferSize)
        {
            if (_Data == nullptr)
                LOOM_LOG_ERROR("Failed to allocate buffer block!");
        }

        Block(const Block&) = delete;
        Block& operator=(const Block&) = delete;

        ~Block()
        {
            if (_Data != nullptr)
                delete[] _Data;
        }

        u8* GetBufferData(u32 index)
        {
            if (_Data != nullptr)
                return &_Data[_BufferSize * index];
            else
                return nullptr;
        }

    private:
        u8* _Data;
        u32 _BufferSize;
    };

public:
    AudioBufferPool(u32 bufferCapacity)
        : _BufferCapacity(bufferCapacity)
        , _Head(0)
    {
        InitializeNewBlock();
    }

    virtual Result AllocateBuffer(AudioBuffer& buffer)
    {
        u32 currentIndex = TailSentinel;
        do
        {
            currentIndex = _Head;
            if (currentIndex == TailSentinel)
                ExpandPool(currentIndex);
        }
        while (!_Head.compare_exchange_strong(currentIndex, _NextBufferIndex[currentIndex]));

        u32 blockIndex = currentIndex / BlockSize;
        u32 bufferIndex = currentIndex % BlockSize;
        u8* bufferData = _Blocks[blockIndex]->GetBufferData(bufferIndex);
        buffer = AudioBuffer(&this, bufferData, _BufferCapacity);
        return Result::Ok;
    }

    virtual Result ReleaseBuffer(AudioBuffer& buffer)
    {
        u8* data = buffer.GetData();
        if (data == nullptr)
            LOOM_RETURN_RESULT(Result::Nullptr);
        Block* block = nullptr;
        u32 blockIndex = TailSentinel;
        if (!FindBlockIndex(data, block, blockIndex))
            LOOM_RETURN_RESULT(Result::BlockOutOfRange);
        u32 bufferIndex = 0;
        if(!FindBufferIndex(data, block, bufferIndex))
            LOOM_RETURN_RESULT(Result::BufferOutOfRange);
        u32 bufferPoolIndex = blockIndex * BlockSize + bufferIndex;
        u32 currentHead = TailSentinel;
        do
        {
            currentHead = _Head;
            _Blocks[blockIndex]->buffers[bufferIndex] = currentHead;
        }
        while (!_Head.compare_exchange_strong(currentHead, bufferPoolIndex));
        return Result::Ok;
    }

private:
    void ExpandPool(u32& currentIndex)
    {
        scoped_lock lock(_ExpansionMutex);
        if (_Head == TailSentinel)
        {
            u32 tailIndexFix = _Blocks.Size() * BlockSize;
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

        u32 baseIndex = BlockSize * (_Blocks.Size() - 1);

        for (u32 i = 0; i < BlockSize; ++i)
            _NextBufferIndex.PushBack(baseIndex + i + 1);

        _NextBufferIndex[_NextBufferIndex.Size() - 1] = TailSentinel;

        return block;
    }

    bool FindBlockIndex(u8* pointer, Block*& block, u32& blockIndex)
    {
        for (blockIndex = 0; blockIndex < m_Blocks.size(); ++blockIndex)
        {
            block= _Blocks[blockIndex]->get();
            u8* blockStart = block->GetBufferData(0);
            u8* blockEnd = blockStart + BlockSize * _BufferCapacity;
            if (pointer >= blockStart && pointer < blockEnd)
                return true;
        }
        block = nullptr;
        blockIndex = TailSentinel;
        return false;
    }

    bool FindBufferIndex(u8* pointer, Block* block, u32& bufferIndex)
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
    mutex _ExpansionMutex;
    u32 _BufferCapacity;
    atomic<u32> _Head;
    vector<unique_ptr<Block>> _Blocks;
    vector<u32> _NextBufferIndex;
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
    string name;
    u32 channels;
    u32 sampleRate;
    bool defaultDevice;
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
    virtual Result EnumerateDevices(u32& deviceCount, const AudioDeviceDescription*& devices) = 0;
    virtual Result SelectPlaybackDevice(const AudioDeviceDescription* device) = 0;
    virtual Result SelectDefaultPlaybackDevice() = 0;
    virtual Result Start() = 0;
    virtual Result Stop() = 0;
};

class AudioDeviceManagerStub : public IAudioDeviceManager
{
public:
    static AudioDeviceManagerStub& GetInstance()
    {
        static AudioDeviceManagerStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_BAD_RESULT(Result::CallingStub);
        return "IAudioDeviceManager stub";
    }

    Result RegisterPlaybackCallback(AudioDevicePlaybackCallback, void*) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result EnumerateDevices(u32&, const AudioDeviceDescription*&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result SelectPlaybackDevice(const AudioDeviceDescription*) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result SelectDefaultPlaybackDevice() final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result Start() final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    Result Stop() final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
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

    virtual Result Seek(float seconds) = 0;
    virtual Result Seek(u32 frame) = 0;
    virtual Result GetFramePosition(u32& frame) = 0;
    virtual Result GetTimePosition(float& seconds) = 0;
    virtual Result Read(u32 frameCountRequested, AudioBuffer*& buffer) = 0;
};

class IAudioDecoder : public IAudioService
{
public:
    AudioServiceType GetType() const final override
    {
        return AudioServiceType::Decoder;
    }
    virtual Result CreateSampleBuffer(const char* filePath, AudioBuffer*& destination) = 0;
    virtual Result OpenFile(const char* filePath, IAudioFile*& audioFile) = 0;
};

class AudioDecoderStub : public IAudioDecoder
{
public:
    static AudioDecoderStub& GetInstance()
    {
        static AudioDecoderStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_BAD_RESULT(Result::CallingStub);
        return "IAudioDecoder stub";
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
class IAudioResampler : public IAudioService
{
public:
    AudioServiceType GetType() const final override
    {
        return AudioServiceType::Resampler;
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
        LOOM_LOG_BAD_RESULT(Result::CallingStub);
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
    static AudioChannelRemapperStub& GetInstance()
    {
        static AudioChannelRemapperStub instance;
        return instance;
    }

    const char* GetName() const final override
    {
        LOOM_LOG_BAD_RESULT(Result::CallingStub);
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

    Result AddInput(AudioNode* node)
    {
        if (node == nullptr)
            LOOM_RETURN_RESULT(Result::Nullptr);
        if (_InputNodes.insert(node).second)
            return Result::Ok;
        else
            LOOM_RETURN_RESULT(Result::UnableToConnect);
    }

    Result AddOutput(AudioNode* node)
    {
        if (node ==  nullptr)
            LOOM_RETURN_RESULT(Result::Nullptr);
        if (_OutputNodes.insert(node).second)
            return Result::Ok;
        else
            LOOM_RETURN_RESULT(Result::UnableToConnect);
    }

    Result DisconnectNode(AudioNode* node)
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
            AudioNode* node = *itr;
            node->Execute(destinationBuffer);
            if (itr == _InputNodes.begin())
            {
                _Buffer = destinationBuffer;
            }
            else
            {
                Result result = _Buffer.MixInBuffer(destinationBuffer);
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
    set<AudioNode*> _InputNodes;
    set<AudioNode*> _OutputNodes;

    IAudioSystem& _System;
    bool _Visited;

};


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

    bool NodeWasVisited(AudioNode* node)
    {
        return node->_Visited;
    }

    set<AudioNode*>& GetNodeOutputNodes(AudioNode* node)
    {
        return node->_OutputNodes;
    }

    set<AudioNode*>& GetNodeInputNodes(AudioNode* node)
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
        LOOM_LOG_BAD_RESULT(Result::CallingStub);
        return "IAudioGraph stub";
    }

    Result Execute(AudioBuffer&) final override
    {
        LOOM_RETURN_RESULT(Result::CallingStub);
    }

    AudioGraphState GetState() const final override
    {
        LOOM_LOG_BAD_RESULT(Result::CallingStub);
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
    virtual IAudioDecoder& GetDecoderInterface() const = 0;
    virtual IAudioDeviceManager& GetDeviceManagerInterface() const = 0;
    virtual IAudioResampler& GetResamplerInterface() const = 0;
    virtual IAudioChannelRemapper& GetChannelRemapperInterface() const = 0;
    virtual IAudioBufferProvider& GetBufferProviderInterface() const = 0;
};

//
// Audio specific helpers
//
using Decibel = float;

Decibel LinearToDecibel(float linearVolume)
{
    if (linearVolume <= 0.0f)
        return -80.0f;
    return 20.0f * std::log10(linearVolume);
}

float DecibelToLinear(float decibel)
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
    T* CreateNode(Args&&... args)
    {
        static_assert(std::is_base_of_v<AudioNode, T>, "T must be derived from AudioNode");

        T* node = new T(std::forward<Args>(args)...);
        if (node != nullptr)
        {
            scoped_lock lock(_UpdateNodesMutex);
            AudioNode* nodeBase = static_cast<AudioNode*>(node);
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
                delete node;
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
        if (_NodesToRemove.insert(static_cast<AudioNode*>(node)))
            return Result::Ok;
        else
            LOOM_RETURN_RESULT(Result::CannotFind);
    }

    struct NodeConnection
    {
        template <class T, class U>
        NodeConnection(T* sourceNode, U* destinationNode)
        {
            static_assert(std::is_base_of_v<AudioNode, T>, "Source node must be derived from AudioNode");
            static_assert(std::is_base_of_v<AudioNode, U>, "Destination node must be derived from AudioNode");
            this->sourceNode = static_cast<AudioNode*>(sourceNode);
            this->destinationNode = static_cast<AudioNode*>(destinationNode);
        }
        AudioNode* sourceNode;
        AudioNode* destinationNode;
    };

    template <class T, class U>
    Result Connect(T* sourceNode, U* destinationNode)
    {
        static_assert(std::is_base_of_v<AudioNode, T>, "Source node must be derived from AudioNode");
        static_assert(std::is_base_of_v<AudioNode, U>, "Destination node must be derived from AudioNode");

        _NodesToConnect.emplace_back(sourceNode, destinationNode);
        return Result::Ok;
    }

    template <class T, class... NodeTypes>
    Result Chain(T* node, NodeTypes*... followingNodes)
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
    Result ChainNodesImpl(T* leftNode, U* rightNode, NodeTypes*... followingNodes)
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
    Result ConnectNodesImpl(T* first)
    {
        return Result::Ok;
    }

    Result UpdateNodes()
    {
        scoped_lock lock(_UpdateNodesMutex);
        for (AudioNode* node : _NodesToRemove)
        {
            _Nodes.erase(node);
            node->Shutdown();
            delete node;
        }
        _NodesToRemove.clear();
        for (AudioNode* node : _NodesToAdd)
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
        set<AudioNode*> outputNodes;
        ClearNodesVisitedFlag();
        for (AudioNode* node : _Nodes)
            SearchOutputNodes(node, outputNodes);
        bool nodesContainsOutputNode = _OutputNode != nullptr && _Nodes.find(_OutputNode) != _Nodes.end();
        if (outputNodes.size() == 1)
        {
            AudioNode* node = *outputNodes.begin();
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
                _OutputNode = static_cast<AudioNode*>(new MixingNode(_System));
                _Nodes.insert(_OutputNode);
            }
            for (AudioNode* node : outputNodes)
                node->AddOutput(_OutputNode);
        }
        return Result::Ok;
    }

    void SearchOutputNodes(AudioNode* node, set<AudioNode*>& outputNodesSearchResult)
    {
        if (NodeWasVisited(node))
            return;
        VisitNode(node);
        set<AudioNode*>& visitedNodeOutputNodes = GetNodeOutputNodes(node);
        if (visitedNodeOutputNodes.empty())
        {
            outputNodesSearchResult.insert(node);
        }
        else
        {
            for (AudioNode* outputNode : visitedNodeOutputNodes)
                SearchOutputNodes(outputNode, outputNodesSearchResult);
        }
    }

    void ClearNodesVisitedFlag()
    {
        for (AudioNode* node : _Nodes)
            ClearNodeVisit(node);
    }

private:
    IAudioSystem& _System;
    atomic<AudioGraphState> _State;
    AudioNode* _OutputNode;
    set<AudioNode*> _Nodes;
    set<AudioNode*> _NodesToAdd;
    set<AudioNode*> _NodesToRemove;
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
    AudioSource* CreateAudioSource(const AudioAsset* audioAsset, const AudioNode* inputNode);
    Result DestroyAudioSource(const AudioSource* audioSource);

    const AudioSystemConfig& GetConfig() const override
    {
        return _Config;
    }

    IAudioGraph& GetGraphInterface()
    {
        return *static_cast<IAudioGraph*>(&_Graph);
    }

    IAudioDecoder& GetDecoderInterface() const override
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

    Result SetService(IAudioService* service)
    {
        if (service == nullptr)
            LOOM_RETURN_RESULT(Result::Nullptr);

        switch(service->GetType())
        {
            case AudioServiceType::Decoder:
                if (_Decoder != nullptr)
                    _Decoder->Shutdown();
                _Decoder.reset(static_cast<IAudioDecoder*>(service));
                return _Decoder->Initialize();

            case AudioServiceType::BufferProvider:
                if (_BufferProvider != nullptr)
                    _BufferProvider->Shutdown();
                _BufferProvider.reset(static_cast<IAudioBufferProvider*>(service));
                return _BufferProvider->Initialize();

            case AudioServiceType::Resampler:
                if (_Resampler != nullptr)
                    _Resampler->Shutdown();
                _Resampler.reset(static_cast<IAudioResampler*>(service));
                return _Resampler->Initialize();

            case AudioServiceType::ChannelRemapper:
                if (_ChannelRemapper != nullptr)
                    _ChannelRemapper->Shutdown();
                _ChannelRemapper.reset(static_cast<IAudioChannelRemapper*>(service));
                return _ChannelRemapper->Initialize();

            case AudioServiceType::DeviceManager:
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
    unique_ptr<IAudioDecoder> _Decoder;
    unique_ptr<IAudioDeviceManager> _DeviceManager;
    unique_ptr<IAudioResampler> _Resampler;
    unique_ptr<IAudioChannelRemapper> _ChannelRemapper;
    unique_ptr<IAudioBufferProvider> _BufferProvider;
};

} // namespace Loom
