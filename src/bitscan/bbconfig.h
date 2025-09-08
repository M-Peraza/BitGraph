/**  
 * @file bbconfig.h
 * @brief Configuration parameters and compile-time settings for BITSCAN 1.0
 * @author Pablo San Segundo  
 * @version 1.0
 * @date 2025
 * @since 09/02/2025
 * 
 * @details This file controls the compile-time behavior of the BITSCAN library
 * through preprocessor macros and constexpr functions. It allows optimization
 * for different hardware architectures and use cases.
 * 
 * Key configuration areas:
 * - Hardware instruction selection (intrinsics vs. lookup tables)
 * - Bit manipulation algorithm choices  
 * - Performance optimization toggles
 * - Debug and assertion controls
 * 
 * @warning Changing these settings affects library performance and compatibility.
 * Test thoroughly on target hardware after modifications.
 * 
 * Permission to use, modify and distribute this software is granted provided 
 * that this copyright notice appears in all copies, in source code or in 
 * binaries. For precise terms see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any kind, express 
 * or implied, and with no claim as to its suitability for any purpose.
 **/

using bitgraph::WORD_SIZE;

/**
 * @defgroup DebugConfig Debug and Assertion Configuration
 * @brief Controls for debugging and runtime assertions
 * @{
 */

/** 
 * @brief Enable runtime assertions in debug builds
 * @details Uncomment the line below to enable assertions even in release builds.
 * This will add runtime checks but may impact performance.
 */
// #undef NDEBUG

/** @} */ // end DebugConfig group

/**
 * @defgroup OptimizationConfig Performance Optimization Settings
 * @brief Compile-time switches for performance tuning
 * @{
 */

/** 
 * @brief Use optimized bitwise operations instead of lookup tables
 * @details When defined, uses constexpr bitwise functions for index calculations.
 * Generally faster than lookup tables and avoids cache misses.
 */
#define USE_BITWISE_OPS

/** @} */ // end OptimizationConfig group

/**
 * @defgroup PopcountConfig Population Count Implementation
 * @brief Hardware vs. software population count selection
 * @details Controls whether to use hardware instructions or software fallback
 * for counting 1-bits in bitboards.
 * @{
 */

/** 
 * @brief Use hardware population count instructions (recommended)
 * @details Uses efficient _popcnt64 hardware instruction when available.
 * This is the most efficient implementation for modern processors.
 */
#define POPCOUNT_INTRINSIC_64

/** 
 * @brief Use software population count fallback
 * @details Uncomment to use lookup table-based population count.
 * Use only if target processor lacks POPCNT instruction.
 */
//#undef  POPCOUNT_INTRINSIC_64

/** @} */ // end PopcountConfig group

/**
 * @defgroup BitscanConfig Bit Scanning Implementation Selection
 * @brief Algorithm selection for finding first/last set bits
 * @details Controls the algorithm used for LSB/MSB operations in bitboards.
 * @{
 */

/** 
 * @brief Use De Bruijn multiplication for bit scanning (recommended)
 * @details Efficient algorithm using De Bruijn sequences for bit position lookup.
 */
#define DE_BRUIJN

#ifndef DE_BRUIJN
	/** @brief Use lookup table method as fallback */
	#define LOOKUP
#endif

#ifdef DE_BRUIJN
	/** 
	 * @brief Use b&(-b) isolate LSB implementation  
	 * @details Alternative De Bruijn approach. Comment out for default b^(b-1) method.
	 */
	#define ISOLANI_LSB
    #undef  ISOLANI_LSB  // Use b^(b-1) implementation (DEFAULT)
#endif

/** @} */ // end BitscanConfig group


////////////////////
//Memory allignment	
		
//#define _MEM_ALIGNMENT 				32					//change this for different allignments (set to 64 bits? - 01/02/2025)
//#undef  _MEM_ALIGNMENT									//DEFAULT - currently MUST BE DISABLED , _TODO CHECK (01/02/2025)
 

