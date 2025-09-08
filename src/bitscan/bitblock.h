/** 
 * @file bitblock.h
 * @brief Low-level bit manipulation operations for 64-bit blocks
 * @author Pablo San Segundo
 * @version 1.0
 * @date 2025
 * @since 30/01/2025 (refactored, added trimming functions, Doxygen tags)
 * 
 * @details This file provides the foundational bit manipulation operations that power
 * the entire BITSCAN library. It contains highly optimized functions for:
 * 
 * - **Bit scanning**: Finding first/last set bits using various algorithms
 * - **Population counting**: Counting 1-bits with hardware and software methods
 * - **Bit masking**: Creating and applying masks for range operations
 * - **Platform abstraction**: Cross-platform intrinsics for optimal performance
 * 
 * The implementation prioritizes performance through:
 * - Hardware intrinsics when available (BSF, BSR, POPCNT)
 * - Fallback algorithms using lookup tables and De Bruijn sequences
 * - Compile-time algorithm selection based on target platform
 * 
 * @note All operations work on 64-bit BITBOARD types. Performance-critical functions
 * are implemented as inline for zero-overhead abstraction.
 **/

#ifndef __BITBLOCK_H__
#define __BITBLOCK_H__

#include <iostream>
#include "bbtypes.h"
#include "bbconfig.h"
#include "tables.h"

/**
 * @defgroup BitblockPlatform Platform-Dependent Hardware Intrinsics and Constants
 * @brief Cross-platform hardware instruction wrappers and compile-time constants
 * @details This group encompasses all platform-specific implementations, hardware
 * intrinsics, and mathematical constants used for optimal bit manipulation across
 * different architectures and compilers.
 * 
 * **Supported Platforms:**
 * - GCC: Linux, MinGW64, x86_64, ARM64
 * - MSVC: Windows x64
 * - Architecture-specific optimizations for x86_64 and ARM64
 * 
 * **Includes:**
 * - Hardware intrinsic wrappers (_BitScanForward64, _BitScanReverse64)
 * - De Bruijn sequence constants for bit scanning
 * - Platform abstraction macros
 * 
 * @{
 */

#ifdef __GNUC__

#ifdef __MINGW64__
//	#define __popcnt64 __builtin_popcountll					/* removed 6/11/2017 for mingw_64 */
	#include <intrin.h>										//windows specific
#else
	#define __popcnt64 __builtin_popcountll

	#if defined(__x86_64__)
		#include <x86intrin.h>
		/**
		 * @brief x86_64 forward bit scan using BSF instruction
		 * @param Index Output parameter for bit position
		 * @param Mask 64-bit value to scan
		 * @return 1 if bit found, 0 if mask is zero
		 * @details Uses inline assembly BSF (Bit Scan Forward) instruction
		 * for optimal performance on x86_64 architecture.
		 */
		static inline unsigned char _BitScanForward64(unsigned long* Index, unsigned long long Mask)
		{
			unsigned long long Ret;
			__asm__(
				"bsfq %[Mask], %[Ret]"
				: [Ret] "=r" (Ret)
				: [Mask] "mr" (Mask)
			);
			*Index = (unsigned long)Ret;
			return Mask ? 1 : 0;
		}

		/**
		 * @brief x86_64 reverse bit scan using BSR instruction
		 * @param Index Output parameter for bit position  
		 * @param Mask 64-bit value to scan
		 * @return 1 if bit found, 0 if mask is zero
		 * @details Uses inline assembly BSR (Bit Scan Reverse) instruction
		 * for optimal performance on x86_64 architecture.
		 */
		static inline unsigned char _BitScanReverse64(unsigned long* Index, unsigned long long Mask)
		{
			unsigned long long Ret;
			__asm__(
				"bsrq %[Mask], %[Ret]"
				: [Ret] "=r" (Ret)
				: [Mask] "mr" (Mask)
			);
			*Index = (unsigned long)Ret;
			return Mask ? 1 : 0;
		}
	#elif defined(__aarch64__)
		/**
		 * @brief ARM64 forward bit scan using count trailing zeros
		 * @param Index Output parameter for bit position
		 * @param Mask 64-bit value to scan
		 * @return 1 if bit found, 0 if mask is zero
		 * @details Uses GCC builtin __builtin_ctzll for ARM64 optimization.
		 */
		static inline unsigned char _BitScanForward64(unsigned long* Index, unsigned long long Mask)
		{
			if (Mask == 0) return 0;
			*Index = __builtin_ctzll(Mask);
			return 1;
		}

		/**
		 * @brief ARM64 reverse bit scan using count leading zeros
		 * @param Index Output parameter for bit position
		 * @param Mask 64-bit value to scan  
		 * @return 1 if bit found, 0 if mask is zero
		 * @details Uses GCC builtin __builtin_clzll for ARM64 optimization.
		 */
		static inline unsigned char _BitScanReverse64(unsigned long* Index, unsigned long long Mask)
		{
			if (Mask == 0) return 0;
			*Index = 63 - __builtin_clzll(Mask);
			return 1;
		}
	#else
		#error "Unsupported architecture - only x86_64 and ARM64 are supported"
	#endif
