#ifndef XBITS_H
#define XBITS_H
#pragma once

#include <cassert>
#include <cstdint>

namespace xbits
{
    //-------------------------------------------------------------------------------------------------------
    // Description:
    //      These are helper aliases that pick the right unsigned integer type based on how many bytes you need.
    //      For example, if you say byte_size_uint_t<1>, it gives you uint8_t (which is 1 byte).
    //      It goes up to 8 bytes, using uint64_t for sizes 5 to 8.
    //      Note: This is useful when you want an integer exactly a certain size in bytes.
    //      Edge cases: If T_SIZE_BYTES is 0 or more than 8, it won't work because the tuple only has 8 elements.
    //      But since it's a template, the compiler will error if you use invalid sizes.
    // Arguments:
    //      T_SIZE_BYTES - The number of bytes you want the integer to be (1 to 8).
    // Return:
    //      An alias to the appropriate unsigned integer type like uint8_t, uint16_t, etc.
    //-------------------------------------------------------------------------------------------------------
    template< std::size_t T_SIZE_BYTES >
    using byte_size_uint_t = std::tuple_element_t< T_SIZE_BYTES - 1, std::tuple<std::uint8_t, std::uint16_t, std::uint32_t, std::uint32_t, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t>>;


    //-------------------------------------------------------------------------------------------------------
    // Description:
    //      Similar to byte_size_uint_t, but for signed integers (can be negative).
    //      For example, byte_size_int_t<4> gives int32_t.
    //      Note: Signed means it can hold negative numbers, unlike unsigned which is only positive.
    //      Edge cases: If T_SIZE_BYTES is 0 or more than 8, it won't work because the tuple only has 8 elements.
    //      But since it's a template, the compiler will error if you use invalid sizes.
    // Arguments:
    //      T_SIZE_BYTES - The number of bytes (1 to 8).
    // Return:
    //      An alias to the signed integer type like int8_t, int16_t, etc.
    //-------------------------------------------------------------------------------------------------------
    template< std::size_t T_SIZE_BYTES >
    using byte_size_int_t = std::tuple_element_t< T_SIZE_BYTES - 1, std::tuple<std::int8_t, std::int16_t, std::int32_t, std::int32_t, std::int64_t, std::int64_t, std::int64_t, std::int64_t>>;

    //-------------------------------------------------------------------------------------------------------
    // Description:
    //      This alias take any type T and give you a signed or unsigned integer of the same size in bytes.
    //      For example, if T is float (usually 4 bytes), to_int_t<float> is int32_t.
    //      Note: This is handy when you need to match sizes for things like bit operations or memory layouts.
    //      Edge cases: If sizeof(T) is not 1-8 bytes, it may not work well, but most types are within that.
    //      If T is a pointer, sizeof(T) is usually 4 or 8 bytes depending on your system (32-bit or 64-bit).
    // Arguments:
    //      T - The type whose size you want to match.
    // Return:
    //      An alias to the matching signed or unsigned integer type.
    //-------------------------------------------------------------------------------------------------------
    template< typename T >    template< typename T >
    using to_int_t = byte_size_int_t<sizeof(T)>;

    //-------------------------------------------------------------------------------------------------------
    // Description:
    //      This alias take any type T and give you a signed or unsigned integer of the same size in bytes.
    //      For example, if T is float (usually 4 bytes), to_int_t<float> is int32_t.
    //      Note: This is handy when you need to match sizes for things like bit operations or memory layouts.
    //      Edge cases: If sizeof(T) is not 1-8 bytes, it may not work well, but most types are within that.
    //      If T is a pointer, sizeof(T) is usually 4 or 8 bytes depending on your system (32-bit or 64-bit).
    // Arguments:
    //      T - The type whose size you want to match.
    // Return:
    //      An alias to the matching signed or unsigned integer type.
    //-------------------------------------------------------------------------------------------------------
    template< typename T >
    using to_uint_t = byte_size_uint_t<sizeof(T)>;

