/**
 * @file tables.h
 * @brief Precomputed tables for the BITSCAN library
 * @version 1.0
 * @details Contains lookup tables for various bitscanning operations
 * @author Pablo San Segundo
 * @date 2014
 * @last_update 2025-02-08
 * 
 * @copyright Copyright (C) Pablo San Segundo, Intelligent Control Research Group (CSIC-UPM)
 * @note Permission to use, modify and distribute this software is granted provided this copyright
 * notice appears in all copies. For precise terms see the accompanying LICENSE file.
 * 
 * @warning This software is provided "AS IS" with no warranty of any kind, express or implied,
 * and with no claim as to its suitability for any purpose.
 */

#ifndef __TABLES_H__
#define __TABLES_H__

#include "bbtypes.h"

/**
 * @class Tables
 * @brief Contains static lookup tables for various bit operations
 * @details Used for optimizing bit manipulation operations in the BITSCAN library
 */
class Tables{
		
private:
	/**
	 * @brief Private constructor - all methods are static
	 */
	Tables(){};
	
	/**
	 * @brief Virtual destructor
	 */
	virtual ~Tables(){};

public:
	/**
	 * @brief Initializes all lookup tables
	 * @return Always returns 1 (success)
	 */
	static int InitAllTables();

private:
	/**
	 * @brief Initializes bit mask tables
	 */
	static void init_masks();
	
	/**
	 * @brief Initializes 8-bit population count table
	 */
	static void init_popc8();
	
	/**
	 * @brief Initializes population count tables
	 */
	static void init_popc();
	
	/**
	 * @brief Initializes MSB lookup tables
	 */
	static void init_mlsb();
	
	/**
	 * @brief Initializes LSB lookup tables
	 * @note Only used when EXTENDED_LOOKUPS is defined
	 */
	static void init_lsb_l();
	
	/**
	 * @brief Initializes cached index tables
	 * @note Only used when CACHED_INDEX_OPERATIONS is defined
	 */
	static void init_cached_index();

////////////////////////////////////////
// Data members

public:
	/**
	 * @brief Single-bit masks for 64-bit blocks
	 * @details mask[i] has the i-th bit set to 1, all others to 0
	 */
	static BITBOARD mask[64];
	
	/**
	 * @brief Single-bit masks for 8-bit blocks
	 * @details mask8[i] has the i-th bit set to 1, all others to 0
	 */
	static U8 mask8[8];
	
	/**
	 * @brief Masks for bits to the right of a given index
	 * @details mask_low[i] has all bits 0 to i-1 set to 1, others to 0
	 */
	static BITBOARD mask_low[65];
	
	/**
	 * @brief Masks for bits to the left of a given index
	 * @details mask_high[i] has all bits i+1 to 63 set to 1, others to 0
	 */
	static BITBOARD mask_high[66];

	/**
	 * @brief Masks for bits between two indices
	 * @details mask_mid[a][b] has all bits from a to b set to 1, others to 0
	 */
	static BITBOARD mask_mid[64/*a*/][64/*b*/];

	/**
	 * @brief Zero masks for different word sizes
	 * @{
	 */
	static BITBOARD mask0_1W;
	static BITBOARD mask0_2W;
	static BITBOARD mask0_3W;
	static BITBOARD mask0_4W;
	/** @} */

	/**
	 * @brief 16-bit population count table
	 * @details pc[i] contains the number of 1-bits in the binary representation of i
	 */
	static int pc[65536];
	
	/**
	 * @brief 16-bit least significant bit lookup table
	 * @details lsb[i] contains the position of the least significant bit in i
	 */
	static int lsb[65536];
	
	/**
	 * @brief LSB lookup tables with position index
	 */
	static int lsba[4][65536];
	
	/**
	 * @brief MSB lookup tables with position index
	 */
	static int msba[4][65536];
	
	/**
	 * @brief 8-bit population count table
	 * @details pc8[i] contains the number of 1-bits in the binary representation of i
	 */
	static int pc8[256];
	
	/**
	 * @brief 16-bit population count table using shift-add method
	 */
	static int pc_sa[65536];
	
	/**
	 * @brief 16-bit most significant bit lookup table
	 * @details msb[i] contains the position of the most significant bit in i
	 */
	static int msb[65536];

#ifdef EXTENDED_LOOKUPS	
	/**
	 * @brief Extended LSB lookup table
	 * @details Contains lists of positions of 1-bits
	 */
	static int lsb_l[65536][16];
#endif

////////////////////////
//magic number tables	
	/**
	 * @brief Magic number table for 32-bit operations
	 */
	static const int T_32[37];
	
	/**
	 * @brief Magic number table for 64-bit operations
	 */
	static const int T_64[67];
	
	/**
	 * @brief De Bruijn sequence for isolation method (b&(-b))
	 */
	static const int indexDeBruijn64_ISOL[64];
	
	/**
	 * @brief De Bruijn sequence for separation method (b^(b-1))
	 */
	static const int indexDeBruijn64_SEP[64];

#ifdef CACHED_INDEX_OPERATIONS 
	/**
	 * @brief Cached division table
	 */
	static int t_wdindex[MAX_CACHED_INDEX];
	
	/**
	 * @brief Cached multiplication table
	 */
	static int t_wxindex[MAX_CACHED_INDEX];
	
	/**
	 * @brief Cached modulo table
	 */
	static int t_wmodindex[MAX_CACHED_INDEX];
#endif

};

#endif // __TABLES_H__