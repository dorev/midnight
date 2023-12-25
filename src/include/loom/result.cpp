#include "loom/result.h"

namespace Loom
{

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

} // namespace Loom