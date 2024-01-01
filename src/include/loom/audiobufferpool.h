#pragma once

#include "loom/interfaces/iaudiobufferprovider.h"
#include "loom/audioformat.h"

namespace Loom
{

class AudioBuffer;
class IAudioSystem;

class AudioBufferPool : public IAudioBufferProvider
{
public:
    static constexpr u32 BlockSize = 32;

    AudioBufferPool(IAudioSystem& system, AudioFormat audioFormat, u32 bufferCapacity);
    const char* GetName() const override;
    Result AllocateBuffer(AudioBuffer& buffer) override;
    Result ReleaseBuffer(AudioBuffer& buffer) override;

private:
    static constexpr u32 TailSentinel = UINT32_MAX;

    class Block
    {
    public:
        atomic<u32> buffers[BlockSize];

    public:
        Block(u32 bufferSize);
        ~Block();
        Block(const Block&) = delete;
        Block& operator=(const Block&) = delete;

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
    AudioFormat _AudioFormat;
    atomic<u32> _Head;
    vector<unique_ptr<Block>> _Blocks;
    vector<u32> _NextBufferIndex;
};


} // namespace Loom
