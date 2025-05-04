/**
 * @file bitblock.cpp
 * @brief Implementation of bitblock operations for the BITSCAN library
 * @details This file implements optimized functions for manipulating 64-bit bitblocks
 * @author pss
 * @date Created prior to 2023
 * @date Last updated: 30/01/2025 (refactored, added trimming functions, improved Doxygen tags)
 **/

#include "bitblock.h"
#include <iostream>

using namespace std;


namespace bblock {

	/**
	 * Implementation of popc64_lup function
	 * Uses a lookup table to count set bits by breaking the 64-bit value
	 * into four 16-bit chunks and summing the population counts
	 */
	int popc64_lup(const BITBOARD bb_dato) {

		register union x {
			U16 c[4];
			BITBOARD b;
		} val;

		val.b = bb_dato;

		return (Tables::pc[val.c[0]] + Tables::pc[val.c[1]] + Tables::pc[val.c[2]] + Tables::pc[val.c[3]]); //Sum of populations 
	}


	/**
	 * Implementation of lsb64_lup function
	 * Uses a lookup table to find the least significant bit
	 * by checking each 16-bit chunk of the 64-bit value
	 */
	int lsb64_lup(const BITBOARD bb) {
		U16 bb16;

		if (bb) {
			bb16 = (U16)bb;
			if (bb16) return (Tables::lsba[0][bb16]);
			bb16 = (U16)(bb >> 16);
			if (bb16) return (Tables::lsba[1][bb16]);
			bb16 = (U16)(bb >> 32);
			if (bb16) return (Tables::lsba[2][bb16]);
			bb16 = (U16)(bb >> 48);
			if (bb16) return (Tables::lsba[3][bb16]);
		}

		return EMPTY_ELEM;		//should not occur
	}


	/**
	 * Implementation of lsb64_pc function
	 * Uses population count to find the least significant bit
	 * by isolating the LSB and counting bits before it
	 */
	int  lsb64_pc(const BITBOARD bb_dato) {

		if (bb_dato) {
			return popc64((bb_dato & -bb_dato) - 1);
		}
		return EMPTY_ELEM;
	}


	/**
	 * Implementation of lsb64_mod function
	 * Uses perfect hashing with modulo 67 to find the least significant bit
	 */
	int lsb64_mod(const BITBOARD bb_dato) {

		if (bb_dato) {
			return(Tables::T_64[(bb_dato & (~bb_dato + 1)) % 67]);
		}

		return EMPTY_ELEM;
	}

	/**
	 * Implementation of lsb64_lup_eff function
	 * More efficient implementation of lookup-based LSB detection
	 * with direct chunk comparison
	 */
	int  lsb64_lup_eff(const BITBOARD bb_dato) {


		register union x {
			U16 c[4];
			BITBOARD b;
		}val;
		val.b = bb_dato;	//union load

		//Control
		if (bb_dato) {
			if (val.c[0])
				return (Tables::lsb[val.c[0]]);
			else if (val.c[1])
				return (Tables::lsb[val.c[1]] + 16);
			else if (val.c[2])
				return (Tables::lsb[val.c[2]] + 32);
			else return (Tables::lsb[val.c[3]] + 48);		//nonzero by (1)
		}

		return EMPTY_ELEM;		//Should not reach
	}

    //////////////////////////////
    //
    // PopCount
    //

	
	/**
	 * Implementation of popc64_lup_1 function
	 * Alternative lookup table population count with direct bit shifting
	 * instead of using a union
	 */
	int popc64_lup_1(const BITBOARD bb_dato) {
		return (Tables::pc[(U16)bb_dato] + Tables::pc[(U16)(bb_dato >> 16)] +
			Tables::pc[(U16)(bb_dato >> 32)] + Tables::pc[(U16)(bb_dato >> 48)]);
	}

	/**
	 * Implementation of msb64_lup function
	 * Uses a lookup table to find the most significant bit
	 * by checking each 16-bit chunk from highest to lowest
	 */
	int msb64_lup(const BITBOARD bb) {

		register union x {
			U16 c[4];
			BITBOARD b;
		} val;
		val.b = bb;

		//if(bb==0) return -1;				//for sparse data

		if (val.b) {
			if (val.c[3]) return (Tables::msba[3][val.c[3]]);
			if (val.c[2]) return (Tables::msba[2][val.c[2]]);
			if (val.c[1]) return (Tables::msba[1][val.c[1]]);
			if (val.c[0]) return (Tables::msba[0][val.c[0]]);
		}

		/*	if (val.c[3])							//access table msb[65536](valores:0-15) in blocks of 16 bits
				return (msb[val.c[3]] + 48);
			else if (val.c[2])
					return (msb[val.c[2]] + 32);
			else if (val.c[1])
					return (msb[val.c[1]] + 16);
			else  return (msb[val.c[0]]);			//should be nonzero by (1)*/

		return EMPTY_ELEM;							//should not reach here
	}
		

    //////
    // I/O

	
	/**
	 * Implementation of print function
	 * Prints the indices of all set bits in the bitblock
	 * followed by the total bit count in brackets
	 */
	std::ostream& print( BITBOARD bb_data, std::ostream& o, bool endofl)
	{

		if (bb_data) {

			int nBit = EMPTY_ELEM;
			BITBOARD bb = bb_data;

			//bitscanning loop
			while ((nBit = lsb64_de_Bruijn(bb)) != EMPTY_ELEM)
			{
				o << nBit << " ";

				//removes the bit (XOR operation)
				bb ^= Tables::mask[nBit];
			}
		}

		//adds population count
		o << "[" << popc64(bb_data) << "]";
		
		if (endofl) { o << std::endl; }

		return o;

	}

	/**
	 * Implementation of copy function
	 * Copies bits from source to destination in the specified range
	 * preserving bits outside this range in the destination
	 */
	void copy	 (int firstBit, int lastBit, const BITBOARD& source,  BITBOARD& dest)
	{
		auto destOri = dest;
	
		//delete left of lastBit and right of firstBit
		dest = source & MASK_1(firstBit, lastBit);
		
		//add left of lastBit and right of firstBit
		dest |= destOri & Tables::mask_high [lastBit];
		dest |= destOri & Tables::mask_low[firstBit];
	}

	/**
	 * Implementation of copy_high function
	 * Copies bits from position 'bit' to the end (63) from source to destination
	 * preserving bits before 'bit' in the destination
	 */
	void copy_high (int bit, const BITBOARD& source, BITBOARD& dest)
	{
		auto destOri = dest;

		//copy the good part of the block (bit included)
		dest = source & MASK_1_HIGH(bit);

		//add right part of the block (excluding bit)
		dest |= destOri & Tables::mask_low[bit];

	}

	/**
	 * Implementation of copy_low function
	 * Copies bits from the beginning (0) to position 'bit' from source to destination
	 * preserving bits after 'bit' in the destination
	 */
	void copy_low(int bit, const BITBOARD& source, BITBOARD& dest)
	{
		auto destOri = dest;

		//copy the good part of the block (bit included)
		dest = source & MASK_1_LOW(bit);

		//add right part of the block (excluding bit)
		dest |= destOri & Tables::mask_high[bit];
	}

}//end namespace bblock



