/**  
 * @file config.h
 * @brief configuration parameters for the BITSCAN 1.0 library
 * @author pss
 * @created ?
 * @last_update 09/02/2025
 * 
 * Permission to use, modify and distribute this software is
 * granted provided that this copyright notice appears in all 
 * copies, in source code or in binaries. For precise terms 
 * see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any 
 * kind, express or implied, and with no claim as to its
 * suitability for any purpose.
 *
 **/

using bitgraph::WORD_SIZE;

////////////////////
//DEBUG mode assertions

// uncomment for assertions in debug mode
//#undef NDEBUG

#define USE_BITWISE_OPS

/////////////
//Popcount
//(switch in case processors do not support operations for population count)

#define POPCOUNT_INTRINSIC_64								//uses HW assembler operation _popcn64 function (most efficient - DEFAULT)
//#undef  POPCOUNT_INTRINSIC_64								//will use other population-count functions (lookup table...)


/////////////
//Bit-scanning implementation choices 

#define DE_BRUIJN
#ifndef DE_BRUIJN
	#define LOOKUP
#endif

#ifdef DE_BRUIJN
	#define ISOLANI_LSB										//b&(-b) implementation
    #undef  ISOLANI_LSB										//b^(b-1) implementation (DEFAULT)
#endif


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

#ifndef BITSCAN_BIT_OPS_DEFINED
#define BITSCAN_BIT_OPS_DEFINED
namespace bitgraph {
	// ONLY WORKS FOR 64-bit WORD_SIZE!
	namespace bit_ops {
		// Which word (64-bit block) contains bit i?
		// Equivalent to i/64 but using bit shift (faster)
		inline constexpr int block_index(int bit) noexcept {
			return bit >> 6;  // Compiler optimizes to single shift instruction
		}
		
		// Which position within that word (0-63)?
		// Equivalent to i%64 but using bit masking (faster)
		inline constexpr int bit_offset(int bit) noexcept {
			return bit & 0x3F;  // Mask with 111111 binary to get lower 6 bits
		}
		
		// Starting bit index of word/block i
		// Equivalent to i*64 but using bit shift (faster)
		inline constexpr int block_to_bit(int block) noexcept {
			return block << 6;  // Compiler optimizes to single shift instruction
		}
		
		// Alternative to WMOD that avoids modulo operation entirely
		// Computes bit offset using subtraction instead of modulo
		inline constexpr int bit_offset_alt(int bit) noexcept {
			return bit - block_to_bit(block_index(bit));
		}
	}
}
#endif // BITSCAN_BIT_OPS_DEFINED


#ifdef USE_BITWISE_OPS

#define WDIV(i) (bitgraph::bit_ops::block_index(i))
#define WMOD(i) (bitgraph::bit_ops::bit_offset(i))
#define WMUL(i) (bitgraph::bit_ops::block_to_bit(i))
#define WMOD_MUL(i) (bitgraph::bit_ops::bit_offset_alt(i))

//MACROS for mapping bit index to bitblock index (0 or 1 based)
#define INDEX_0TO0(p)			(WDIV(p))									//p>0  0-based to 0-based
#define INDEX_0TO1(p)			(WDIV(p)+1)									//p>0  0-based to 1-based
#define INDEX_1TO1(p)			(bitgraph::bit_ops::block_index((p)-1)+1)    //p>0  1-based to 1-based
#define INDEX_1TO0(p)			(bitgraph::bit_ops::block_index((p)-1))		//p>0  1-based to 0-based

#else
// OLD IMPLEMENTATION -> kept for compatibility, testing and dinamic WORD_SIZE
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

