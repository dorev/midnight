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

} // namespace Loom
