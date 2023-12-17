#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <utility>
#include <type_traits>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <variant>
#include <algorithm>
#include <memory>

namespace Midnight
{

///////////////////////////////////////////////////////////////////////////////
// Primitive type aliases
///////////////////////////////////////////////////////////////////////////////

using Bool = bool;
using Char = char;
using WChar = wchar_t;
using U8 = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;
using S8 = int8_t;
using S16 = int16_t;
using S32 = int32_t;
using S64 = int64_t;
using F32 = float;
using F64 = double;
constexpr U8 U8MAX = UINT8_MAX;
constexpr U16 U16MAX = UINT16_MAX;
constexpr U32 U32MAX = UINT32_MAX;
constexpr U64 U64MAX = UINT64_MAX;
constexpr S8 S8MAX = INT8_MAX;
constexpr S8 S8MIN = INT8_MIN;
constexpr S16 S16MAX = INT16_MAX;
constexpr S16 S16MIN = INT16_MIN;
constexpr S32 I32MAX = INT32_MAX;
constexpr S32 S32MIN = INT32_MIN;
constexpr S64 S64MAX = INT64_MAX;
constexpr S64 S64MIN = INT64_MIN;
constexpr F32 F32MAX = FLT_MAX;
constexpr F32 F32MIN = FLT_MIN;
constexpr F64 F64MAX = DBL_MAX;
constexpr F64 F64MIN = DBL_MIN;

///////////////////////////////////////////////////////////////////////////////
// Traits
///////////////////////////////////////////////////////////////////////////////

template <class T, class U>
constexpr bool IsSame = std::is_same<T, U>::value;

template <class T>
constexpr bool IsPointer = std::is_pointer<T>::value;

template <class T>
constexpr bool IsFloatingPoint = std::is_floating_point<T>::value;

template <class T>
constexpr bool IsIntegral = std::is_integral<T>::value>;

template <class T>
constexpr bool IsArray = std::is_array<T>::value;

template <class T>
using Decay = typename std::decay<T>::type;

template <class T>
using RemoveExtent = typename std::remove_extent<T>::type;

template <bool B, class T = void>
using EnableIf = typename std::enable_if<B, T>::type;

template <class Base, class Derived>
constexpr bool IsDerivedFrom = std::is_base_of<Base, Derived>::value;

///////////////////////////////////////////////////////////////////////////////
// Containers
///////////////////////////////////////////////////////////////////////////////

//
// String
//

using StringInternal = std::string;

class String
{
private:
    StringInternal _String;

public:
    template <typename T>
    static String ToString(T number)
    {
        std::ostringstream stream;
        stream << number;
        return stream.str();
    }

public:
    template <class... Args>
    String(Args... args)
        : _String(std::forward<Args>(args)...)
    {
    }

    Char& operator[](U32 index)
    {
        return _String[index];
    }

    const Char& operator[](U32 index) const
    {
        return _String[index];
    }

    const Char* CStr() const
    {
        return _String.c_str();
    }

    void Clear()
    {
        _String.clear();
    }

    U32 Size() const
    {
        return static_cast<U32>(_String.size());
    }
    
    String& operator=(const String& other)
    {
        if (this != &other)
        {
            _String = other._String;
        }
        return *this;
    }

    bool operator==(const String& other) const
    {
        return _String == other._String;
    }

    String& operator+=(const String& other)
    {
        _String += other._String;
        return *this;
    }

    String Substring(U32 start, U32 length) const
    {
        return _String.substr(start, length);
    }

    U32 Find(const String& substr) const
    {
        return static_cast<U32>(_String.find(substr._String));
    }

    void Replace(const String& from, const String& to)
    {
        size_t startPos = _String.find(from._String);
        while (startPos != std::string::npos)
        {
            _String.replace(startPos, from.Size(), to._String);
            startPos = _String.find(from._String, startPos + to.Size());
        }
    }

    void ToUpper()
    {
        std::transform(_String.begin(), _String.end(), _String.begin(),
            [](unsigned char c){ return std::toupper(c); });
    }

    void ToLower()
    {
        std::transform(_String.begin(), _String.end(), _String.begin(),
            [](unsigned char c){ return std::tolower(c); });
    }
};

//
// Vector
//

template <class T>
using VectorInternal = std::vector<T>;

template <class T>
class Vector
{
private:
    VectorInternal<T> _Vector;

public:
    template <class... Args>
    Vector(Args... args)
        : _Vector(std::forward<Args>(args)...)
    {
    }

