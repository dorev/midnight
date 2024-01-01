#include "loom/audiobufferpool.h"
#include "loom/audiobuffer.h"

namespace Loom
{

AudioBufferPool::AudioBufferPool(IAudioSystem& system, AudioFormat audioFormat, u32 bufferCapacity)
    : IAudioBufferProvider(system)
    , _AudioFormat(audioFormat)
    , _BufferCapacity(bufferCapacity)
    , _Head(0)
{
    InitializeNewBlock();
}

const char* AudioBufferPool::GetName() const
{
    return "AudioBufferPool";
}

Result AudioBufferPool::AllocateBuffer(AudioBuffer& buffer)
{
    u32 currentIndex = TailSentinel;
    do
    {
        currentIndex = _Head.load(std::memory_order_relaxed);
        if (currentIndex == TailSentinel)
            ExpandPool(currentIndex);
    }
    while (!_Head.compare_exchange_strong(currentIndex, _NextBufferIndex[currentIndex]));

    u32 blockIndex = currentIndex / BlockSize;
    u32 bufferIndex = currentIndex % BlockSize;
    u8* bufferData = _Blocks[blockIndex]->GetBufferData(bufferIndex);
    buffer = AudioBuffer(GetSystemInterface(), _AudioFormat, bufferData, _BufferCapacity);
    return Result::Ok;
}

Result AudioBufferPool::ReleaseBuffer(AudioBuffer& buffer)
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
        currentHead = _Head.load(std::memory_order_relaxed);
        _Blocks[blockIndex]->buffers[bufferIndex].store(currentHead, std::memory_order_seq_cst);
    }
    while (!_Head.compare_exchange_strong(currentHead, bufferPoolIndex));
    return Result::Ok;
}

void AudioBufferPool::ExpandPool(u32& currentIndex)
{
    scoped_lock lock(_ExpansionMutex);
    if (_Head == TailSentinel)
    {
        u32 tailIndexFix = static_cast<u32>(_Blocks.size()) * BlockSize;
        _NextBufferIndex[static_cast<u32>(_NextBufferIndex.size()) - 1] = tailIndexFix;
        _Head = tailIndexFix;
        currentIndex = tailIndexFix;
        InitializeNewBlock();
    }
}

typename AudioBufferPool::Block* AudioBufferPool::InitializeNewBlock()
{
    Block* block = new Block(_BufferCapacity);
    _Blocks.emplace_back(block);
    u32 baseIndex = BlockSize * (static_cast<u32>(_Blocks.size()) - 1);
    for (u32 i = 0; i < BlockSize; ++i)
        _NextBufferIndex.push_back(baseIndex + i + 1);
    _NextBufferIndex[_NextBufferIndex.size() - 1] = TailSentinel;
    return block;
}

bool AudioBufferPool::FindBlockIndex(u8* pointer, Block*& block, u32& blockIndex)
{
    for (blockIndex = 0; blockIndex < static_cast<u32>(_Blocks.size()); ++blockIndex)
    {
        block= _Blocks[blockIndex].get();
        u8* blockStart = block->GetBufferData(0);
        u8* blockEnd = blockStart + BlockSize * _BufferCapacity;
        if (pointer >= blockStart && pointer < blockEnd)
            return true;
    }
    block = nullptr;
    blockIndex = TailSentinel;
    return false;
}

bool AudioBufferPool::FindBufferIndex(u8* pointer, Block* block, u32& bufferIndex)
{
    for (bufferIndex = 0; bufferIndex < BlockSize; ++bufferIndex)
    {
        if (block->GetBufferData(bufferIndex) == pointer)
            return true;
    }
    bufferIndex = TailSentinel;
    return false;
}

AudioBufferPool::Block::Block(u32 bufferSize)
    : _Data(new u8[bufferSize * BlockSize])
    , _BufferSize(bufferSize)
{
    if (_Data == nullptr)
        LOOM_LOG_ERROR("Failed to allocate buffer block!");
}

AudioBufferPool::Block::~Block()
{
    if (_Data != nullptr)
        delete[] _Data;
}

u8* AudioBufferPool::Block::GetBufferData(u32 index)
{
    if (_Data != nullptr)
        return &_Data[_BufferSize * index];
    else
        return nullptr;
}

} // namespace Loom
