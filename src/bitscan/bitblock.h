/** 
 * @file bitblock.h
 * @brief Header file for bitblock operations of the BITSCAN library
 * @details This file contains optimized functions for manipulating 64-bit bitblocks
 * @author pss
 * @date Created prior to 2023
 * @date Last updated: 30/01/2025 (refactored, added trimming functions, improved Doxygen tags)
 **/

#ifndef __BITBLOCK_H__
#define __BITBLOCK_H__

#include <iostream>
#include "bbtypes.h"
#include "bbconfig.h"
#include "tables.h"

///////////////////////////
//Platform-dependent dependency settings for basic lsb, msb and popcount bitblcok operations

#ifdef __GNUC__

#ifdef __MINGW64__
//	#define __popcnt64 __builtin_popcountll					/* removed 6/11/2017 for mingw_64 */
	#include <intrin.h>										//windows specific
#else
	#define __popcnt64 __builtin_popcountll			
	#include <x86intrin.h>										//linux specific
	 static inline unsigned char _BitScanForward64(unsigned long* Index,  unsigned long long  Mask)
		{
			unsigned long long  Ret;
			__asm__
			(
				"bsfq %[Mask], %[Ret]"
				:[Ret] "=r" (Ret)
				:[Mask] "mr" (Mask)
			);
			*Index = (unsigned long)Ret;
			return Mask?1:0;
		}
		static inline unsigned char _BitScanReverse64(unsigned long* Index,  unsigned long long  Mask)
		{
			 unsigned long long  Ret;
			__asm__
			(
				"bsrq %[Mask], %[Ret]"
				:[Ret] "=r" (Ret)
				:[Mask] "mr" (Mask)
			);
			*Index = (unsigned long)Ret;
			return Mask?1:0;
		}
#endif


#else
	#include <intrin.h>										//windows specific
#endif


/////////////////////////////////
//
// namespace bblock
// 
// (manages bitset optimizations in 64-bitsets)
//
///////////////////////////////////

/**
 * @namespace bblock
 * @brief Namespace containing optimized operations for 64-bit bitblocks
 * @details This namespace provides functions for bit manipulation, bit scanning, 
 *          population count, and mask generation for 64-bit bitblocks.
 *          It includes platform-dependent optimizations and multiple implementation 
 *          strategies (lookup tables, De Bruijn sequences, intrinsics).
 */
namespace bblock {

	/**
	 * @brief Magic constants for De Bruijn sequence-based bit scanning
	 * @details These constants are used in the De Bruijn bit scanning algorithms
	 *          for efficient bitblock operations without lookup tables
	 */
	//compile time globals for bitscanning operations in bitblocks
	constexpr unsigned long long DEBRUIJN_MN_64_ISOL = 0x07EDD5E59A4E28C2; /**< De Bruijn constant for isolated bit scanning */
	constexpr unsigned long long DEBRUIJN_MN_64_SEP = 0x03f79d71b4cb0a89;  /**< De Bruijn constant for separated bit scanning */
	constexpr unsigned long long DEBRUIJN_MN_64_SHIFT = 58;                 /**< Shift value for De Bruijn bit scanning */

    ////////////////////
    // Boolean
	/**
	 * @defgroup Boolean Boolean Operations
	 * @brief Functions for boolean operations on bitblocks
	 * @{
	 */

	/**
	* @brief Determines if a specific bit is set in the bitblock
	* @param bb The bitblock to check
	* @param bit The bit position to check [0...63]
	* @return TRUE if the bit is set, FALSE otherwise
	*/
	inline
	bool is_bit			(const BITBOARD bb, int bit)		 { return (bb & Tables::mask[bit]); }

	/** @} */ // end of Boolean group

/////////////////////
// BitScanning

	/**
	 * @defgroup BitScanning Bit Scanning Operations
	 * @brief Functions for finding the position of set bits in a bitblock
	 * @{
	 */

	/**
	* @brief Index of the least significant bit using modulo perfect hashing
	* @param bb Input 64-bit bitblock
	* @return Index of the least significant bit or -1 if empty
	* @details Uses T_64[67] table with the modulo operation to return the index of 
	*          the single 1-bit in bitblocks of form 000...1...000
	* @note Modulus operation is not efficient in most processors
	*/
	 int  lsb64_mod		(const BITBOARD bb);				
   
	/**
	* @brief Index of the least significant bit using a 16-bit lookup table
	* @param bb Input 64-bit bitblock
	* @return Index of the least significant bit or -1 if empty
	*/
	 int  lsb64_lup		(const BITBOARD bb);

	/**
	* @brief Index of the least significant bit using an efficient 16-bit lookup table
	* @param bb Input 64-bit bitblock
	* @return Index of the least significant bit or -1 if empty
	* @details Best implementation for 32 bits on x86 on average. Is worse than LSB_32x
	*          only for sparse bitblocks with 1-bits in the final portion
	*/
	 int  lsb64_lup_eff	(const BITBOARD bb);
	