    T& operator[](U32 index)
    {
        return _Vector[index];
    }

    const T& operator[](U32 index) const
    {
        return _Vector[index];
    }

    void PushBack(const T& item)
    {
        _Vector.push_back(item);
    }

    void Clear()
    {
        _Vector.clear();
    }

    void Clean(U32 size = 0)
    {
        _Vector.clear();
        _Vector.shrink_to_fit();
        if (size != 0)
        {
            _Vector.reserve(size);
        }
    }

    U32 Erase(const T& item)
    {
        U32 removed = 0;
        for (auto itr = _Vector.rbegin(); itr != _Vector.rend();)
        {
            if (*itr == item)
            {
                auto forwardItr = itr.base();
                // Adjust to point to the current element
                --forwardItr;

                auto nextItr = _Vector.erase(forwardItr);
                itr = std::reverse_iterator<decltype(nextItr)>(nextItr);
                removed++;
            }
            else
            {
                ++itr;
            }
        }
        return removed;
    }

    template <class U = T, class = EnableIf<!IsPointer<U>>>
    Bool Contains(const U& item) const
    {
        for (const U& content : _Vector)
        {
            if (content == item)
            {
                return true;
            }
        }
        return false;
    }

    template <class U = T, class = EnableIf<IsPointer<U>>>
    Bool Contains(U item) const
    {
        for (const U& content : _Vector)
        {
            if (content == item)
            {
                return true;
            }
        }
        return false;
    }

    U32 Size() const
    {
        return static_cast<U32>(_Vector.size());
    }

    Bool Empty() const
    {
        return _Vector.empty();
    }

    Bool NotEmpty() const
    {
        return !Empty();
    }

    void Resize(U32 size)
    {
        _Vector.resize(size);
    }

    void Reserve(U32 size)
    {
        _Vector.reserve(size);
    }

    //
    // `for range` facilities
    //

    using iterator = typename std::vector<T>::iterator;

    auto begin()
    {
        return _Vector.begin();
    }

    auto end()
    {
        return _Vector.end();
    }

    const auto begin() const
    {
        return _Vector.cbegin();
    }

    const auto end() const
    {
        return _Vector.cend();
    }
};

//
// Set
//

template <class T>
using SetInternal = std::set<T>;

template <class T>
class Set
{
private:
    SetInternal<T> _Set;

public:
    template <class... Args>
    Set(Args... args)
        : _Set(std::forward<Args>(args)...)
    {
    }

    Bool Insert(const T& value)
    {
        return _Set.insert(value).second;
    }

    Bool Contains(const T& value) const
    {
        return _Set.find(value) != _Set.end();
    }

    U32 Size() const
    {
        return static_cast<U32>(_Set.size());
    }

    Bool Empty() const
    {
        return _Set.empty();
    }

    Bool Remove(const T& value)
    {
        return _Set.erase(value) > 0;
    }

    void Clear()
    {
        _Set.clear();
    }

    Bool operator==(const Set& other) const
    {
        return _Set == other._Set;
    }

    Bool operator!=(const Set& other) const
    {
        return _Set != other._Set;
    }

    //
    // `for range` iteration facilities
    //
    auto begin()
    {
        return _Set.begin();
    }

    auto end()
    {
        return _Set.end();
    }

    const auto begin() const
    {
        return _Set.cbegin();
    }

    const auto end() const
    {
        return _Set.cend();
    }
};

//
// Map
//

template <class K, class V>
using MapInternal = std::map<K, V>;

template <class K, class V>
class Map
{
private:
    MapInternal<K, V> _Map;

public:
    template <class... Args>
    Map(Args... args)
        : _Map(std::forward<Args>(args)...)
    {
    }

    V& operator[](const K& key)
    {
        return _Map[key];
    }

    Bool Find(const K& key, V* outValuePtr)
    {
        auto itr = _Map.find(key);
        if (itr != _Map.end())
        {
            *outValuePtr = &itr->second;
            return true;
        }
        return false;
    }

    Bool Contains (const K& key) const
    {
        return _Map.find(key) != _Map.end();
    }

    U32 Size() const
    {
        return static_cast<U32>(_Map.size());
    }

    Bool Empty() const
    {
        return _Map.empty();
    }

    Bool Remove(const K& key)
    {
        return _Map.erase(key) > 0;
    }

    Bool operator==(const Map& other) const
    {
        return _Map == other._Map;
    }

    //
    // `for range` facilities
    //