#endif

#else
	/** @brief Windows/MSVC intrinsics header */
	#include <intrin.h>
#endif

/** @} */ // end BitblockPlatform group

namespace bitgraph {

	/**
	 * @namespace bitgraph::bblock
	 * @brief High-performance operations on single 64-bit blocks
	 * @details This namespace contains optimized functions for manipulating individual
	 * 64-bit bitboards. These form the building blocks for larger bitset operations
	 * and are heavily optimized for performance in combinatorial algorithms.
	 * 
	 * **Key Features:**
	 * - Hardware-accelerated bit scanning (BSF/BSR/CLZ/CTZ)
	 * - Multiple algorithm implementations for different use cases
	 * - Compile-time constants for De Bruijn sequences
	 * - Population count operations with hardware fallbacks
	 * 
	 * All functions operate on single BITBOARD (64-bit) values and are designed
	 * to be called millions of times in tight loops during optimization algorithms.
	 */
	namespace bblock {

		/**
		 * @brief De Bruijn sequence constants for efficient bit scanning
		 * @details These constants enable branch-free bit position lookup through
		 * multiplication and bit shifting operations.
		 */

		/** @brief De Bruijn multiplier for isolated LSB (b & -b) method */
		constexpr unsigned long long DEBRUIJN_MN_64_ISOL = 0x07EDD5E59A4E28C2;

		/** @brief De Bruijn multiplier for separated bits (b ^ (b-1)) method */
		constexpr unsigned long long DEBRUIJN_MN_64_SEP = 0x03f79d71b4cb0a89;

		/** @brief Bit shift amount for De Bruijn index extraction */
		constexpr unsigned long long DEBRUIJN_MN_64_SHIFT = 58;
				
		using _impl::Tables; ///< Access to lookup tables for bit operations

		/**
		 * @defgroup BitblockCore Core Bit Manipulation Operations
		 * @brief Essential bit scanning, population counting, and testing functions
		 * @details This group encompasses all fundamental bit manipulation operations
		 * including bit scanning (finding first/last set bits), population counting,
		 * bit testing, and their various algorithm implementations.
		 * 
		 * **Algorithm Categories:**
		 * - **Hardware intrinsics**: BSF/BSR/POPCNT instructions (fastest)
		 * - **De Bruijn sequences**: Multiplication-based lookup (fast, portable)
		 * - **Lookup tables**: Memory-based approach (good for repeated patterns)
		 * - **Population count**: Mathematical approaches using various methods
		 * 
		 * All functions are optimized for performance in tight loops typical of
		 * combinatorial optimization algorithms.
		 * @{
		 */

		/**
		 * @brief Test if a specific bit is set in a bitboard
		 * @param bb The 64-bit bitboard to test
		 * @param bit Bit position to test [0-63]
		 * @return true if bit is set, false otherwise
		 * @details Uses bit masking for O(1) testing. More efficient than
		 * shifting operations for single bit tests.
		 * @note Complexity: O(1) - Single bitwise AND operation
		 */
		inline bool is_bit(const BITBOARD bb, int bit) { 
			return (bb & Tables::mask[bit]); 
		}

		/**
		 * @brief Find LSB using modulo-based perfect hashing
		 * @param bb Input 64-bit bitboard
		 * @return Index of least significant bit [0-63], or -1 if empty
		 * @details Uses perfect hashing with modulo operation to map single-bit
		 * patterns to their positions. The lookup table T_64[67] contains indices
		 * for all possible single-bit bitboards (000...1...000 patterns).
		 * 
		 * @warning Modulus operation is not efficient on most processors.
		 * This method is primarily for educational/reference purposes.
		 * @note Complexity: O(1) but with high constant due to modulo
		 */
		int lsb64_mod(const BITBOARD bb);