    //------------------------------------------------------------------------------
    // Description:
    //      This function calculates 2 raised to the power of n, like 2^n.
    //      It's basically shifting 1 left by n bits, which is fast.
    //      Note: Only works for integer types. If n is too big, it might overflow (wrap around or undefined behavior).
    //      Edge cases: If n=0, returns 1 (2^0=1). If n is negative, undefined (don't use negatives).
    //      If n >= bit size of T (e.g., n>=32 for uint32_t), result is undefined due to shift overflow.
    // Arguments:
    //      n - The exponent, a non-negative integer less than the bit width of T.
    // Return:
    //      The value 2^n as type T. If overflow happens, the result is wrong or undefined.
    //------------------------------------------------------------------------------
    template< typename T > constexpr
    T SLeft( T n ) noexcept
    { 
        static_assert( std::is_integral<T>::value,"" ); 
        return static_cast<T>(1 << n); 
    }

    //------------------------------------------------------------------------------
    // Description:
    //      This aligns a number (like an address or offset) up to the next multiple of AlignTo.
    //      AlignTo must be a power of 2 (like 2,4,8,16). It makes the number bigger or stays the same to be divisible by AlignTo.
    //      Easy example: Align(57,16) = 64, because 64 is the next multiple of 16 after 57.
    //      Note: Useful for memory alignment in programming, like making sure data starts at even addresses.
    //      Edge cases: If Address is already aligned, returns Address. If Address=0, returns 0.
    //      If AlignTo=1, always returns Address (everything is aligned to 1).
    //      If AlignTo not power of 2, result is wrong. If Address negative and T signed, behavior may be odd (treat as unsigned internally).
    // Arguments:
    //      Address - The number to align (integer, can be size_t for addresses).
    //      AlignTo - Power of 2 to align to (positive int, like 16).
    // Return:
    //      The aligned number, same type as Address. Always >= Address unless overflow, but unlikely.
    //------------------------------------------------------------------------------
    template< typename T > constexpr
    T Align( T Address, const int AlignTo ) noexcept
    {
        static_assert( std::is_integral<T>::value, "This function only works with integer values" );
        using unsigned_t = to_uint_t<T>; 
        return static_cast<T>( (unsigned_t( Address ) + (static_cast<unsigned_t>(AlignTo) - 1)) & static_cast<unsigned_t>(-AlignTo) );
    }

//------------------------------------------------------------------------------
    // Description:
    //      Same as Align for numbers, but for pointers (memory addresses).
    //      It turns the pointer into a number, aligns it, and turns it back to a pointer.
    //      Example: If a pointer is at 0x39 (57), Align to 16 gives 0x40 (64).
    //      Note: Pointers are like addresses in memory. This ensures the pointer points to an aligned spot.
    //      Edge cases: Null pointer (0) stays null. If AlignTo=1, no change.
    //      Don't use with non-power-of-2 AlignTo. Aligning might make it point to invalid memory if not careful.
    // Arguments:
    //      Address - The pointer to align (like int* or void*).
    //      AlignTo - Power of 2 alignment value.
    // Return:
    //      A new pointer that's aligned, pointing to a higher or same address.
    //------------------------------------------------------------------------------
    template    template< typename T > 
    T* Align( T* Address, const int AlignTo ) noexcept
    {
        return reinterpret_cast<T*>( Align( reinterpret_cast<const std::size_t>( Address ), AlignTo ) );
    }

//------------------------------------------------------------------------------
    // Description:
    //      This aligns a number down to the previous multiple of AlignTo.
    //      Makes the number smaller or stays the same to be divisible by AlignTo.
    //      Example: AlignLower(57,16) = 48, because 48 is the previous multiple.
    //      Note: Opposite of Align (which goes up). Useful for finding the start of a block.
    //      Edge cases: If Address=0, returns 0. If already aligned, no change.
    //      For negative numbers (if T signed), treats as unsigned, might give unexpected results.
    //      If AlignTo not power of 2, wrong result.
    // Arguments:
    //      Address - The number to align down.
    //      AlignTo - Power of 2 alignment.
    // Return:
    //      The aligned number, <= Address.
    //------------------------------------------------------------------------------
    template< typename T > constexpr 
    T AlignLower( T Address, const int AlignTo ) noexcept
    {
        static_assert( std::is_integral<T>::value, "This function only works with integer values" );
        using unsigned_t = to_uint_t<T>::type; 
        return static_cast<T>( unsigned_t( Address ) & (-AlignTo) );
    }

