#include "types.h"

namespace Midnight
{
    enum Error
    {
        Ok,
        Nullptr,
        InvalidPosition,
    };

    class AudioManager
    {
    };

    struct OutputDeviceDescription
    {
        String name;
        U32 channels;
        U32 sampleRate;
        bool defaultDevice;
    };

    using AudioOutputCallback = void(*)(U8* destination, U32 channels, U32 frames);
    class IAudioOutput
    {
        Error RegisterCallback(AudioOutputCallback callback);
        Error EnumerateDevices(U32& deviceCount, OutputDeviceDescription* devices);
        Error SelectDevice(const OutputDeviceDescription& device);
        Error Start();
        Error Stop();
    };

    class AudioData
    {
        UniquePtr<U8> samplesData;
        U32 channels;
        U32 sampleRate;
        U32 samples;
    };

    class AudioInstance
    {
        SharedPtr<const AudioData> audioData;
        U32 id;
        U32 position;

        Error Play();
        Error Pause();
        Error Stop();
        Error Seek(U32 position);
    };

    class AudioParameter
    {
        String name;
        U32 id;
        SharedPtr<MixerEffectNode> mixerEffectNode;
    };

    class Mixer
    {
        Vector<SharedPtr<MixerInputNode>> inputs;
        Vector<SharedPtr<MixerOutputNode>> outputs;

        SharedPtr<MixerInputNode> CreateInput();
    };

    class MixerEffectNode
    {
    };

    class MixerInputNode
    {
    };
    
    class MixerOutputNode
    {
    };
}