		/**
		 * @brief Find LSB using 16-bit lookup table approach
		 * @param bb Input 64-bit bitboard  
		 * @return Index of least significant bit [0-63], or -1 if empty
		 * @details Divides 64-bit value into 16-bit chunks and uses lookup
		 * tables to find the first set bit. Balances memory usage with speed.
		 * @note Complexity: O(1) with moderate memory footprint
		 */
		int lsb64_lup(const BITBOARD bb);

		/**
		 * @brief Find LSB using efficient 16-bit lookup table
		 * @param bb Input 64-bit bitboard
		 * @return Index of least significant bit [0-63], or -1 if empty  
		 * @details Optimized lookup table implementation that provides best
		 * performance for 32-bit x86 systems on average. Performance degrades
		 * only for sparse bitboards with 1-bits in the final 16-bit segment.
		 * 
		 * @note Recommended for systems without hardware BSF instruction
		 * @note Complexity: O(1) with optimized memory access patterns
		 */
		int lsb64_lup_eff(const BITBOARD bb);

		/**
		 * @brief Find LSB using population count approach  
		 * @param bb Input 64-bit bitboard
		 * @return Index of least significant bit [0-63], or -1 if empty
		 * @details Mathematical approach using population count operations.
		 * Computes LSB index as: popcount((bb & -bb) - 1).
		 * 
		 * @note Requires efficient POPCNT instruction for good performance
		 * @note Complexity: O(1) when POPCNT available, O(log n) otherwise
		 * @since 2008
		 */
		int lsb64_pc(const BITBOARD bb);

		/**
		* @brief Index of the least significant bit of bb function implemented
		*		 with a De Bruijn magic word for perfect hashing as
		*		 a product and a shift operation.
		* @param bb: input 64-bit bitblock
		* @returns index of the least significant bit or -1 if empty
		* @date 2008
		* @details There are two implementations:
		*		    a) ISOLANI_LSB with hashing bb &(-bb)
		*			b) All 1-bits to LSB with hashing bb^(bb-1)
		*
		*			Option b) would seem to exploit the CPU HW better on average
		*			and is defined as default.
		*			To change this option go to config.h file
		**/
		inline	 int lsb64_de_Bruijn(const BITBOARD bb);

		/**
		* @brief Index of the least significant bit of bb (default)
		*		(intrin.h WIN / x86intrin.h Linux lib)
		* @param bb: input 64-bit bitblock
		* @details implemented by calling processor instructions
		* @returns index of the least significant bit or -1 if empty
		**/
		inline int lsb64_intrinsic(const BITBOARD bb);


		/**
		* @brief Index of the least significant bit in bb
		*		 (RECOMMENDED to use - calls lsb64_intrinsic)
		* @param bb: input 64-bit bitblock
		* @returns index of the least significant bit or -1 if empty
		**/
		inline
			int lsb(const BITBOARD bb) { return 	lsb64_intrinsic(bb); }


		/**
		* @brief Index of the most significant bit in bb implemented
		*		 with a 16-bit lookup table
		* @param bb: input 64-bit bitblock
		* @returns Index of the most least significant bit or -1 if empty
		**/
		int msb64_lup(const BITBOARD bb);

		/**
		* @brief Index of the most significant bit of bb implemented
		*		 with a De Bruijn magic word for perfect hashing as
		*		 a product and a shift operation.
		*
		*		I. Does not require lookup tables
		*		II. It first creates 1-bits from the least significant to MSB
		* @param bb: input 64-bit bitblock
		* @details Nice to not require LUPs of 65535 entries
		*
		* TODO - efficiency tests
		**/
		int  msb64_de_Bruijn(const BITBOARD bb);		//De Bruijn magic word 

		/**
		* @brief Index of the most significant bit of bb which uses processor instructions
		*		(intrin.h WIN / x86intrin.h Linux lib)
		* @param bb: input 64-bit bitblock
		* @returns index of the most significant bit or -1 if empty
		**/
		inline  int msb64_intrinsic(const BITBOARD bb);


		/**
		* @brief Index of themost significant bit of in bb
		* @param bb: input 64-bit bitblock
		* @returns index of the most significant bit or -1 if empty
		**/
		inline  int msb(const BITBOARD bb) { return msb64_intrinsic(bb); }