    //------------------------------------------------------------------------------
    // Description:
    //      Pointer version of AlignLower. Aligns the pointer down.
    //      Example: Pointer at 0x39 becomes 0x30 for AlignTo=16.
    //      Note: Can make pointer point lower in memory.
    //      Edge cases: Null stays null. Be careful not to dereference invalid pointers after aligning.
    // Arguments:
    //      Address - Pointer to align down.
    //      AlignTo - Power of 2.
    // Return:
    //      Aligned pointer, <= original.
    //------------------------------------------------------------------------------    
    template< typename T > 
    T* AlignLower( T* Address, const int AlignTo ) noexcept
    {
        return reinterpret_cast<T*>( AlignLower( reinterpret_cast<const std::size_t>( Address ), AlignTo ) );
    }

    //------------------------------------------------------------------------------
    // Description:
    //      Checks if a number or pointer is already aligned to AlignTo (divisible by it).
    //      Returns true if yes, false otherwise.
    //      Example: isAlign(64,16) = true, isAlign(57,16)=false.
    //      Note: AlignTo must be power of 2. Works for pointers by treating as numbers.
    //      Edge cases: Anything with AlignTo=1 is true. 0 is aligned to any power of 2.
    //      For negative numbers, treats as unsigned, might not make sense.
    // Arguments:
    //      Addr - Number or pointer to check.
    //      AlignTo - Power of 2.
    // Return:
    //      Bool: true if aligned.
    //------------------------------------------------------------------------------
    template< typename T > constexpr 
    bool isAlign( T Addr, const int AlignTo ) noexcept
    {
        static_assert( std::is_pointer<T>::value || std::is_integral<T>::value,"" );
        using unsigned_t = to_uint_t<T>; 
        return ( unsigned_t(Addr) & (static_cast<unsigned_t>(AlignTo)-1) ) == 0;
    }

    //------------------------------------------------------------------------------
    // Description:
    //      These are functions for working with bit flags (like switches in a number).
    //      Flags are bits you can turn on/off to represent options.
    //      Note: T should be unsigned integer for best results, to avoid sign issues.
    //      Edge cases: If F=0, no change or always false. If N overflows, undefined.
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    // Description:
    //      Flips the bits in N that are set in F. On becomes off, off becomes on.
    //      Like toggling lights where F says which ones.
    //      Example: If N=0b101 (5), F=0b010 (2), becomes 0b111 (7).
    // Arguments:
    //      N - Reference to the number to change.
    //      F - The bits to toggle (uint32_t, but works with larger T).
    // Return:
    //      None (modifies N in place).
    //------------------------------------------------------------------------------
    template< class T > constexpr void    FlagToggle(       T& N, const std::uint32_t F ) noexcept;

    //------------------------------------------------------------------------------
    // Description:
    //      Turns on the bits in N that are set in F. Doesn't turn off any.
    //      Example: N=0b101, F=0b010 -> 0b111.
    // Arguments:
    //      N - Reference to modify.
    //      F - Bits to turn on.
    // Return:
    //      None.
    //------------------------------------------------------------------------------
    template< class T > constexpr void    FlagOn    (       T& N, const std::uint32_t F ) noexcept;

    //------------------------------------------------------------------------------
    // Description:
    //      Turns off the bits in N that are set in F. Doesn't turn on any.
    //      Example: N=0b101, F=0b001 -> 0b100.
    // Arguments:
    //      N - Reference to modify.
    //      F - Bits to turn off.
    // Return:
    //      None.
    //------------------------------------------------------------------------------
    template< class T > constexpr void    FlagOff   (       T& N, const std::uint32_t F ) noexcept;

