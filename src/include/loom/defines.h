#pragma once

namespace Loom
{

#if defined(_MSC_VER)
    #define LOOM_DEBUG_BREAK() __debugbreak()
    #define LOOM_FUNCTION __FUNCSIG__
#elif defined(__GNUC__) || defined(__clang__)
    #define LOOM_DEBUG_BREAK() __builtin_trap()
    #define LOOM_FUNCTION __PRETTY_FUNCTION__
#else
    #define LOOM_DEBUG_BREAK() assert(false)
    #define LOOM_FUNCTION __FUNCTION__
#endif

#define LOOM_LOG(format, ...) printf(format "\n", ##__VA_ARGS__);
#define LOOM_LOG_WARNING(format, ...) printf("[WARNING] {%s}" format "\n", LOOM_FUNCTION, ##__VA_ARGS__);
#define LOOM_LOG_ERROR(format, ...) printf("[ERROR] {%s}" format " [%s l.%d]\n", LOOM_FUNCTION, ##__VA_ARGS__, __FILE__, __LINE__);

#define LOOM_LOG_RESULT(result) LOOM_LOG_WARNING("Returned %s (%d).", ResultToString(result), static_cast<u32>(result))
#define LOOM_RETURN_RESULT(result) { LOOM_LOG_RESULT(result); return result; }
#define LOOM_CHECK_RESULT(result) if (result != Result::Ok) { LOOM_RETURN_RESULT(result); }

#define LOOM_DEBUG_ASSERT(condition, format, ...) \
if (!(condition)) \
{ \
    LOOM_LOG("[ASSERT] " format "\n", ##__VA_ARGS__); \
    LOOM_DEBUG_BREAK(); \
}

#define LOOM_UNUSED(variable) (void)(variable)

// Flags declaration helper
#define LOOM_DECLARE_FLAG_ENUM(EnumName, UnderlyingType) \
enum class EnumName : UnderlyingType; \
constexpr EnumName operator|(EnumName a, EnumName b) \
{ \
    return static_cast<EnumName>(static_cast<UnderlyingType>(a) | static_cast<UnderlyingType>(b)); \
} \
constexpr EnumName operator&(EnumName a, EnumName b) \
{ \
    return static_cast<EnumName>(static_cast<UnderlyingType>(a) & static_cast<UnderlyingType>(b)); \
} \
constexpr EnumName operator<<(EnumName a, EnumName b) \
{ \
    return static_cast<EnumName>(static_cast<UnderlyingType>(a) << static_cast<UnderlyingType>(b)); \
} \
inline EnumName& operator|=(EnumName& a, EnumName b) \
{ \
    return a = a | b; \
} \
inline EnumName& operator&=(EnumName& a, EnumName b) \
{ \
    return a = a & b; \
} \
enum class EnumName : UnderlyingType

#define LOOM_FLAG(name, shift) name = 1 << shift

} // namespace Loom
