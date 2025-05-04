/**  
 * @file bbtypes.h
 * @brief Type definitions and constants for the BITSCAN library
 * @version 1.0
 * @details BITSCAN is a C++ library for bit set optimization. BITSCAN has been used to implement BBMC, 
 * a very successful bit-parallel algorithm for exact maximum clique. (see license file for references)
 * 
 * @author Pablo San Segundo
 * @date 2014
 * @last_update 2025-02-08
 * 
 * @copyright Copyright (C) Pablo San Segundo, Intelligent Control Research Group (CSIC-UPM)
 * @note Permission to use, modify and distribute this software is granted provided that this 
 * copyright notice appears in all copies, in source code or in binaries. For precise terms 
 * see the accompanying LICENSE file.
 * 
 * @warning This software is provided "AS IS" with no warranty of any kind, express or implied, 
 * and with no claim as to its suitability for any purpose.
 **/

#ifndef __BBTYPES_H__
#define __BBTYPES_H__
		
/**
 * @typedef U8
 * @brief Type alias for unsigned 8-bit integer
 */
using U8 = unsigned char;

/**
 * @typedef U16
 * @brief Type alias for unsigned 16-bit integer
 */
using U16 = unsigned short;

/**
 * @typedef U32
 * @brief Type alias for unsigned 32-bit integer
 */
using U32 = unsigned long;

/**
 * @typedef BITBOARD
 * @brief Type alias for 64-bit unsigned integer used to represent bitboards
 */
using BITBOARD = unsigned long long;

/**
 * @typedef BYTE
 * @brief Type alias for unsigned 8-bit integer
 */
using BYTE = unsigned char;

/**
 * @typedef BOOL
 * @brief Type alias for integer used as boolean
 * @deprecated Use bool instead (as of 2025-02-08)
 */
using BOOL = int;

/**
 * @def ONE
 * @brief Constant for a full 64-bit mask (all bits set to 1)
 */
constexpr BITBOARD ONE = 0xFFFFFFFFFFFFFFFF;

/**
 * @def EVEN
 * @brief Constant for a 64-bit mask with bits set in even positions
 */
constexpr BITBOARD EVEN = 0x5555555555555555;

/**
 * @def EVEN_32
 * @brief Constant for a 32-bit mask with bits set in even positions
 */
constexpr U32 EVEN_32 = 0x55555555;

/**
 * @def EVEN_8
 * @brief Constant for an 8-bit mask with bits set in even positions
 */
constexpr U8 EVEN_8 = 0x55;

/**
 * @def ZERO
 * @brief Constant for an empty 64-bit mask (all bits set to 0)
 */
constexpr BITBOARD ZERO = 0x0000000000000000;

/**
 * @def EMPTY_ELEM
 * @brief Constant representing an empty table element, bitboard or bitstring
 */
constexpr int EMPTY_ELEM = -1;

/**
 * @def WORD_SIZE
 * @brief Size of the register word in bits
 * @warning Do not change this value!
 */
constexpr int WORD_SIZE = 64;

/**
 * @def WORD_SIZE_MINUS_ONE
 * @brief Word size minus one, used in various bit operations
 */
constexpr int WORD_SIZE_MINUS_ONE = WORD_SIZE - 1;

/**
 * @def MASK_LIM
 * @brief Mask limit for bitscan operations of a single BITBOARD
 */
constexpr int MASK_LIM = WORD_SIZE + 1;

#endif