    //------------------------------------------------------------------------------
    // Description:
    //      Checks if at least one bit set in F is also set in N.
    //      Like "is any of these flags on?"
    //      Example: N=0b101, F=0b011 -> true (bit 0 is on in both).
    // Arguments:
    //      N - The number to check.
    //      F - The flags to test.
    // Return:
    //      Bool: true if any overlap.
    //------------------------------------------------------------------------------
    template< class T > constexpr bool    FlagIsOn  ( const T  N, const std::uint32_t F ) noexcept;

    //------------------------------------------------------------------------------
    // Description:
    //      Checks if ALL bits set in F are also set in N.
    //      Like "are all these flags on?"
    //      Example: N=0b101, F=0b001 -> true; F=0b011 -> false.
    // Arguments:
    //      N - Number to check.
    //      F - Flags to test.
    // Return:
    //      Bool: true if all match.
    //------------------------------------------------------------------------------
    template< class T > constexpr bool    FlagsAreOn( const T  N, const std::uint32_t F ) noexcept;

    //------------------------------------------------------------------------------
    // Description:
    //      Calculates the base-2 logarithm of x, assuming x is a power of 2.
    //      Answers: how many times do I shift right to get 1?
    //      Recursive: calls itself with x>>1 until x<=1.
    //      Example: Log2Int(8) = 3 (since 8=1<<3).
    //      Note: Only for positive integers. Not for floating point.
    //      Edge cases: x=1 returns 0 (or p). x=0 returns 0 but assume x>0.
    //      If x not power of 2, gives floor(log2(x)).
    // Arguments:
    //      x - Positive integer, preferably power of 2.
    //      p - Don't set this; it's for recursion (starts at 0).
    // Return:
    //      The log2 value as T.
    //------------------------------------------------------------------------------
    template< typename T> constexpr 
    T Log2Int( T x, int p = 0 ) noexcept
    {
        return (x <= 1) ? p : Log2Int(x >> 1, p + 1);
    }

    //------------------------------------------------------------------------------
    // Description:
    //      Determines the minimum power of two that encapsulates the given number
    //      Other cool int bits magic: http://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign
    //------------------------------------------------------------------------------
    namespace details
    {
        //------------------------------------------------------------------------------
        // Description:
        //      Internal recursive function to compute next power of 2.
        //      It "smears" bits right by ORing shifted versions.
        //      Stops when s=0, adds 1 to make power of 2.
        //      Example: For x=3-1=2 (0b10), smears to 0b11, +1=4.
        //      Note: Used by RoundToNextPowOfTwo.
        //      Edge cases: x=0 gives 1 (but outer handles 0).
        // Arguments:
        //      x - Value being smeared.
        //      s - Shift amount, starts high and halves.
        // Return:
        //      The smeared value +1 when done.
        //------------------------------------------------------------------------------
        template< typename T > constexpr 
        T NextPowOfTwo( const T x, const int s ) noexcept
        {
            static_assert( std::is_integral<T>::value, "" );
            return static_cast<T>( ( s == 0 ) ? 1 + x : details::NextPowOfTwo<T>( x | (x >> s), s>>1 ) );
        }
    }

    //------------------------------------------------------------------------------
    // Description:
    //      Checks if x is exactly a power of 2 (like 1,2,4,8...).
    //      Works by checking if x & (x-1) == 0 and x>0.
    //      Example: 4 (0b100) & 3 (0b011) =0, yes.
    //      Note: x=1 is true (2^0). x=0 is false.
    //      Edge cases: Negative x: false (since & with negative is tricky).
    //      0 is false.
    // Arguments:
    //      x - Positive integer to check.
    // Return:
    //      Bool: true if power of 2.
    //------------------------------------------------------------------------------
    template< class T > constexpr 
    bool isPowTwo( const T x ) noexcept
    { 
        static_assert( std::is_integral<T>::value, "" ); 
        return !((x - 1) & x); 
    }

