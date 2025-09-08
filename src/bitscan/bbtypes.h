/**
 * @file bbtypes.h
 * @brief Core type definitions for the BITSCAN library
 * @author Pablo San Segundo
 * @version 1.0
 * @date 2025
 * 
 * @details This file defines the fundamental data types used throughout 
 * the BITSCAN library for high-performance bitset operations. The library 
 * has been instrumental in implementing BBMC, a state-of-the-art bit-parallel 
 * algorithm for exact maximum clique problems.
 * 
 * The type system is designed for 64-bit architectures and leverages 
 * compile-time constants for optimal performance in combinatorial optimization.
 *
 * Copyright (C) Intelligent Control Research Group (CSIC-UPM)
 * 
 * Permission to use, modify and distribute this software is granted provided 
 * that this copyright notice appears in all copies, in source code or in 
 * binaries. For precise terms see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any kind, express 
 * or implied, and with no claim as to its suitability for any purpose.
 */

#ifndef __BBTYPES_H__
#define __BBTYPES_H__
		
namespace bitgraph {
	
	/**
	 * @brief Primary bitboard type for 64-bit bitset operations
	 * @details Used as the fundamental storage unit for bitsets. Each BITBOARD 
	 * can store 64 bits and is optimized for hardware bit manipulation instructions.
	 * This type is central to all BITSCAN operations and should be 64-bit aligned.
	 */
	using BITBOARD = unsigned long long;
	
	/**
	 * @brief 8-bit unsigned integer type
	 * @details Used for byte-level operations and memory-efficient counters
	 */
	using U8 = unsigned char;
	
	/**
	 * @brief 16-bit unsigned integer type  
	 * @details Used for intermediate calculations and lookup table indices
	 */
	using U16 = unsigned short;
	
	/**
	 * @brief 32-bit unsigned integer type
	 * @details Used for bit positions, block indices, and intermediate results
	 * from hardware bit scanning instructions
	 */
	using U32 = unsigned long;
	
	/**
	 * @brief Alias for 8-bit unsigned integer
	 * @details Alternative name for U8, used in specific contexts for clarity
	 */
	using BYTE = unsigned char;	
	
	/**
	 * @deprecated Use bool instead
	 * @brief Legacy boolean type - scheduled for removal
	 * @details This type exists for backward compatibility only. 
	 * New code should use the standard bool type.
	 */
	using BOOL = int;

// Optenemos type safety en tiempo de compilacion y optimizaciones en compilado gracias a constexpr, 
// mejoramos encapsulado en su namespace(mejor integracion con otras librerias etc..)
// ademas de (en mi opinion) mas expresividad

	/**
	 * @namespace bitgraph::constants
	 * @brief Compile-time constants for bitset operations
	 * @details This namespace provides type-safe constexpr constants that enable 
	 * compile-time optimizations and improve code expressiveness. These constants
	 * replace legacy macros with modern C++11 constexpr declarations.
	 */
	namespace constants {
		/**
		 * @brief Bitboard with all 64 bits set to 1
		 * @details Used for bit masking operations, bitset initialization, and 
		 * as a base for creating custom masks. Equivalent to binary: 1111...1111 (64 ones)
		 */
		constexpr BITBOARD ALL_ONES     = 0xFFFFFFFFFFFFFFFFULL;
		
		/**
		 * @brief Bitboard with all 64 bits set to 0  
		 * @details Used for bitset clearing operations and initialization.
		 * Equivalent to binary: 0000...0000 (64 zeros)
		 */
		constexpr BITBOARD ALL_ZEROS    = 0x0000000000000000ULL;
		
		/**
		 * @brief Mask for even-positioned bits (0, 2, 4, 6, ...)
		 * @details Used in bit manipulation algorithms requiring alternating patterns.
		 * Pattern: 0101010101... (alternating 0,1 starting with 1)
		 */
		constexpr BITBOARD EVEN_MASK    = 0x5555555555555555ULL;
		
		/**
		 * @brief 32-bit mask for even-positioned bits
		 * @details 32-bit version of EVEN_MASK for smaller bitset operations
		 */
		constexpr BITBOARD EVEN_MASK_32 = 0x55555555U;
		
		/**
		 * @brief 8-bit mask for even-positioned bits  
		 * @details 8-bit version of EVEN_MASK for byte-level operations
		 */
		constexpr BITBOARD EVEN_MASK_8  = 0x55U;
	}

	/**
	 * @namespace bitgraph::_impl  
	 * @brief Implementation details and internal constants
	 * @details This namespace contains internal constants used by BITSCAN 
	 * implementation. These are not part of the public API and may change 
	 * between versions.
	 */
	namespace _impl {
		/**
		 * @brief Sentinel value for empty or invalid elements
		 * @details Used to indicate empty table elements, invalid bitboard 
		 * positions, or uninitialized bitstring indices. Equivalent to BBObject::noBit.
		 */
		constexpr int EMPTY_ELEM = -1;

		/**
		 * @brief Size of the fundamental word in bits
		 * @details CRITICAL: This value must remain 64 for the entire BITSCAN 
		 * library to function correctly. All bit manipulation, indexing, and 
		 * hardware instruction assumptions depend on 64-bit words.
		 * @warning DO NOT CHANGE! The entire library assumes 64-bit architecture.
		 */
		constexpr int WORD_SIZE = 64;
		
		/**
		 * @brief WORD_SIZE - 1, used for bit masking operations
		 * @details Frequently used in bit position calculations. 
		 * Equal to 63, which in binary is 111111 (6 ones).
		 */
		constexpr int WORD_SIZE_MINUS_ONE = WORD_SIZE - 1;
		
		/**
		 * @brief Upper limit for mask operations on single bitboards
		 * @details Used as boundary value for bitscanning operations within 
		 * a single BITBOARD. Set to WORD_SIZE + 1 = 65.
		 */
		constexpr int MASK_LIM = WORD_SIZE + 1;
	}

	using _impl::WORD_SIZE;
	using _impl::EMPTY_ELEM;
}
#endif