//////////////////////
// Configuration of precomputed tables

/*
#define CACHED_INDEX_OPERATIONS								//uses extra storage space for fast bitscanning 
#undef  CACHED_INDEX_OPERATIONS								//comment for fast bitscanning	(DEFAULT-  uncommented)

#ifdef CACHED_INDEX_OPERATIONS
	constexpr int MAX_CACHED_INDEX = 15001;					//size of cached WMOD, WDIV, WMUL indexes (number of bits)
#endif
*/

/*Testear las operaciones de abajo, si funcionan deberian ser mas rapidas
inline constexpr int wdiv(const int& i) noexcept{
	return i/WORD_SIZE;
}

inline constexpr int wmod(const int& i) noexcept{
	return i%WORD_SIZE;
}

inline constexpr int wmul(const int& i) noexcept{
	return i*WORD_SIZE;
}

inline constexpr int wmod_mul(const int& i) noexcept{
	return i-wmul(wdiv(i));
}*/

// Type safe al contratio que simplemente las macros, 
// mas rapido tanto que las operaciones directas (espceialmente lenta el modulo) como las tablas(lentas si no estan en cache)
// usamos inline(compilador), constexpr(tiempo compilado) y noexcept(relajamos el stack) para optimizar aun mas

/**
 * @defgroup BitOperations High-Performance Bit Operations
 * @brief Optimized bit manipulation functions for 64-bit architectures
 * @details These functions provide type-safe, compile-time optimized alternatives
 * to macro-based operations. They leverage bit shifting and masking for maximum
 * performance, avoiding expensive division and modulo operations.
 * 
 * Performance benefits:
 * - Type safety compared to plain macros
 * - Faster than direct arithmetic operations (especially modulo)
 * - Faster than lookup tables when not in cache
 * - Uses inline, constexpr, and noexcept for optimal compilation
 * @{
 */

#ifndef BITSCAN_BIT_OPS_DEFINED
#define BITSCAN_BIT_OPS_DEFINED
namespace bitgraph {
	/**
	 * @namespace bitgraph::bit_ops
	 * @brief High-performance bit manipulation operations
	 * @warning ONLY WORKS FOR 64-bit WORD_SIZE! Do not use with other word sizes.
	 */
	namespace bit_ops {
		/**
		 * @brief Determine which 64-bit block contains the given bit
		 * @param bit The bit index (0-based)
		 * @return Block index containing the bit
		 * @details Equivalent to bit/64 but uses bit shift for optimal performance.
		 * The compiler optimizes this to a single shift instruction.
		 * @note Complexity: O(1) - Single CPU instruction
		 */
		inline constexpr int block_index(int bit) noexcept {
			return bit >> 6;  // Right shift by 6 equals division by 64
		}
		
		/**
		 * @brief Get bit position within its 64-bit block (0-63)
		 * @param bit The bit index (0-based)  
		 * @return Position within the block (0-63)
		 * @details Equivalent to bit%64 but uses bit masking for optimal performance.
		 * Mask 0x3F = 111111 in binary extracts the lower 6 bits.
		 * @note Complexity: O(1) - Single CPU instruction
		 */
		inline constexpr int bit_offset(int bit) noexcept {
			return bit & 0x3F;  // Mask with 0x3F to get remainder of division by 64
		}
		
		/**
		 * @brief Get the starting bit index of a given block
		 * @param block The block index (0-based)
		 * @return Starting bit index of the block 
		 * @details Equivalent to block*64 but uses bit shift for optimal performance.
		 * The compiler optimizes this to a single shift instruction.
		 * @note Complexity: O(1) - Single CPU instruction
		 */
		inline constexpr int block_to_bit(int block) noexcept {
			return block << 6;  // Left shift by 6 equals multiplication by 64
		}
		
