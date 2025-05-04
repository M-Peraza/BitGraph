/**
 * @file bitscan.h
 * @brief Main header file for the BITSCAN library
 * @version 1.0
 * @details The BITSCAN library is a C++ library for bit set optimization.
 * @date 2014
 * @last_update 2025-02-12
 * @author pss
 **/

#include "bbsentinel.h"			//base of the non-sparse hierarchy
#include "bbscan_sparse.h"	

/**
 * @brief Useful aliases for BITSCAN types
 * @{
 */

/**
 * @typedef bbo
 * @brief Alias for the base class of the BITSCAN hierarchy
 */
using bbo = BBObject;

/**
 * @name Non-sparse bitset types
 * @{
 */

/**
 * @typedef simple_bitarray
 * @brief Simple bitarray type without advanced bitscanning
 */
using simple_bitarray = BitSet;

/**
 * @typedef bitarray
 * @brief Standard bitarray type with optimized bitscanning
 */
using bitarray = BBScan;

/**
 * @typedef watched_bitarray
 * @brief Bitarray that uses sentinels to watch empty bit ranges
 */
using watched_bitarray = BBSentinel;
/** @} */

/**
 * @name Sparse bitset types
 * @{
 */

/**
 * @typedef simple_sparse_bitarray
 * @brief Simple sparse bitarray type without advanced bitscanning
 */
using simple_sparse_bitarray = BitSetSp;

/**
 * @typedef sparse_bitarray
 * @brief Sparse bitarray with optimized bitscanning for large, sparse sets
 */
using sparse_bitarray = BBScanSp;
/** @} */

/** @} */