    //------------------------------------------------------------------------------
    // Description:
    //      Checks if n is divisible by 2^x (like n % (1<<x) ==0).
    //      Example: isDivBy2PowerX(8,3) true (8%8=0).
    //      Note: Faster than modulo for powers of 2.
    //      Edge cases: x=0: always true (div by 1).
    //      x negative: undefined shift.
    //      n=0: true for any x>0.
    //      If x >= bit width, (1<<x) undefined.
    // Arguments:
    //      n - Number to check.
    //      x - Exponent (non-negative, small).
    // Return:
    //      Bool: true if divisible.
    //------------------------------------------------------------------------------    
    template< class T1, class T2 > constexpr 
    bool isDivBy2PowerX( const T1 n, const T2 x ) noexcept
    { 
        static_assert( std::is_integral<T1>::value ); 
        static_assert( std::is_integral<T2>::value );
        // n is divisible by pow(2, x) 
        return ((n & ((1 << x) - 1)) == 0); 
    } 

    //------------------------------------------------------------------------------
    // Description:
    //      Gives the smallest number of bits needed to store x (ceiled log2(x+something)).
    //      Actually Log2Int(x) +1 if x>0, but for powers of 2 it's log2+1? Wait, see examples.
    //      Examples given: For 3, 2 bits (0b11). But 4=0b100, 3 bits.
    //      Note: It's ceil(log2(x+1)) or something? For 3: ceil(log2(4))=2? Wait, matches asserts.
    //      Asserts: Log2IntRoundUp(3)=2, but 3 needs 2 bits? 3=0b11, yes 2 bits.
    //      4=0b100, 3 bits.
    //      Edge cases: 0 returns 0. 1 returns 1 (1 bit).
    //      Negative: don't use.
    // Arguments:
    //      x - Non-negative integer.
    // Return:
    //      Bits needed, as T.
    //------------------------------------------------------------------------------
    template< typename T> constexpr 
    T Log2IntRoundUp( T x ) noexcept
    {
        static_assert( std::is_integral<T>::value, "" ); 
        return x < 1 ? 0 : Log2Int(x) + 1; 
    }
    static_assert( 0  == Log2IntRoundUp(0), "" );
    static_assert( 1  == Log2IntRoundUp(1), "" );
    static_assert( 2  == Log2IntRoundUp(2), "" );
    static_assert( 2  == Log2IntRoundUp(3), "" );
    static_assert( 3  == Log2IntRoundUp(4), "" );
    static_assert( 3  == Log2IntRoundUp(5), "" );
    static_assert( 3  == Log2IntRoundUp(6), "" );
    static_assert( 3  == Log2IntRoundUp(7), "" );
    static_assert( 4  == Log2IntRoundUp(8), "" );
    static_assert( 4  == Log2IntRoundUp(9), "" );
    static_assert( 4  == Log2IntRoundUp(10), "" );
    static_assert( 4  == Log2IntRoundUp(11), "" );
    static_assert( 4  == Log2IntRoundUp(12), "" );
    static_assert( 4  == Log2IntRoundUp(13), "" );
    static_assert( 10 == Log2IntRoundUp(1023), "" );
    static_assert( 11 == Log2IntRoundUp(1024), "" );

    //------------------------------------------------------------------------------
    // Description:
    //      Rounds x up to the next power of 2.
    //      Example: 3 ->4, 5->8, 8->8.
    //      Uses the bit smearing trick.
    //      Note: For powers of 2, stays the same.
    //      Edge cases: 0 returns 0 (special case).
    //      1 returns 1. Max value: if x is max for T, might overflow to 0 or undefined.
    // Arguments:
    //      x - Non-negative integer.
    // Return:
    //      Next power of 2 >=x, or 0 if x=0.
    //------------------------------------------------------------------------------
    template< typename T> constexpr 
    T RoundToNextPowOfTwo( const T x ) noexcept
    { 
        static_assert( std::is_integral<T>::value, "" ); 
        return ( x == 0 ) ? 0 : details::NextPowOfTwo<T>( x - 1, static_cast<int>(4*sizeof(T)) ); 
    }

