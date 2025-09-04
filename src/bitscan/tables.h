/*  
 * tables.h file from the BITSCAN library, a C++ library for bit 
 * sets optimization. It has been used to implement BBMC, a very
 * succesful bit-parallel algorithm for exact maximum clique. 
 * (see license file for references)
 *
 * Copyright (C)
 * Author: Pablo San Segundo
 * Intelligent Control Research Group (CSIC-UPM) 
 *
 * Permission to use, modify and distribute this software is
 * granted provided that this copyright notice appears in all 
 * copies, in source code or in binaries. For precise terms 
 * see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any 
 * kind, express or implied, and with no claim as to its
 * suitability for any purpose.
 */

#ifndef __TABLES_H__
#define __TABLES_H__

#include "bbtypes.h"
#include <array>

namespace bitgraph {

	namespace _impl {

		class Tables{

		private:
		
			Tables(){};
			virtual ~Tables(){};
		
		public:
		
			static int InitAllTables();							//Driver for all inits
		
		private:
		
			static void init_masks();
			static void init_popc8();
			static void init_popc();
			static void init_mlsb();
			static void init_lsb_l();							//Conditioned to EXTENDED_LOOKUPS 

			//Table
			static void init_cached_index();					//Conditioned to CACHED_INDEX_OPERATIONS 
		
		////////////////////////////////////////
		//data members
		
		public:
		
			/*Example
			  Purpose: Instantly isolate or select specific bit
		  	  ranges without computation.
		  		- mask[5] = 0x0000000000000020 (solo bit 5 set)
		  		- mask_low[5] = 0x000000000000001F (bits 0-4 set)
		  	*/
		
			static std::array<BITBOARD, 64> mask;						//masks for 64 bit block of a single bit
			static std::array<U8, 8> mask8;								//masks for 8 bit block of a single bit
			static std::array<BITBOARD, 65> mask_low;					//1_bits to the right of index (less significant bits, excluding index) - mask_low[WORD_SIZE] = bitgraph::constants::ALL_ONES
			static std::array<BITBOARD, 66> mask_high;					//1_bits to the left of index (more significant bits, excluding index)
		
		//private:
			static std::array<std::array<BITBOARD, /*b*/64>, /*a*/64> mask_mid;	//1-bits between intervals (a<=b). All bits between positions a and b
		
			//0 but word masks
			static BITBOARD  mask0_1W;						
			static BITBOARD  mask0_2W;
			static BITBOARD  mask0_3W;
			static BITBOARD	 mask0_4W;
		
			static std::array<int, 65536> pc;				//16 bit population count  "how many 1-bits are in this number?" 
			static std::array<int, 65536> lsb;				//LSB lookup table 16 bits
			static std::array<int, 65536> msb;				//MSB for 16 bits
			static std::array<int, 256> pc8;				//population count for 8 bits
			static std::array<int, 65536> pc_sa;			//populaton count for 16 bits (Shift + Add)
		
			static std::array<std::array<int, 65536>, 4> lsba;						//LSB lookup table 16 bits con pos indes
			static std::array<std::array<int, 65536>, 4> msba;						//MSB lookup table 16 bits con pos indes							
		
		#ifdef EXTENDED_LOOKUPS	
			static std::array<std::array<int, 16>, 65536> lsb_l;			//LSB for 16 bits list of position of 1-bits)
		#endif
		
			//64 bit block index cache
		#ifdef  CACHED_INDEX_OPERATIONS 
			static int t_wdindex[MAX_CACHED_INDEX];			
			static int t_wxindex[MAX_CACHED_INDEX];			
			static int t_wmodindex[MAX_CACHED_INDEX];
		#endif
		
		////////////////////////
		//magic number tables
		
		    //4BYTES (for 32 bits)
		    static constexpr std::array<int,37> T_32={	
		        -1,0,1,26,2,23,27,-1,			
		        3,16,24,30,28,11,-1,13,
		        4,7,17,-1,25,22,31,15,
		        29,10,12,6,-1,21,14,9,	
		        5,20,8,19,18				
		    };
		
		    //8BYTES (64 bits)
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
		
			//bit scan with b&(-b)	returns most right 1
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
		
			//bit scan with b^(b-1) returns flipped most right 1 -1
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
		
		};

	};
	
};

#endif