    using iterator = typename std::map<K, V>::iterator;

    auto begin()
    {
        return _Map.begin();
    }

    auto end()
    {
        return _Map.end();
    }

    const auto begin() const
    {
        return _Map.cbegin();
    }

    const auto end() const
    {
        return _Map.cend();
    }
};

//
// List
//

template <class T>
using ListInternal = std::list<T>;

template <class T>
class List
{
    ListInternal<T> _List;

public:
    template <class... Args>
    List(Args... args)
        : _List(std::forward<Args>(args)...)
    {
    }

    void Clear()
    {
        _List.clear();
    }

    U32 Size() const
    {
        return static_cast<U32>(_List.size());
    }

    Bool Empty() const
    {
        return _List.empty();
    }

    Bool NotEmpty() const
    {
        return !Empty();
    }

    void PushBack(const T& value)
    {
        _List.push_back(value);
    }

    void PushFront(const T& value)
    {
        _List.push_front(value);
    }

    void PopFront()
    {
        _List.pop_front();
    }

    void PopBack()
    {
        _List.pop_back();
    }

    T& Front()
    {
        return _List.front();
    }

    T& Back()
    {
        return _List.back();
    }

    const T& Front() const
    {
        return _List.front();
    }

    const T& Back() const
    {
        return _List.back();
    }

    Bool InsertBefore(const T& item, const T& pivot)
    {
        for (auto itr = _List.begin(); itr != _List.end(); ++itr)
        {
            if (*itr == pivot)
            {
                _List.insert(itr, item);
                return true;
            }
        }
        return false;
    }

    Bool InsertAfter(const T& item, const T& pivot)
    {
        for (auto itr = _List.begin(); itr != _List.end(); ++itr)
        {
            if (*itr == pivot)
            {
                // If the iterator is not at the end of the list, advance it one position before inserting.
                // Otherwise, just insert at the current position which would be equivalent to inserting at the end.
                if (std::next(itr) != _List.end())
                {
                    ++itr;
                }
                _List.insert(itr, item);
                return true;
            }
        }
        return false;
    }

    Bool Remove(const T& item)
    {
        for (auto itr = _List.begin(); itr != _List.end(); ++itr)
        {
            if (*itr == item)
            {
                _List.erase(itr);
                return true;
            }
        }
        return false;
    }

    //
    // `for range` facilities
    //

    auto begin()
    {
        return _List.begin();
    }

    auto end()
    {
        return _List.end();
    }

    const auto begin() const
    {
        return _List.begin();
    }

    const auto end() const
    {
        return _List.end();
    }
};

//
// Variant
//

template <class... Args>
using VariantInternal = std::variant<Args...>;

template <class... Args>
class Variant
{
private:
    VariantInternal<Args...> _Variant;

public:
    template <class T>
    Variant(T value)
        : _Variant(value)
    {
    }

    template <class T>
    Variant& operator=(T value)
    {
        _Variant = value;
        return *this;
    }

    template <class T>
    Bool Contains() const
    {
        return std::holds_alternative<T>(_Variant);
    }

    template <class T>
    T& Get()
    {
        return std::get<T>(_Variant);
    }

    template <class T>
    const T& Get() const
    {
        return std::get<T>(_Variant);
    }
};

///////////////////////////////////////////////////////////////////////////////
// Pointers
///////////////////////////////////////////////////////////////////////////////

template <class T>
using UniquePtr = std::unique_ptr<T>;

template <class T, class... Args>
UniquePtr<T> MakeUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <class T>
using SharedPtr = std::shared_ptr<T>;

template <class T, class U>
SharedPtr<T> SharedPtrCast(SharedPtr<U>& sharedPtr)
{
    return std::static_pointer_cast<T>(sharedPtr);
}

template <class T, class U>
const SharedPtr<T> SharedPtrCast(const SharedPtr<U>& sharedPtr)
{
    return std::static_pointer_cast<T>(sharedPtr);
}

template <class T, class... Args>
EnableIf<!IsArray<T>, SharedPtr<T>> MakeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <class T>
EnableIf<IsArray<T>, SharedPtr<T>> MakeShared(U64 size)
{
    using U = RemoveExtent<T>;
    return SharedPtr<T>(new U[size], std::default_delete<U[]>());
}

///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////

template<typename ReturnType, typename... Args>
class FreeFunction
{
public:
    using FunctionType = ReturnType(*)(Args...);

    FreeFunction(FunctionType function)
        : m_Function(function)
    {
    }