		/**
		 * @brief population count in bb implemented with 16-bit lookup tables
		 * @param bb input 64-bit bitboard
		 * @return number of 1-bits in the bitboard
		 * @details Default lookup table implementation for systems without POPCNT
		 */
		int popc64_lup(const BITBOARD bb);

		/**
		 * @brief population count in bb with optimized lookup tables
		 * @param bb input 64-bit bitboard
		 * @return number of 1-bits in the bitboard  
		 * @details Optimized lookup table version without intermediate storage
		 */
		int popc64_lup_1(const BITBOARD bb);

		/**
		 * @brief Default population count in bb (RECOMMENDED)
		 * @param bb input 64-bit bitboard
		 * @return number of 1-bits in the bitboard
		 * @details Calls assembler POPCNT instructions when POPCOUNT_64 is enabled,
		 * otherwise falls back to table lookup implementation.
		 * @note By default POPCOUNT_64 switch is ON for optimal performance
		 */
		inline int popc64(const BITBOARD bb);

		/** @} */ // end BitblockCore group

		/**
		 * @defgroup BitblockMasks Bit Masking and Trimming Operations
		 * @brief Functions for creating bit masks and trimming bitboards
		 * @details This group contains all operations for creating various bit masks,
		 * trimming operations, and bit copying functions. These are essential for
		 * range-based bit operations and selective bit manipulation.
		 * 
		 * **Mask Types:**
		 * - Single bit masks (MASK_BIT)
		 * - Range masks (MASK_1, MASK_0) 
		 * - Directional masks (MASK_1_LOW, MASK_1_HIGH, etc.)
		 * - Trimming operations (trim_low, trim_high)
		 * - Copying operations (copy, copy_low, copy_high)
		 * @{
		 */
		/**
		* @brief Sets to 1 the bit passed and zero the rest of bits
		* @param bit: bit in the bitblock [0...63]
		* @returns 64-bit bitblock mask with only one bit set
		**/
		inline
			BITBOARD MASK_BIT(int bit) { return Tables::mask[bit]; }

		/**
		* @brief Sets to 1 the bits inside the closed range [low, high], sets to 0 the rest
		* @param low, high: positions in the bitblock [0...63]
		* @returns 64-bit bitblock mask
		**/
		inline
			BITBOARD MASK_1(int low, int high) { return Tables::mask_mid[low][high]; };

		/**
		* @brief Sets to 1 all bits in the closed range [0, 63]
		* @param idx: input reference bit position [0...63]
		* @returns 64-bit bitblock mask
		**/
		inline
			BITBOARD MASK_1_LOW(int idx) { return ~Tables::mask_high[idx]; }

		/**
		* @brief Sets to 1 all bits in the closed range [idx, 63]
		* @param idx: input reference bit position [0...63]
		* @returns 64-bit bitblock mask
		**/
		inline
			BITBOARD MASK_1_HIGH(int idx) { return ~Tables::mask_low[idx]; }

		/**
		* @brief Sets to 0 the bits inside the closed range [low, high], sets to 1 the rest
		* @param low, high: positions in the bitblock [0...63]
		* @returns 64-bit bitblock mask
		**/
		inline
			BITBOARD MASK_0(int low, int high) { return ~Tables::mask_mid[low][high]; }

		/**
	   * @brief Sets to 0 all bits in the closed range [0, idx]
	   * @param idx: input reference bit position [0...63]
	   * @returns 64-bit bitblock mask
	   **/
		inline
			BITBOARD MASK_0_LOW(int idx) { return Tables::mask_high[idx]; }

		/**
	   * @brief Sets to 0 all bits in the closed range [idx, 63]
	   * @param idx: input reference bit position [0...63]
	   * @returns 64-bit bitblock mask
	   **/
		inline
			BITBOARD MASK_0_HIGH(int idx) { return Tables::mask_low[idx]; }

		/**
		* @brief sets to 0 the bits of the bitblock bb to the right of index (the index-bit is not trimmed)
		* @param bb: input 64-bit bitblock
		* @param idx: position in the bitblock [0...63]
		* @returns the trimmed bitblock
		* @date 30/01/2015
		**/
		inline
			BITBOARD trim_low(BITBOARD bb, int idx) { return bb & ~Tables::mask_low[idx]; }