		/**
		 * @brief Alternative bit offset calculation avoiding modulo entirely
		 * @param bit The bit index (0-based)
		 * @return Position within the block (0-63)
		 * @details Computes bit offset using subtraction: bit - (block * 64).
		 * May be more efficient than bit_offset() in some compiler/architecture combinations.
		 * @note Complexity: O(1) - Two CPU instructions (block_index + subtraction)
		 */
		inline constexpr int bit_offset_alt(int bit) noexcept {
			return bit - block_to_bit(block_index(bit));
		}
	}
}
/** @} */ // end BitOperations group
#endif // BITSCAN_BIT_OPS_DEFINED


#ifdef USE_BITWISE_OPS

/**
 * @defgroup CoreBitMacros Core Bit Manipulation Macros
 * @brief Primary macros for bit-to-block index conversion
 * @details These macros provide the fundamental operations used throughout
 * BITSCAN for converting between bit indices and block indices. They use
 * the optimized bit_ops functions when USE_BITWISE_OPS is defined.
 * @{
 */

/** @brief Get block index containing bit i (Word DIVision) */
#define WDIV(i) (bitgraph::bit_ops::block_index(i))

/** @brief Get bit position within block (Word MODulo) */
#define WMOD(i) (bitgraph::bit_ops::bit_offset(i))

/** @brief Get starting bit of block i (Word MULtiplication) */
#define WMUL(i) (bitgraph::bit_ops::block_to_bit(i))

/** @brief Alternative WMOD avoiding modulo (Word MODulo via MULtiplication) */
#define WMOD_MUL(i) (bitgraph::bit_ops::bit_offset_alt(i))

/** @} */ // end CoreBitMacros group

/**
 * @defgroup IndexMacros Index Conversion Macros
 * @brief Macros for converting between different indexing schemes
 * @details These macros handle conversion between 0-based and 1-based indexing
 * for bit positions and block indices. Essential for interfacing with different
 * data structures and external APIs.
 * @{
 */

/** @brief Convert 0-based bit index to 0-based block index */
#define INDEX_0TO0(p)			(WDIV(p))

/** @brief Convert 0-based bit index to 1-based block index */  
#define INDEX_0TO1(p)			(WDIV(p)+1)

/** @brief Convert 1-based bit index to 1-based block index */
#define INDEX_1TO1(p)			(bitgraph::bit_ops::block_index((p)-1)+1)

/** @brief Convert 1-based bit index to 0-based block index */
#define INDEX_1TO0(p)			(bitgraph::bit_ops::block_index((p)-1))

/** @} */ // end IndexMacros group

#else
// CODIGO ORIGINAL -> se deja por compatitibilidad y testing
#ifdef  CACHED_INDEX_OPERATIONS 
#define WDIV(i) (Tables::t_wdindex[(i)])
#define WMOD(i) (Tables::t_wmodindex[(i)])
#define WMUL(i) (Tables::t_wxindex[(i)])
#else

#define WDIV(i) ((i)/WORD_SIZE)                               // Which word contains bit i?
#define WMOD(i) ((i)%WORD_SIZE)                               // Which position within that word?
#define WMUL(i) ((i)*WORD_SIZE)                               // Starting bit of word i
#define WMOD_MUL(i) ((i)-WMUL(WDIV(i)))						  // WMOD without modulo operation

#define INDEX_0TO0(p)			(WDIV(p))					//p>0  
#define INDEX_0TO1(p)			(WDIV(p)+1)					//p>0  
#define INDEX_1TO1(p)			((((p)-1)/WORD_SIZE)+1)		//p>0  
#define INDEX_1TO0(p)			((((p)-1)/WORD_SIZE))		//p>0  

#endif
#endif

//Bitscan operations which used extended lookups
//have been removed in this release
#define EXTENDED_LOOKUPS									//uses extra storage space for fast lookup 
#undef	EXTENDED_LOOKUPS									//comment for fast bitset operations - DEFAULT, DO NOT CHANGE!	