    ReturnType operator()(Args... args)
    {
        return m_Function(std::forward<Args>(args)...);
    }

private:
    FunctionType m_Function;
};

template<typename ClassType, typename ReturnType, typename... Args>
class MemberFunction
{
public:
    using MemberFunctionType = ReturnType(ClassType::*)(Args...);
    MemberFunction(ClassType* object, MemberFunctionType memberFunction)
        : m_Object(object)
        , m_MemberFunction(memberFunction)
    {
    }

    ReturnType operator()(Args... args)
    {
        return (m_Object->*m_MemberFunction)(std::forward<Args>(args)...);
    }

private:
    ClassType* m_Object;
    MemberFunctionType m_MemberFunction;
};

// Deduction guide
template<typename ClassType, typename ReturnType, typename... Args>
MemberFunction(ClassType*, ReturnType(ClassType::*)(Args...))->MemberFunction<ClassType, ReturnType, Args...>;

///////////////////////////////////////////////////////////////////////////////
// Multithreading
///////////////////////////////////////////////////////////////////////////////

template <class T>
using Atomic = std::atomic<T>;

template <typename T>
Bool CompareExchange(Atomic<T>& atomicValue, T& expected, T desired)
{
    return atomicValue.compare_exchange_strong(expected, desired);
}

//
// Mutex
//

using MutexInternal = std::mutex;

class Mutex
{
private:
    MutexInternal _Mutex;


public:
    void Lock()
    {
        _Mutex.lock();
    }

    void Unlock()
    {
        _Mutex.unlock();
    }

    Bool TryLock()
    {
        return _Mutex.try_lock();
    }
};

class ScopedMutex
{
private:
    Mutex& _Mutex;

public:
    ScopedMutex(Mutex& mutex)
        : _Mutex(mutex)
    {
        _Mutex.Lock();
    }

    ~ScopedMutex()
    {
        _Mutex.Unlock();
    }
};

//
// Thread
//

using ThreadInternal = std::thread;

template <class Callable>
class Thread
{
private:
    ThreadInternal _Thread;
    U64 _Id;
    String _Name;
    Callable _Function;

private:
    static Mutex _MapMutex;
    static Map<U64, String> _ThreadNames;

    static void SetThreadName(U64 threadId, const String& name)
    {
        ScopedMutex lock(_MapMutex);
        _ThreadNames[threadId] = name;
    }

public:
    static U64 CurrentThreadId()
    {
        return std::hash<std::thread::id>{}(std::this_thread::get_id());
    }

    static String CurrentThreadName()
    {
        ScopedMutex lock(_MapMutex);
        U64 threadId = CurrentThreadId();
        String threadName = String::ToString(threadId);
        _ThreadNames.Find(threadId, threadName);
        return threadName;
    }

    static void SleepMs(U32 milliseconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    static void SleepSeconds(F32 seconds)
    {
        std::this_thread::sleep_for(std::chrono::duration<F32>(seconds));
    }

public:
    Thread(const String& name, Callable callable)
        : _Name(name)
        , _Function(std::move(callable))
    {
    }

    template <class... Args>
    void Start(Args... args)
    {
        _Thread = std::thread([this, &args...]()
        {
            _Callable(std::forward<Args>(args)...);
        });
        _Id = std::hash<std::thread::id>{}(_Thread.get_id());
        SetThreadName(_Id, _Name);
    }

    Bool IsRunning() const
    {
        return _Thread.joinable();
    }

    void Join()
    {
        if (IsRunning())
        {
            _Thread.join();
            ClearThreadId();
        }
    }

    void Detach()
    {
        _Thread.detach();
        ClearThreadId();
    }

private:
    void ClearThreadId()
    {
        ScopedMutex lock(_MapMutex);
        _ThreadNames.Remove(_Id);
        _Id = 0;
    }
};

///////////////////////////////////////////////////////////////////////////////
// World positioning
///////////////////////////////////////////////////////////////////////////////

struct Vector3
{
    F32 x, y, z;

    Vector3(F32 x = 0.0f, F32 y = 0.0f, F32 z = 0.0f)
        : x(x)
        , y(y)
        , z(z)
    {
    }