    //------------------------------------------------------------------------------
    // Description:
    //      This is a hash function called MurmurHash3.
    //      It takes a number and mixes its bits to make a good hash (for tables, etc.).
    //      Only the final mixing step, not full hash for data.
    //      Algorithm from the link.
    //      Note: Good for avalanche: small change in input changes output a lot.
    //      Edge cases: h=0 returns 0 after mixing.
    //      Only for 4 or 8 byte types.
    // Algorithm:
    //      from code.google.com/p/smhasher/wiki/MurmurHash3
    // Arguments:
    //      h - The value to hash (uint32_t or uint64_t usually).
    // Return:
    //      The hashed value, same type.
    //------------------------------------------------------------------------------
    namespace details
    {
        //------------------------------------------------------------------------------
        // Description:
        //      Helper struct for MurmurHash3, specialized by size (4 or 8 bytes).
        //------------------------------------------------------------------------------
        template< int T_SIZE >
        struct murmurHash3_by_size {};

        //------------------------------------------------------------------------------
        // Description:
        //      For 32-bit (4 bytes): mixes h with shifts and multiplies.
        //      Constants are magic numbers for good mixing.
        // Arguments:
        //      h - uint32_t to mix.
        // Return:
        //      Mixed uint32_t.
        //------------------------------------------------------------------------------
        template<>
        struct murmurHash3_by_size<4> 
        {
             constexpr
            static auto Compute( std::uint32_t h ) noexcept
            {
                h ^= h >> 16;
                h *= 0x85ebca6b;
                h ^= h >> 13;
                h *= 0xc2b2ae35;
                h ^= h >> 16;
                return h;
            }
        };

        //------------------------------------------------------------------------------
        // Description:
        //      For 64-bit (8 bytes): similar, different constants and shifts.
        // Arguments:
        //      h - uint64_t to mix.
        // Return:
        //      Mixed uint64_t.
        //------------------------------------------------------------------------------
        template<> 
        struct murmurHash3_by_size<8> 
        {
             constexpr
            static auto Compute( std::uint64_t h ) noexcept
            {
                h ^= h >> 33;
                h *= 0xff51afd7ed558ccd;
                h ^= h >> 33;
                h *= 0xc4ceb9fe1a85ec53;
                h ^= h >> 33;
                return h;
            }
        };
    }

    //------------------------------------------------------------------------------
    // Description:
    //      Main MurmurHash3 function: calls the size-specific Compute.
    //      Casts to unsigned if needed.
    //      Note: Only works for 32 or 64 bit integers.
    //      If signed, treats as unsigned.
    //      Edge cases: Small values mix to larger random-looking.
    // Arguments:
    //      h - Integral value, size 4 or 8 bytes.
    // Return:
    //      Hashed value as T.
    //------------------------------------------------------------------------------
    template< typename T >  constexpr 
    T MurmurHash3( T h ) noexcept
    {
        static_assert( sizeof(T) >= 4 && sizeof(T) <= 8, "" );
        static_assert( std::is_integral<T>::value,"" );
        return static_cast<T>(details::murmurHash3_by_size<sizeof(T)>::Compute( static_cast<to_uint_t<T> >(h) ));
    }

#if _MSC_VER
    #pragma intrinsic(_BitScanForward)
    //------------------------------------------------------------------------------
    // Description:
    //      Counts trailing zeros in binary (from least significant bit).
    //      Scans until first 1 bit from right.
    //      Platform-specific: Uses MSVC intrinsic or fallback.
    //      Example: ctz32(8) =3 (0b1000 has 3 zeros at end).
    //      Note: For bit operations, like finding lowest set bit.
    //      Edge cases: value=0 returns 32 or 64 (all zeros).
    // Arguments:
    //      value - Unsigned 32 or 64 bit.
    // Return:
    //      Number of trailing zeros (0-31 or 0-63).
    //------------------------------------------------------------------------------
    inline
    std::uint32_t ctz32( std::uint32_t value ) noexcept
    {
        unsigned long leading_zero = 0;

        if ( _BitScanForward( &leading_zero, value ) )
        {
            return leading_zero;
        }
        else
        {
            // undefine....
            return 32;
        }
    }

