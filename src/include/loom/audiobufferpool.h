#pragma once

#include "defines.h"
#include "types.h"

namespace Loom
{

class AudioBuffer;

template <u32 BlockSize = 32>
class AudioBufferPool : IAudioBufferProvider
{
public:
    AudioBufferPool(u32 bufferCapacity);
    virtual Result AllocateBuffer(AudioBuffer& buffer);
    virtual Result ReleaseBuffer(AudioBuffer& buffer);

private:
    static constexpr u32 TailSentinel = UINT32_MAX;

    class Block
    {
    public:
        u32 buffers[BlockSize];

        Block(u32 bufferSize);

        Block(const Block&) = delete;
        Block& operator=(const Block&) = delete;

        ~Block();

        u8* GetBufferData(u32 index);

    private:
        u8* _Data;
        u32 _BufferSize;
    };

    void ExpandPool(u32& currentIndex);
    Block* InitializeNewBlock();
    bool FindBlockIndex(u8* pointer, Block*& block, u32& blockIndex);
    bool FindBufferIndex(u8* pointer, Block* block, u32& bufferIndex);

private:
    mutex _ExpansionMutex;
    u32 _BufferCapacity;
    atomic<u32> _Head;
    vector<unique_ptr<Block>> _Blocks;
    vector<u32> _NextBufferIndex;
};


} // namespace Loom