    Vector3 operator+(const Vector3& other) const
    {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const
    {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(F32 scalar) const
    {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    F32 Dot(const Vector3& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 Cross(const Vector3& other) const
    {
        return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }

    F32 Length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3& Normalize()
    {
        F32 len = Length();
        if (len > 0)
        {
            x /= len;
            y /= len;
            z /= len;
        }
        return *this;
    }

    bool operator==(const Vector3& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const Vector3& other) const
    {
        return !(*this == other);
    }
};

struct Quaternion
{
    F32 x, y, z, w;

    Quaternion(F32 x = 0.0f, F32 y = 0.0f, F32 z = 0.0f, F32 w = 1.0f)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
    }

    Quaternion operator*(const Quaternion& other) const
    {
        return Quaternion(
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w,
            w * other.w - x * other.x - y * other.y - z * other.z
        );
    }

    Quaternion& Normalize()
    {
        F32 len = std::sqrt(x * x + y * y + z * z + w * w);
        if (len > 0)
        {
            x /= len;
            y /= len;
            z /= len;
            w /= len;
        }
        return *this;
    }

    bool operator==(const Quaternion& other) const
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    bool operator!=(const Quaternion& other) const
    {
        return !(*this == other);
    }
};

struct Point
{
    F32 x, y, z;

    Point(F32 x = 0.0f, F32 y = 0.0f, F32 z = 0.0f)
        : x(x)
        , y(y)
        , z(z)
    {
    }

    bool operator==(const Point& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const Point& other) const
    {
        return !(*this == other);
    }
};

F32 Distance(const Point& p1, const Point& p2)
{
    return std::sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y) + (p2.z - p1.z) * (p2.z - p1.z));
}

Point operator+(const Point& point, const Vector3& vector)
{
    return Point(point.x + vector.x, point.y + vector.y, point.z + vector.z);
}

Point operator-(const Point& point, const Vector3& vector)
{
    return Point(point.x - vector.x, point.y - vector.y, point.z - vector.z);
}

Vector3 operator-(const Point& p1, const Point& p2)
{
    return Vector3(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
}

struct Transform
{
    Point position;
    Quaternion orientation;

    Transform(const Point& position = Point(), const Quaternion& orientation = Quaternion())
        : position(position)
        , orientation(orientation)
    {
    }

    void SetPosition(const Point& newPosition)
    {
        position = newPosition;
    }

    void SetOrientation(const Quaternion& newOrientation)
    {
        orientation = newOrientation;
    }

    void Translate(const Vector3& translation)
    {
        position = position + translation;
    }

    void Rotate(const Quaternion& rotation)
    {
        orientation = rotation * orientation;
        orientation.Normalize();
    }

    bool operator==(const Transform& other) const
    {
        return position == other.position && orientation == other.orientation;
    }

    bool operator!=(const Transform& other) const
    {
        return !(*this == other);
    }
};

///////////////////////////////////////////////////////////////////////////////
// Logging
///////////////////////////////////////////////////////////////////////////////

#define LOG(format, ...) printf(format "\n", ##__VA_ARGS__);
#define LOG_WARNING(format, ...) printf("[WARNING] " format "\n", ##__VA_ARGS__);
#define LOG_ERROR(format, ...) printf("[ERROR] " format " [%s l.%d]\n", ##__VA_ARGS__, __FILE__, __LINE__);

#if defined(_MSC_VER)
    #define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    #define DEBUG_BREAK() __builtin_trap()
#else
    #define DEBUG_BREAK() assert(false)
#endif

#define DEBUG_ASSERT(condition, format, ...) \
    if (!(condition)) \
    { \
        LOG("[ASSERT] " format "\n", ##__VA_ARGS__); \
        DEBUG_BREAK(); \
    }

///////////////////////////////////////////////////////////////////////////////
// Utilities
///////////////////////////////////////////////////////////////////////////////

#define DECLARE_FLAG_ENUM(EnumName, UnderlyingType) \
    enum class EnumName : UnderlyingType; \
    constexpr EnumName operator|(EnumName a, EnumName b) \
    { \
        return static_cast<EnumName>(static_cast<UnderlyingType>(a) | static_cast<UnderlyingType>(b)); \
    } \
    constexpr EnumName operator&(EnumName a, EnumName b) \
    { \
        return static_cast<EnumName>(static_cast<UnderlyingType>(a) & static_cast<UnderlyingType>(b)); \
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

#define FLAG(name, shift) name = 1 << shift

    void ZeroizeMemory(void* pointer, U64 size)
    {
        std::memset(pointer, 0, size);
    }

    template <typename T>
    class Passkey
    {
    private:
        friend T;
        Passkey() {}
        Passkey(const Passkey&) {}
        Passkey& operator=(const Passkey&) = delete;
    };

} // namespace Midnight