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

const char* ResultToString(Result result);

} // namespace Loom