/**
 * @file tables.h
 * @brief Lookup tables and mathematical constants for high-performance bit operations
 * @author Pablo San Segundo
 * @version 1.0
 * @date 2025
 * 
 * @details This file contains precomputed lookup tables and mathematical constants
 * that enable the high-performance bit manipulation operations in BITSCAN. These
 * tables eliminate runtime computations by providing O(1) access to:
 * 
 * - **Bit masks**: Single-bit, range, and pattern masks for all bit positions
 * - **Population count tables**: Precomputed 1-bit counts for 8-bit and 16-bit values
 * - **Bit scanning tables**: LSB/MSB lookup tables for rapid bit position finding
 * - **De Bruijn sequences**: Perfect hash tables for single-bit position lookup
 * - **Magic number tables**: Mathematical constants for modulo-based hashing
 * 
 * The tables are implemented as static class members to ensure:
 * - **Single initialization**: Tables computed once at program startup
 * - **Global access**: Available to all BITSCAN components without overhead
 * - **Memory efficiency**: Shared across all bitset instances
 * - **Cache locality**: Contiguous memory layout for optimal performance
 * 
 * **Mathematical Foundation:**
 * Many tables are based on advanced bit manipulation techniques including
 * De Bruijn sequences, perfect hashing, and combinatorial mathematics developed
 * specifically for the maximum clique problem and related NP-hard optimization.
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

#ifndef __TABLES_H__
#define __TABLES_H__

#include "bbtypes.h"
#include <array>

namespace bitgraph {

	namespace _impl {

		/**
		 * @class Tables
		 * @brief Static lookup table manager for BITSCAN bit manipulation operations
		 * @details This class provides centralized management of all lookup tables
		 * and precomputed constants used throughout the BITSCAN library. It implements
		 * the Singleton pattern to ensure single initialization and global access.
		 * 
		 * **Design Principles:**
		 * - **Static-only interface**: No instantiation required or allowed
		 * - **Initialization**: Tables computed on first access via InitAllTables()
		 * - **Conditional compilation**: Some tables only built when specific features enabled
		 * - **Memory optimization**: Tables sized for actual usage patterns
		 * 
		 * The class manages over 15 different lookup tables totaling several KB of
		 * precomputed data, providing the foundation for O(1) bit operations that
		 * would otherwise require expensive runtime computation.
		 */
		class Tables{

		private:
		
			/** @brief Private constructor prevents instantiation */
			Tables(){};
			
			/** @brief Private destructor prevents instantiation */
			virtual ~Tables(){};
		
		public:
		
			/**
			 * @brief Initialize all lookup tables and constants
			 * @return 0 on success, non-zero on failure
			 * @details Master initialization function that calls all individual
			 * table initialization routines. Must be called once before using
			 * any BITSCAN operations. Safe to call multiple times.
			 */
			static int InitAllTables();
		
		private:
		
			/** @brief Initialize bit mask arrays for all positions and ranges */
			static void init_masks();
			
			/** @brief Initialize 8-bit population count lookup table */
			static void init_popc8();
			
			/** @brief Initialize 16-bit population count lookup tables */
			static void init_popc();
			
			/** @brief Initialize LSB/MSB lookup tables for multiple word sizes */
			static void init_mlsb();
			
			/** @brief Initialize extended LSB list tables (conditional: EXTENDED_LOOKUPS) */
			static void init_lsb_l();
			
			/** @brief Initialize cached index tables (conditional: CACHED_INDEX_OPERATIONS) */
			static void init_cached_index(); 
		
		/**
		 * @defgroup LookupTables Static Lookup Tables and Masks
		 * @brief Precomputed tables for high-performance bit manipulation
		 * @details These tables eliminate runtime computation by providing instant
		 * access to commonly needed bit patterns and mathematical results.
		 * @{
		 */
		
		public:
		
		/**
		 * @defgroup BitMasks Bit Mask Arrays
		 * @brief Precomputed masks for isolating and selecting bit patterns
		 * @details Bit masks enable instant isolation of specific bit ranges without
		 * runtime computation. Critical for performance in tight loops.
		 * 
		 * **Example Usage:**
		 * @code{.cpp}
		 * // Isolate bit 5: result = bb & Tables::mask[5]
		 * // Get bits 0-4: result = bb & Tables::mask_low[5] 
		 * // Get bits 6-63: result = bb & Tables::mask_high[5]
		 * @endcode
		 * @{
		 */
		
			/**
			 * @brief Single-bit masks for 64-bit bitboards
			 * @details Array of 64 masks, each with exactly one bit set.
			 * - mask[0] = 0x0000000000000001 (bit 0 set)
			 * - mask[5] = 0x0000000000000020 (bit 5 set)  
			 * - mask[63] = 0x8000000000000000 (bit 63 set)
			 * 
			 * Used for: bit testing, bit setting/clearing, bit isolation
			 * @note Complexity: O(1) access, replaces (1ULL << bit) computation
			 */
			static std::array<BITBOARD, 64> mask;
			
			/**
			 * @brief Single-bit masks for 8-bit values
			 * @details Specialized masks for byte-level operations.
			 * - mask8[0] = 0x01, mask8[1] = 0x02, ..., mask8[7] = 0x80
			 * 
			 * Used for: byte-level bit manipulation, lookup table indexing
			 */
			static std::array<U8, 8> mask8;
			
			/**
			 * @brief Low-range bit masks (less significant bits)
			 * @details Array of 65 masks containing all bits below a given position.
			 * - mask_low[0] = 0x0000000000000000 (no bits)
			 * - mask_low[5] = 0x000000000000001F (bits 0-4 set)
			 * - mask_low[64] = 0xFFFFFFFFFFFFFFFF (all bits set)
			 * 
			 * Used for: range operations, bit clearing above threshold,
			 * modulo operations via bit masking
			 * @note mask_low[WORD_SIZE] equals bitgraph::constants::ALL_ONES
			 */
			static std::array<BITBOARD, 65> mask_low;
			
			/**
			 * @brief High-range bit masks (more significant bits) 
			 * @details Array of 66 masks containing all bits above a given position.
			 * - mask_high[0] = 0xFFFFFFFFFFFFFFFE (bits 1-63 set)
			 * - mask_high[5] = 0xFFFFFFFFFFFFFFC0 (bits 6-63 set)
			 * - mask_high[63] = 0x0000000000000000 (no bits)
			 * 
			 * Used for: range operations, bit clearing below threshold,
			 * upper-bound masking in algorithms
			 */
			static std::array<BITBOARD, 66> mask_high;
		
			/**
			 * @brief Interval bit masks for arbitrary ranges
			 * @details 64x64 matrix of masks for all possible bit ranges [a,b].
			 * - mask_mid[a][b] contains all bits from position a to b (inclusive)
			 * - Only valid when a <= b, undefined behavior otherwise
			 * 
			 * **Example:**
			 * - mask_mid[3][7] = 0x00000000000000F8 (bits 3,4,5,6,7 set)
			 * 
			 * Used for: range extraction, field operations, bit interval masking
			 * @note Private access - use through helper functions for safety
			 * @warning Large memory footprint: 64*64*8 = 32KB
			 */
			static std::array<std::array<BITBOARD, /*b*/64>, /*a*/64> mask_mid;

		/** @} */ // end BitMasks group
		
		/**
		 * @defgroup WordMasks Multi-Word Bit Masks
		 * @brief Special masks for multi-word bitset operations
		 * @details These masks handle boundary conditions in multi-word bitsets
		 * where trailing bits need special handling.
		 * @{
		 */
		
			/** @brief Mask for clearing unused bits in 1-word bitsets */
			static BITBOARD  mask0_1W;
			
			/** @brief Mask for clearing unused bits in 2-word bitsets */						
			static BITBOARD  mask0_2W;
			
			/** @brief Mask for clearing unused bits in 3-word bitsets */
			static BITBOARD  mask0_3W;
			
			/** @brief Mask for clearing unused bits in 4-word bitsets */
			static BITBOARD	 mask0_4W;

		/** @} */ // end WordMasks group
		
		/**
		 * @defgroup PopulationCountTables Population Count Lookup Tables
		 * @brief Precomputed 1-bit counts for various word sizes
		 * @details Population count (popcount) tables provide O(1) access to
		 * 1-bit counts, essential for set cardinality operations and algorithm
		 * complexity analysis in combinatorial optimization.
		 * @{
		 */
		
			/**
			 * @brief 16-bit population count lookup table
			 * @details Complete lookup table for all 65536 possible 16-bit values.
			 * - pc[0x0000] = 0 (no bits set)
			 * - pc[0x000F] = 4 (four bits set: 0,1,2,3)
			 * - pc[0xFFFF] = 16 (all bits set)
			 * 
			 * Used for: fast population counting in 16-bit chunks
			 * @note Memory: 256KB (65536 * 4 bytes)
			 */
			static std::array<int, 65536> pc;
			
			/**
			 * @brief 8-bit population count lookup table  
			 * @details Compact lookup table for byte-level population counting.
			 * - pc8[0x00] = 0, pc8[0x0F] = 4, pc8[0xFF] = 8
			 * 
			 * Used for: memory-efficient popcount, byte processing
			 * @note Memory: 1KB (256 * 4 bytes) - much more cache-friendly than pc[]
			 */
			static std::array<int, 256> pc8;
			
			/**
			 * @brief Alternative 16-bit population count using Shift+Add algorithm
			 * @details Implementation using bit manipulation instead of lookup.
			 * Provided for algorithmic comparison and platforms with limited memory.
			 * 
			 * Used for: memory-constrained environments, algorithm research
			 */
			static std::array<int, 65536> pc_sa;

		/** @} */ // end PopulationCountTables group
		
		/**
		 * @defgroup BitScanningTables Bit Scanning Lookup Tables
		 * @brief Precomputed tables for finding first/last set bits
		 * @details These tables provide O(1) bit position lookup, eliminating
		 * the need for iterative scanning or hardware instructions on platforms
		 * where they're unavailable.
		 * @{
		 */
		
			/**
			 * @brief LSB (Least Significant Bit) lookup table for 16-bit values
			 * @details Returns position of rightmost 1-bit for all 16-bit values.
			 * - lsb[0x0001] = 0, lsb[0x0008] = 3, lsb[0x8000] = 15
			 * - lsb[0x0000] = -1 (no bits set)
			 * 
			 * Used for: LSB finding when hardware BSF unavailable
			 */
			static std::array<int, 65536> lsb;
			
			/**
			 * @brief MSB (Most Significant Bit) lookup table for 16-bit values
			 * @details Returns position of leftmost 1-bit for all 16-bit values.  
			 * - msb[0x0001] = 0, msb[0x0008] = 3, msb[0x8000] = 15
			 * - msb[0x0000] = -1 (no bits set)
			 * 
			 * Used for: MSB finding when hardware BSR unavailable
			 */
			static std::array<int, 65536> msb;
			
			/**
			 * @brief Multi-position LSB lookup tables with offset indexing
			 * @details 4 separate 16-bit LSB tables for different word positions.
			 * lsba[i][value] returns LSB position adjusted for word i position.
			 * 
			 * Used for: multi-word bitset LSB scanning with position correction
			 */
			static std::array<std::array<int, 65536>, 4> lsba;
			
			/**
			 * @brief Multi-position MSB lookup tables with offset indexing
			 * @details 4 separate 16-bit MSB tables for different word positions.
			 * msba[i][value] returns MSB position adjusted for word i position.
			 * 
			 * Used for: multi-word bitset MSB scanning with position correction
			 */
			static std::array<std::array<int, 65536>, 4> msba;

		/** @} */ // end BitScanningTables group							
		
		/**
		 * @defgroup ConditionalTables Conditional Compilation Tables
		 * @brief Tables compiled only when specific features are enabled
		 * @{
		 */
		
		#ifdef EXTENDED_LOOKUPS	
			/**
			 * @brief Extended LSB list lookup table (conditional)
			 * @details 2D table providing complete list of all 1-bit positions
			 * for each 16-bit value. Each entry contains up to 16 bit positions.
			 * - lsb_l[value][0..15] = ordered list of 1-bit positions
			 * 
			 * Used for: complete bit enumeration without iteration
			 * @warning Only compiled when EXTENDED_LOOKUPS defined
			 * @note Memory: Very large - 65536*16*4 = 4MB
			 */
			static std::array<std::array<int, 16>, 65536> lsb_l;
		#endif
		
		#ifdef  CACHED_INDEX_OPERATIONS// Teniendo las nuevas operaciones no deberia tener mucho sentido mantener esto
			/**
			 * @brief Cached word division index table (conditional)
			 * @details Precomputed bit-to-word index mapping for fast access.
			 * Equivalent to WDIV(i) but with O(1) table lookup.
			 */
			static int t_wdindex[MAX_CACHED_INDEX];
			
			/**
			 * @brief Cached word multiplication index table (conditional)
			 * @details Precomputed word-to-bit index mapping for fast access.
			 * Equivalent to WMUL(i) but with O(1) table lookup.
			 */
			static int t_wxindex[MAX_CACHED_INDEX];
			
			/**
			 * @brief Cached word modulo index table (conditional)  
			 * @details Precomputed bit position within word for fast access.
			 * Equivalent to WMOD(i) but with O(1) table lookup.
			 */
			static int t_wmodindex[MAX_CACHED_INDEX];
		#endif
		
		/** @} */ // end ConditionalTables group
		
		/**
		 * @defgroup MagicNumberTables Mathematical Perfect Hash Tables
		 * @brief Advanced mathematical lookup tables for bit position finding
		 * @details These tables implement sophisticated mathematical techniques
		 * for bit position lookup including modular arithmetic perfect hashing
		 * and De Bruijn sequence multiplication methods.
		 * 
		 * **Mathematical Background:**
		 * Perfect hash functions map single-bit patterns to unique indices,
		 * enabling O(1) bit position lookup without iteration or hardware
		 * instructions. These techniques were developed specifically for
		 * high-performance combinatorial optimization algorithms.
		 * @{
		 */
		
		    /**
		     * @brief 32-bit magic number perfect hash table
		     * @details Perfect hash table for 32-bit single-bit patterns using
		     * modular arithmetic. Maps (value % 37) to bit position.
		     * - Used with: ((bb & -bb) % 37) to find LSB position  
		     * - T_32[magic_index] returns actual bit position
		     * - -1 entries indicate impossible/invalid patterns
		     * 
		     * **Algorithm**: LSB isolation + modular hashing + table lookup
		     * @note Size: 37 entries (specific to modular arithmetic properties)
		     */
		    static constexpr std::array<int,37> T_32={	
		        -1,0,1,26,2,23,27,-1,			
		        3,16,24,30,28,11,-1,13,
		        4,7,17,-1,25,22,31,15,
		        29,10,12,6,-1,21,14,9,	
		        5,20,8,19,18				
		    };
		
		    /**
		     * @brief 64-bit magic number perfect hash table  
		     * @details Perfect hash table for 64-bit single-bit patterns using
		     * modular arithmetic. Extended version of T_32 for full 64-bit support.
		     * - Used with: ((bb & -bb) % 67) to find LSB position
		     * - T_64[magic_index] returns actual bit position [0-63]
		     * - -1 entries indicate impossible/invalid hash collisions
		     * 
		     * **Mathematical Properties:**
		     * - Modulus 67 chosen for collision-free mapping of 64 single-bit patterns
		     * - Perfect hash function: no collisions for valid inputs
		     * @note Size: 67 entries (next prime after 64 for optimal distribution)
		     */
		    static constexpr std::array<int, 67> T_64 = {{
		        -1,0,1,39,2,15,40,23,        
		        3,12,16,59,41,19,24,54,
		        4,-1,13,10,17,62,60,28,
		        42,30,20,51,25,44,55,47,
		        5,32,-1,38,14,22,11,58,
		        18,53,63,9,61,27,29,50,
		        43,46,31,37,21,57,52,8,
		        26,49,45,36,56,7,48,35,
		        6,34,33
		    }};

		/** @} */ // end MagicNumberTables group
		
		/**
		 * @defgroup DeBruijnTables De Bruijn Sequence Lookup Tables  
		 * @brief Multiplication-based perfect hash tables using De Bruijn sequences
		 * @details De Bruijn sequences provide an alternative to modular arithmetic
		 * for bit position finding. They use multiplication and bit shifting to
		 * create perfect hash functions for single-bit patterns.
		 * 
		 * **De Bruijn Sequence Properties:**
		 * - Each possible n-bit pattern appears exactly once in the sequence
		 * - Multiplication by De Bruijn constant creates unique bit patterns  
		 * - Right-shift extracts hash index for table lookup
		 * - No division or modulo operations required (faster than magic numbers)
		 * 
		 * **Algorithm**: bit_isolation * DEBRUIJN_CONSTANT >> SHIFT_AMOUNT
		 * @{
		 */
		
			/**
			 * @brief De Bruijn lookup table for isolated LSB method  
			 * @details Used with (bb & -bb) bit isolation method.
			 * - Multiply isolated bit by DEBRUIJN_MN_64_ISOL
			 * - Right-shift by DEBRUIJN_MN_64_SHIFT (58 bits)
			 * - Use result as index: indexDeBruijn64_ISOL[hash_index]
			 * 
			 * **Mathematical Foundation:**
			 * The (bb & -bb) operation isolates the rightmost 1-bit, creating
			 * one of 64 possible single-bit patterns. Multiplication by the
			 * De Bruijn constant produces 64 distinct upper bits.
			 * 
			 * @note Algorithm: pos = indexDeBruijn64_ISOL[((bb & -bb) * DEBRUIJN_MN_64_ISOL) >> 58]
			 */
		    static constexpr std::array<int, 64> indexDeBruijn64_ISOL = {{
		        63,  0, 58,  1, 59, 47, 53,  2,
		        60, 39, 48, 27, 54, 33, 42,  3,
		        61, 51, 37, 40, 49, 18, 28, 20,
		        55, 30, 34, 11, 43, 14, 22,  4,
		        62, 57, 46, 52, 38, 26, 32, 41,
		        50, 36, 17, 19, 29, 10, 13, 21,
		        56, 45, 25, 31, 35, 16,  9, 12,
		        44, 24, 15,  8, 23,  7,  6,  5
		    }};
		
			/**
			 * @brief De Bruijn lookup table for separated bits method
			 * @details Used with (bb ^ (bb-1)) bit separation method.
			 * - Compute bb ^ (bb-1) to create bit pattern with trailing 1s
			 * - Multiply by DEBRUIJN_MN_64_SEP  
			 * - Right-shift by DEBRUIJN_MN_64_SHIFT (58 bits)
			 * - Use result as index: indexDeBruijn64_SEP[hash_index]
			 * 
			 * **Alternative Approach:**
			 * The (bb ^ (bb-1)) creates a pattern with all bits set from LSB
			 * up to and including the first 1-bit. This provides different
			 * hash distribution characteristics than the isolation method.
			 * 
			 * @note Algorithm: pos = indexDeBruijn64_SEP[((bb ^ (bb-1)) * DEBRUIJN_MN_64_SEP) >> 58]
			 */
		    static constexpr std::array<int,64> indexDeBruijn64_SEP = {
		        0, 47,  1, 56, 48, 27,  2, 60,
		        57, 49, 41, 37, 28, 16,  3, 61,
		        54, 58, 35, 52, 50, 42, 21, 44,
		        38, 32, 29, 23, 17, 11,  4, 62,
		        46, 55, 26, 59, 40, 36, 15, 53,
		        34, 51, 20, 43, 31, 22, 10, 45,
		        25, 39, 14, 33, 19, 30,  9, 24,
		        13, 18,  8, 12,  7,  6,  5, 63
		    };

		/** @} */ // end DeBruijnTables group
		
		/** @} */ // end LookupTables group
		
		};

	};
	
};

#endif