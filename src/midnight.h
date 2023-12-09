#include "types.h"

namespace Midnight
{
    enum class Error : U32
    {
        Ok = 0,
        Nullptr,
        InvalidPosition,
        UnsupportedFormat,
        InvalidFile,
        OutputDeviceDisconnected,
        NotYetImplemented,
        Unknown = U32MAX
    };

    class AudioManager
    {
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

    class IAudioDeviceManager
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
        virtual Error RegisterPlaybackCallback(AudioPlaybackCallback callback) = 0;
        virtual Error EnumerateDevices(U32& deviceCount, const AudioDeviceDescription* devices) = 0;
        virtual Error SelectPlaybackDevice(const AudioDeviceDescription& device) = 0;
        virtual Error Start() = 0;
        virtual Error Stop() = 0;
    };

    class IAudioDecoder
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
        virtual Error DecodeFile(const String& filePath, AudioBuffer& audioData) = 0;
    };

    class IAudioResampler
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
        virtual Error Resample(const AudioBuffer& input, AudioBuffer& output) = 0;
    };

    class IAudioChannelRemapper
    {
    public:
        virtual Error Initialize() = 0;
        virtual Error Shutdown() = 0;
        virtual Error Remap(const AudioBuffer& input, AudioBuffer& output) = 0;
    };

    struct AudioBuffer
    {
        SharedPtr<U8[]> samplesData;
        SharedPtr<U8[]> metadata;
        U32 channels;
        U32 sampleRate;
        U32 samples;
    };

    class AudioSource
    {
    public:
        Error Play(F32 fadeIn = 0.0f);
        Error Pause(F32 fadeOut = 0.05f);
        Error Stop(F32 fadeOut = 0.05f);
        Error Seek(U32 position);

    private:
        U32 _Id;
        U32 _Position;
        SharedPtr<const AudioBuffer> _AudioBuffer;
        SharedPtr<MixerInputNode> _MixerInput;
    };

    enum class AudioParameterType
    {
        Unsigned,
        Signed,
        Float,
        Boolean
    };

    class AudioParameter
    {
    private:
        AudioParameterType _Type;
        Variant<U32, S32, F32, Bool> _Value;
        SharedPtr<MixerEffectNodeBase> _MixerEffectNode;
        String _Name;
    };

    class Mixer
    {
    public:
        SharedPtr<MixerInputNode> CreateInputNode();

    private:
        Vector<SharedPtr<MixerInputNode>> _Inputs;
        Vector<SharedPtr<MixerOutputNode>> _Outputs;
        Vector<SharedPtr<MixerEffectNodeBase>> _Nodes;
    };

    class MixerNode
    {
    private:
        friend class Mixer;

    public:
        Error ConnectPrevious(SharedPtr<MixerNode> node);
        Error ConnectNext(SharedPtr<MixerNode> node);
        Bool ReadyToProcess() const;

        enum MixerNodeState
        {
            NeedsProcessing,
            Processing,
            DoneProcessing
        };

        MixerNodeState GetState() const { return _State; }

    protected:
        virtual Error Process(const AudioBuffer& input, AudioBuffer& output) = 0;

    private:
        SharedPtr<MixerNode> _Previous;
        SharedPtr<MixerNode> _Next;
        MixerNodeState _State;
    };

    class MixerEffectNodeBase : public MixerNode
    {
     protected:
        Vector<SharedPtr<AudioParameter>> _Parameters;
    };

    class MixerInputNode : public MixerNode
    {
    private:
        using MixerNode::ConnectPrevious;
    };
    
    class MixerOutputNode : public MixerNode
    {
    private:
        using MixerNode::ConnectNext;
    };
}