#pragma once

#include <cstdint>
#include <QPointer>
#include <stdfloat>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;

using i64 = std::int64_t;
using i32 = std::int32_t;
using i16 = std::int16_t;
using i8 = std::int8_t;

using f16 = _Float16;
using f32 = _Float32;
using f64 = _Float64;
using f128 = _Float128;

template <typename T, typename... Args>
auto make_ptr(Args &&...args)
{
  return QPointer<T>(new T(std::forward<Args>(args)...));
}
