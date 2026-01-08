#pragma once

#include <filesystem>
#include <cstdint>
#include <QPointer>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <stdfloat>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;
using usz = std::size_t;

using i64 = std::int64_t;
using i32 = std::int32_t;
using i16 = std::int16_t;
using i8 = std::int8_t;
using isz = std::make_signed_t<std::size_t>;

using f16 = _Float16;
using f32 = _Float32;
using f64 = _Float64;
using f128 = _Float128;

static constexpr auto read_asset(std::string path) -> std::string
{
  auto file_path = std::filesystem::current_path() / "assets" / path;
  auto stream = std::ifstream(file_path);
  std::cout << file_path.filename() << std::endl << std::filesystem::absolute(file_path) << std::endl;
  std::stringstream buffer;
  if (not stream.is_open()) throw std::invalid_argument("failed to open asset at " + file_path.string());
  buffer << stream.rdbuf();
  stream.close();
  return buffer.str();
}
