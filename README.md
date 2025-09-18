# xbits: Essential Bit Manipulation Utilities in C++

xbits is a lightweight, header-only C++ library providing efficient bit-level operations, 
alignment tools, power-of-two calculations, and hashing for performance-oriented applications. 
Built for C++20 and above, it leverages constexpr where possible for compile-time evaluation and zero-runtime overhead.

## Key Features

- **Size-Agnostic Integer Types**: `byte_size_uint_t` and `byte_size_int_t` select optimal signed/unsigned ints based on byte size (1-8 bytes).
- **Alignment Primitives**: `Align`, `AlignLower`, and `isAlign` for pointer and integer alignment to power-of-2 boundaries, with pointer overloads.
- **Flag Management**: `FlagToggle`, `FlagOn`, `FlagOff`, `FlagIsOn`, and `FlagsAreOn` for bitflag manipulation on any integral type.
- **Power-of-Two Utilities**: `isPowTwo`, `RoundToNextPowOfTwo`, `Log2Int`, and `Log2IntRoundUp` for fast bit math and rounding.
- **Divisibility Check**: `isDivBy2PowerX` to verify if a number is divisible by 2^m.
- **MurmurHash3**: Compile-time 32/64-bit hash for integers, based on the standard algorithm.
- **Bit Scanning**: `ctz32` and `ctz64` (count trailing zeros) using intrinsics (MSVC) or fallback constexpr implementations; includes `popcnt32` and `clz32` for non-MSVC.
- **Header-Only & Constexpr-Heavy**: No dependencies, minimal overhead, works in C++11+ (full constexpr in C++14+).

## Dependencies

None! Pure standard C++.

## Code Example

```cpp
#include "xbits.h"
#include <cassert>
#include <cstdint>

int main() {
    // Alignment
    constexpr std::uintptr_t addr = 57;
    static_assert(xbits::Align(addr, 16) == 64, "Upper alignment");
    static_assert(xbits::AlignLower(addr, 16) == 48, "Lower alignment");
    static_assert(xbits::isAlign(64, 16), "Is aligned");

    // Power-of-two checks and rounding
    static_assert(xbits::isPowTwo(8u), "Is power of two");
    static_assert(xbits::RoundToNextPowOfTwo(9u) == 16, "Next power of two");
    static_assert(xbits::Log2IntRoundUp(7u) == 3, "Log2 roundup");

    // Flag operations
    std::uint32_t flags = 0;
    constexpr std::uint32_t bit1 = 1u << 0;
    xbits::FlagOn(flags, bit1);
    assert(xbits::FlagIsOn(flags, bit1));
    xbits::FlagToggle(flags, bit1);
    assert(!xbits::FlagIsOn(flags, bit1));

    // Hashing
    constexpr std::uint64_t hash = xbits::MurmurHash3(42ull);
    // hash == 0x... (compile-time constant)

    // Trailing zeros (ctz)
    assert(xbits::ctz32(8u) == 3);  // 8 = 1000b, 3 trailing zeros
    assert(xbits::ctz64(16ull) == 4);

    // Divisibility by 2^m
    assert(xbits::isDivBy2PowerX(24, 3));  // 24 divisible by 8 (2^3)

    return 0;
}
```

## Usage

1. **Include the Header**: `#include "xbits.h"` in your source files.
2. **Use Namespaces**: Access via `xbits::` (e.g., `xbits::Align(ptr, 16)`).
3. **Templates for Flexibility**: Functions are templated on integral/pointer types; constexpr ensures compile-time use where possible.
4. **MSVC Optimizations**: Bit-scan intrinsics enabled; falls back to portable constexpr on other compilers.
5. **Edge Cases**: Handles zero inputs (e.g., `RoundToNextPowOfTwo(0) == 0`) and assertions for type safety.

## Installation

1. Download `xbits.h` and place it in your project's include path.
2. Compile with C++20 or later (e.g., `-std=c++20`).
3. No linking required—header-only.

## Contributing

Star, fork, and contribute to xbits on GitHub! 🚀 Report issues or submit PRs for new utilities, compiler support, or docs. MIT licensed for free use in any project.