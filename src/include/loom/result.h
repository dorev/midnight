#pragma once

#include "loom/types.h"

namespace Loom
{

enum class Result : u32
{
    Ok = 0,
    Nullptr,
    InvalidPosition,
    UnsupportedFormat,
    InvalidFile,
    OutputDeviceDisconnected,
    WrongParameterType,
    CannotFind,
    NotYetImplemented,
    UnexpectedState,
    EndOfFile,
    ExceedingLimits,
    NotReady,
    Busy,
    UnableToConnect,
    UnableToAddNode,
    MissingOutputNode,
    OutOfRange,
    BlockOutOfRange,
    BufferOutOfRange,
    FailedAllocation,
    InvalidEnumValue,
    CallingStub,
    ServiceUnavailable,
    BufferCapacityMismatch,
    BufferFormatMismatch,
    NoData,
    InvalidBufferSampleFormat,
    Unknown = UINT32_MAX
};

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