		/**
		* @brief sets to 0 the bits of the bitblock bb to the left side of index (the index-bit is not trimmed)
		* @param bb: input 64-bit bitblock
		* @param idx: position in the bitblock [0...63]
		* @returns the trimmed bitblock
		* @date 30/01/2015
		**/
		inline
			BITBOARD trim_high(BITBOARD bb, int idx) { return bb & ~Tables::mask_high[idx]; }

		/**
		* @brief replaces bits in the closed range [firstBit, lastBit] of source bitblock (source)
		*		  in destination bitblock (dest)
		* @param firstBit, lastBit: closed range of bits [0...63]
		* @param source, dest: input bitblocks
		**/
		void copy(int firstBit, int lastBit, const BITBOARD& source, BITBOARD& dest);

		/**
		* @brief replaces bits in the range [bit, 63] of source bitblock (source)
		*		  in destination bitblock (dest)
		* @param firstBit, lastBit: closed range of bits [0...63]
		* @param source, dest: input bitblocks
		**/
		void copy_high(int bit, const BITBOARD& source, BITBOARD& dest);

		/**
		* @brief replaces bits in the range [0, bit] of source bitblock (source)
		*		  in destination bitblock (dest)
		* @param firstBit, lastBit: closed range of bits [0...63]
		* @param source, dest: input bitblocks
		**/
		void copy_low(int bit, const BITBOARD& source, BITBOARD& dest);

		/** @} */ // end BitblockMasks group

		/**
		 * @brief Print bitboard with population count
		 * @param bb The 64-bit bitboard to print
		 * @param output stream (default std::cout)
		 * @param endofl whether to add end-of-line (default true)
		 * @return reference to output stream
		 * @details Streams bb and its popcount to the output stream
		 * in format: ...000111 [3]
		 */
		std::ostream& print(BITBOARD bb, std::ostream & = std::cout, bool endofl = true);

	} //end namespace bblock

}//end namespace bitgraph



//////////////////////////////
// inline implementations in header file 

namespace bitgraph {

	namespace bblock {

		using _impl::Tables;

		int lsb64_intrinsic(const BITBOARD bb_dato) {
			unsigned long index;
			if (_BitScanForward64(&index, bb_dato))
				return(index);

			return EMPTY_ELEM;
		}


		int msb64_intrinsic(const BITBOARD bb_dato) {
			unsigned long index;
			if (_BitScanReverse64(&index, bb_dato))
				return(index);

			return EMPTY_ELEM;
		}


		int popc64(const BITBOARD bb_dato) {

#ifdef POPCOUNT_INTRINSIC_64
			return __popcnt64(bb_dato);
#else
			//lookup table popcount
			register union x {
				U16 c[4];
				BITBOARD b;
			} val;

			val.b = bb_dato; //Carga unisn

			return (Tables::pc[val.c[0]] + Tables::pc[val.c[1]] + Tables::pc[val.c[2]] + Tables::pc[val.c[3]]); //Suma de poblaciones  
#endif

		}

		//alias for population count	
		inline
			int size(const BITBOARD bb_dato) { return popc64(bb_dato); }


		int lsb64_de_Bruijn(const BITBOARD bb_dato) {

#ifdef ISOLANI_LSB
			return (bb_dato == 0) ? EMPTY_ELEM : Tables::indexDeBruijn64_ISOL[((bb_dato & -bb_dato) * DEBRUIJN_MN_64_ISOL) >> DEBRUIJN_MN_64_SHIFT];
#else
			return (bb_dato == 0) ? EMPTY_ELEM : Tables::indexDeBruijn64_SEP[((bb_dato ^ (bb_dato - 1)) * DEBRUIJN_MN_64_SEP) >> DEBRUIJN_MN_64_SHIFT];
#endif

		}

		inline
			int msb64_de_Bruijn(const BITBOARD bb_dato) {

			if (bb_dato == 0) return EMPTY_ELEM;

			//creates all 1s up to MSB position
			BITBOARD bb = bb_dato;
			bb |= bb >> 1;
			bb |= bb >> 2;
			bb |= bb >> 4;
			bb |= bb >> 8;
			bb |= bb >> 16;
			bb |= bb >> 32;

			//applys same computation as for LSB-de Bruijn
			return Tables::indexDeBruijn64_SEP[(bb * DEBRUIJN_MN_64_SEP) >> DEBRUIJN_MN_64_SHIFT];
		}
	}//end namespace bblock

}//end namespace bitgraph


#endif