	/**
	* @brief Index of the least significant bit of bb function implemented
	*		 as a population count operation 
	* @param bb: input 64-bit bitblock
	* @returns index of the least significant bit or -1 if empty
	* @date 2008
	*/
	 int  lsb64_pc			(const BITBOARD bb);

	/**
	* @brief Index of the least significant bit of bb function implemented
	*		 with a De Bruijn magic word for perfect hashing as 
	*		 a product and a shift operation.
	* @param bb: input 64-bit bitblock
	* @returns index of the least significant bit or -1 if empty
	* @date 2008
	* @details There are two implementations:
	*          a) ISOLANI_LSB with hashing bb &(-bb)
	*          b) All 1-bits to LSB with hashing bb^(bb-1)
	* 
	*          Option b) typically exploits the CPU hardware better on average
	*          and is defined as default.
	*          To change this option go to bbconfig.h file
	*/ 
 inline	 int lsb64_de_Bruijn (const BITBOARD bb);
	
	/**
	* @brief Index of the least significant bit using processor intrinsics
	* @param bb Input 64-bit bitblock
	* @return Index of the least significant bit or -1 if empty
	* @details Implemented by calling processor instructions through
	*          intrin.h (Windows) or x86intrin.h (Linux)
	*/
 inline int lsb64_intrinsic	(const BITBOARD bb);


	/**
	* @brief Index of the least significant bit (recommended function)
	* @param bb Input 64-bit bitblock
	* @return Index of the least significant bit or -1 if empty
	* @details This is a convenience function that calls the most efficient
	*          implementation (currently lsb64_intrinsic)
	*/
	inline
	int lsb					(const BITBOARD bb) { return 	lsb64_intrinsic(bb); }
	

	/**
	* @brief Index of the most significant bit using a 16-bit lookup table
	* @param bb Input 64-bit bitblock
	* @return Index of the most significant bit or -1 if empty
	*/
	 int msb64_lup			(const BITBOARD bb);
		
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
	 int  msb64_de_Bruijn	(const BITBOARD bb);		//De Bruijn magic word 
   
	/**
	* @brief Index of the most significant bit using processor intrinsics
	* @param bb Input 64-bit bitblock
	* @return Index of the most significant bit or -1 if empty
	* @details Implemented by calling processor instructions through
	*          intrin.h (Windows) or x86intrin.h (Linux)
	*/
	 inline  int msb64_intrinsic (const BITBOARD bb);
	

	/**
	* @brief Index of the most significant bit (recommended function) of bb
	* @param bb Input 64-bit bitblock
	* @return Index of the most significant bit or -1 if empty
	*/
	 inline  int msb		(const BITBOARD bb)		{ return msb64_intrinsic(bb); }

	/** @} */ // end of BitScanning group

/////////////////////
// Bit population
	
	/**
	 * @defgroup BitPopulation Bit Population Operations
	 * @brief Functions for counting set bits in a bitblock
	 * @{
	 */
	
	/**
	* @brief Population count using 16-bit lookup tables
	* @param bb Input 64-bit bitblock
	* @return Number of 1-bits in the bitblock
	* @details Default lookup table implementation for population count
	*/	
	 int popc64_lup			(const BITBOARD bb);		

	/**
	* @brief population count in bb implemented with 16-bit lookup tables
	*	     but no intermediate storage
	* @param bb: input 64-bit bitblock
	* @returns number of 1-bits in the bitblock
	**/
	 int popc64_lup_1		(const BITBOARD bb);	
		
	/**
	* @brief Default population count function (recommended)
	* @param bb Input 64-bit bitblock
	* @return Number of 1-bits in the bitblock
	* @details Calls assembler processor instructions if POPCOUNT_64 switch is ON (see config.h),
	*          otherwise uses a table lookup implementation.
	*          By default, POPCOUNT_64 switch is ON.
	*/
	////////////////////////////////////////////////
	 inline	 int popc64		(const BITBOARD bb);
	////////////////////////////////////////////////

	/**
	* @brief Alias for popc64() - gets the size (number of set bits) of a bitblock
	* @param bb Input 64-bit bitblock
	* @return Number of 1-bits in the bitblock
	*/
	inline
	int size(const BITBOARD bb) { return popc64(bb); }
	
	/** @} */ // end of BitPopulation group

//////////////////////
//  Masks

	/**
	 * @defgroup Masks Bitblock Mask Operations
	 * @brief Functions for creating and applying masks to bitblocks
	 * @{
	 */
	
	/**
	* @brief Creates a mask with a single bit set. Sets to 1 the bit passed and zero the rest of bits 
	* @param bit The bit position to set [0...63]
	* @return A 64-bit bitblock with only the specified bit set
	*/	  
	 inline
	BITBOARD MASK_BIT		(int bit)					 { return Tables::mask[bit]; }

	/**
	* @brief Creates a mask with bits set in the specified range. Sets to 1 the bits inside the closed range [low, high], sets to 0 the rest
	* @param low The lower bound bit position (inclusive) [0...63]
	* @param high The upper bound bit position (inclusive) [0...63]
	* @return A 64-bit bitblock with bits set in the specified range
	*/
	 inline
	 BITBOARD MASK_1	(int low, int high)				{ return Tables::mask_mid[low][high]; };

