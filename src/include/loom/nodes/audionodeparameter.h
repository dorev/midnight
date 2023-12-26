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

    const char* GetName() const
    {
        return _Name.c_str();
    }

    AudioNodeParameterType GetType() const
    {
        return _Type;
    }

    template <class T>
    Result SetValue(const T& value)
    {
        if (GetNodeParameterType<T>() == _Type)
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
        if (GetNodeParameterType<T>() == _Type)
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

template <class T>
constexpr AudioNodeParameterType GetNodeParameterType()
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

} // namespace Loom
