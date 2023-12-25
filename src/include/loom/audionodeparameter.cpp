#include "loom/audionodeparameter.h"

namespace Loom
{

AudioNodeParameter::AudioNodeParameter(const char* name, AudioNodeParameterType type, ValueType initialValue, bool hasLimits, ValueType min, ValueType max)
    : _Name(name)
    , _Type(type)
    , _Value(initialValue)
    , _HasLimits(hasLimits)
    , _Min(min)
    , _Max(max)
{
}

template <class T>
Result AudioNodeParameter::SetValue(const T& value)
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
Result AudioNodeParameter::GetValue(T& value) const
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

template <class T>
constexpr AudioNodeParameterType AudioNodeParameter::GetParameterType()
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
