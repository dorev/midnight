#pragma once

#include "loom/defines.h"
#include "loom/types.h"

#include "loom/result.h"

namespace Loom
{

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

    AudioNodeParameter(const char* name, AudioNodeParameterType type, ValueType initialValue = 0, bool hasLimits = false, ValueType min = 0, ValueType max = 0);

    template <class T>
    Result SetValue(const T& value);

    template <class T>
    Result GetValue(T& value) const;

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
    static constexpr AudioNodeParameterType GetParameterType();
};


} // namespace Loom
