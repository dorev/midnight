#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <initializer_list>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace Loom
{

// Primitives
using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s16 = int16_t;
using s32 = int32_t;

// Concurrency
using mutex = std::mutex;
using scoped_lock = std::lock_guard<mutex>;
using shared_mutex = std::shared_mutex;
using shared_lock = std::shared_lock<shared_mutex>;
using unique_lock = std::unique_lock<shared_mutex>;
template <class T> using atomic = std::atomic<T>;

// Pointers
template <class T> using unique_ptr = std::unique_ptr<T>;
template <class T> using shared_ptr = std::shared_ptr<T>;
template <typename T, typename... Args>
auto make_shared(Args&&... args) -> shared_ptr<T>
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template <typename T, typename U>
shared_ptr<T> shared_ptr_cast(const shared_ptr<U>& ptr) \
{
    return std::static_pointer_cast<T>(ptr);
}

// Containers
using string = std::string;
template <class T> using vector = std::vector<T>;
template <class T> using initializer_list = std::initializer_list<T>;
template <class T> using set = std::set<T>;
template <class K, class V> using map = std::map<K, V>;
template <class... T> using variant = std::variant<T...>;

// World positioning
struct Vector3
{
    float x,y,z;
};

struct Quaternion
{
    float w,x,y,z;
};

struct Transform
{
    Vector3 point;
    Quaternion rotation;
};

} // namespace Loom
