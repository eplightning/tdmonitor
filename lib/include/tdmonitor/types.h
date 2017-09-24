#pragma once

#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>

#include <stdint.h>

#define TDM_NAMESPACE namespace TDM {
#define END_NAMESPACE }
#define UNUSED(x) (void)(x)

TDM_NAMESPACE

// STL
typedef std::string String;
typedef std::chrono::steady_clock SteadyClock;
typedef std::lock_guard<std::mutex> MutexLock;
typedef std::unique_lock<std::mutex> UniqueLock;
typedef std::mutex Mutex;

template<class T> using UniquePtr = std::unique_ptr < T > ;
template<class T> using SharedPtr = std::shared_ptr < T > ;
template<class T> using WeakPtr = std::weak_ptr < T > ;
template<class T, class T2> using Map = std::map < T, T2 > ;
template<class T, class T2> using HashMap = std::unordered_map < T, T2 > ;
template<class T> using Vector = std::vector < T > ;

// unsigned int
typedef unsigned int uint;

// 8-bit integer
typedef uint8_t u8;
typedef int8_t s8;

// 16-bit integer
typedef uint16_t u16;
typedef int16_t s16;

// 32-bit integer
typedef uint32_t u32;
typedef int32_t s32;

// 64-bit integer
typedef uint64_t u64;
typedef int64_t s64;

END_NAMESPACE