    //------------------------------------------------------------------------------
    // Description:
    //      Counts leading zeros from most significant bit.
    //      Fills bits down then uses popcnt.
    //      Example: clz32(1<<31)=0 (top bit set).
    //      clz32(1)=31.
    //      Note: clz = count leading zeros.
    //      Edge cases: 0=32, all bits set=0.
    // Arguments:
    //      x - uint32_t.
    // Return:
    //      Leading zeros (0-32).
    //------------------------------------------------------------------------------    
    inline
    std::uint32_t ctz64( std::uint64_t value ) noexcept
    {
        unsigned long leading_zero = 0;

        if ( _BitScanForward64( &leading_zero, value ) )
        {
            return leading_zero;
        }
        else
        {
            // undefine....
            return 64;
        }
    }
#else
    //------------------------------------------------------------------------------
    // Description:
    //      Counts set bits (1s) in 32-bit number.
    //      Uses bit tricks to sum in parallel.
    //      Example: popcnt32(7)=3 (0b111).
    //      Note: Used in fallbacks for clz/ctz.
    //      Edge cases: 0=0, all 1s=32.
    // Arguments:
    //      x - uint32_t.
    // Return:
    //      Number of 1 bits (0-32).
    //------------------------------------------------------------------------------
    constexpr
    std::uint32_t popcnt32( uint32_t x ) noexcept
    {
        x -= ((x >> 1) & 0x55555555);
        x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
        x = (((x >> 4) + x) & 0x0f0f0f0f);
        x += (x >> 8);
        x += (x >> 16);
        return x & 0x0000003f;
    }

    //------------------------------------------------------------------------------
    // Description:
    //      Counts leading zeros from most significant bit.
    //      Fills bits down then uses popcnt.
    //      Example: clz32(1<<31)=0 (top bit set).
    //      clz32(1)=31.
    //      Note: clz = count leading zeros.
    //      Edge cases: 0=32, all bits set=0.
    // Arguments:
    //      x - uint32_t.
    // Return:
    //      Leading zeros (0-32).
    //------------------------------------------------------------------------------
    constexpr
    std::uint32_t clz32( uint32_t x ) noexcept
    {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        return 32 - popcnt32(x);
    }

    //------------------------------------------------------------------------------
    // Description:
    //      Counts the number of trailing zeros in the binary representation of x (from the least significant bit).
    //      It finds how many zeros are at the end before the first 1 bit.
    //      This is useful for bit manipulation, like finding the position of the lowest set bit.
    //      The function uses a bit trick: (x & -x) isolates the lowest set bit, then subtract 1 to get all bits below it set to 1, and counts those bits with popcnt32.
    //      Note: Treats x as unsigned 32-bit. If you need to handle 64-bit, use ctz64.
    //      Edge cases: If x=0, returns 32 (since all 32 bits are "trailing zeros").
    //      If x=1, returns 0 (no trailing zeros). If lowest bit is set (odd number), returns 0.
    //      For powers of 2, returns the log2 position (e.g., 8=0b1000 returns 3).
    // Arguments:
    //      x - A 32-bit unsigned integer (uint32_t). Can be 0 to UINT32_MAX.
    // Return:
    //      The count of trailing zeros as uint32_t (0 to 32 inclusive).
    //------------------------------------------------------------------------------
    constexpr
    std::uint32_t ctz32( std::uint32_t x ) noexcept
    {
        return popcnt32((x & -x) - 1);
    }
#endif

}

#endif