	/**
	* @brief Sets to 1 all bits in the closed range [0, 63]. Creates a mask with bits set from position 0 to idx
	* @param idx input reference bit position [0...63]
	* @return 64-bit bitblock mask. A 64-bit bitblock with bits set from 0 to idx
	*/
	 inline
	 BITBOARD MASK_1_LOW	(int idx)					{ return ~Tables::mask_high[idx]; }
	
	 /**
	 * @brief Sets to 1 all bits in the closed range [idx, 63]
	 * @param idx: input reference bit position [0...63]
	 * @returns 64-bit bitblock mask
	 **/
	 inline
	 BITBOARD MASK_1_HIGH	(int idx)			{ return ~Tables::mask_low[idx]; }

	/**
	* @brief Creates a mask with bits cleared in the specified range. Sets to 0 the bits inside the closed range [low, high], sets to 1 the rest
	* @param low The lower bound bit position (inclusive) [0...63]
	* @param high The upper bound bit position (inclusive) [0...63]
	* @return A 64-bit bitblock with bits cleared in the specified range
	*/
	 inline
	 BITBOARD MASK_0		(int low, int high) {  return ~Tables::mask_mid[low][high];	 }

	/**
	* @brief Creates a mask with bits cleared from position 0 to idx. Sets to 0 all bits in the closed range [0, idx]
	* @param idx Input reference bit position [0...63]. The upper bound bit position (inclusive) [0...63]
	* @return 64-bit bitblock mask.A 64-bit bitblock with bits cleared from 0 to idx
	*/
	 inline
	 BITBOARD MASK_0_LOW	(int idx)			{ return Tables::mask_high[idx]; }

	/**
	* @brief Creates a mask with bits cleared from position idx to 63. Sets to 0 all bits in the closed range [idx, 63]
	* @param idx The lower bound bit position (inclusive) [0...63]. Input reference bit position [0...63]
	* @return 64-bit bitblock mask. A 64-bit bitblock with bits cleared from idx to 63
	*/
	 inline
	 BITBOARD MASK_0_HIGH	(int idx)			{ return Tables::mask_low[idx]; }

	/**
	* @brief sets to 0 the bits of the bitblock bb to the right of index (the index-bit is not trimmed)
	* @param bb: input 64-bit bitblock
	* @param idx: position in the bitblock [0...63]
	* @returns the trimmed bitblock
	* @date 30/01/2015 
	**/	
	 inline
	 BITBOARD trim_low	(BITBOARD bb, int idx) { return bb &~ Tables::mask_low[idx]; }

	/**
	* @brief sets to 0 the bits of the bitblock bb to the left side of index (the index-bit is not trimmed)
	* @param bb: input 64-bit bitblock
	* @param idx: position in the bitblock [0...63]
	* @returns the trimmed bitblock
	* @date 30/01/2015 
	**/	
	 inline
	 BITBOARD trim_high		(BITBOARD bb, int idx) { return bb &~ Tables::mask_high[idx]; }

	/** @} */ // end of Masks group

	/**
	 * @defgroup Copy Bitblock Copy Operations
	 * @brief Functions for copying bits between bitblocks
	 * @{
	 */

	/**
	 * @brief Replaces bits in a specified range from source to destination bitblock
	 * @param firstBit The lower bound bit position (inclusive) [0...63]
	 * @param lastBit The upper bound bit position (inclusive) [0...63]
	 * @param source The source bitblock to copy bits from
	 * @param dest The destination bitblock where bits will be copied to
	 */
	 void copy				(int firstBit, int lastBit, const BITBOARD& source,  BITBOARD& dest);
	
	/**
	 * @brief Replaces bits in the range [bit, 63] from source to destination bitblock
	 * @param bit The lower bound bit position (inclusive) [0...63]
	 * @param source The source bitblock to copy bits from
	 * @param dest The destination bitblock where bits will be copied to
	 */
	 void copy_high			(int bit, const BITBOARD& source, BITBOARD& dest);
	
	/**
	 * @brief Replaces bits in the range [0, bit] from source to destination bitblock
	 * @param bit The upper bound bit position (inclusive) [0...63]
	 * @param source The source bitblock to copy bits from
	 * @param dest The destination bitblock where bits will be copied to
	 */
	 void copy_low			(int bit, const BITBOARD& source, BITBOARD& dest);

	/** @} */ // end of Copy group

/////////////////////
// I/O

	/**
	 * @defgroup IO Input/Output Operations
	 * @brief Functions for bitblock I/O operations
	 * @{
	 */

	/**
	 * @brief Prints the bitblock to the specified output stream
	 * @param bb_data The bitblock to print
	 * @param o The output stream to print to
	 * @param endofl Whether to add a newline character after printing
	 * @return The output stream after printing
	 * @details Prints the indices of set bits, followed by the total bit count in brackets
	 */
	std::ostream& print(BITBOARD bb_data, std::ostream& o = std::cout, bool endofl = true);

	/** @} */ // end of IO group

} //end namespace bblock



//////////////////////////////
// inline implementations in header file 

namespace bblock {

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


#endif
