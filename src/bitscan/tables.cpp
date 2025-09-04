#include "tables.h"
#include <algorithm>

using namespace bitgraph;
using bitgraph::_impl::Tables;

//common masks and lookup tables always available
std::array<int, 65536> Tables::pc;								//1_bit population in 16 bit blocks
std::array<int, 65536> Tables::lsb;								//LSB lookup table 16 bits
std::array<int, 65536> Tables::msb;								//MSB in 16 bit blocks	
std::array<int, 256> Tables::pc8;								//population of 1-bits en U8
std::array<int, 65536> Tables::pc_sa;							//population of 1-bits en BITBOARD16 (Shift + Add implementations)
std::array<std::array<int, 65536>, 4> Tables::msba;				//MSB lookup table 16 bits con pos index
std::array<std::array<int, 65536>, 4> Tables::lsba;				//LSB lookup table 16 bits con pos index

std::array<BITBOARD, 64> Tables::mask;						//masks for 64 bit block of a single bit
std::array<BITBOARD, 65> Tables::mask_low;					//1_bit to the right of index (less significant bits, excluding index)
std::array<BITBOARD, 66> Tables::mask_high;
std::array<U8, 8> Tables::mask8;						    //masks for 8 bit block of a single bit			
std::array<std::array<BITBOARD, /*b*/64>, /*a*/64> Tables::mask_mid;		//1-bits between intervals

BITBOARD	Tables::mask0_1W;						
BITBOARD	Tables::mask0_2W;
BITBOARD	Tables::mask0_3W;
BITBOARD	Tables::mask0_4W;

constexpr std::array<int, 37> Tables::T_32;
constexpr std::array<int, 67> Tables::T_64;
constexpr std::array<int, 64> Tables::indexDeBruijn64_ISOL;
constexpr std::array<int, 64> Tables::indexDeBruijn64_SEP;


#ifdef CACHED_INDEX_OPERATIONS 
std::array<int, MAX_CACHED_INDEX> Tables::t_wdindex;
std::array<int, MAX_CACHED_INDEX> Tables::t_wxindex;
std::array<int, MAX_CACHED_INDEX> Tables::t_wmodindex;
#endif

//extended lookups
#ifdef EXTENDED_LOOKUPS				
std::array<std::array<int, 16>, 65536> Tables::lsb_l;				//LSB position list of 1-bits in BITBOARD16
#endif

//global initialization of tables
struct Init{
	Init(){Tables::InitAllTables();}
} initTables;



////////////////////
// magic number tables of 64 bits (always available since space requierement is trivial)


/////////////////////////////////////

void Tables::init_masks(){

	BITBOARD uno = 1;
	// Crea todas las poiciones de 1s en un objeto de 64 bits
	  for (size_t c = 0; c < mask.size(); ++c) {
        mask[c] = uno << c;
    }

	////////////////////////////
	//mask8[8]
 	U8 uno8 = 1;
    for (size_t c = 0; c < mask8.size(); ++c) {
        mask8[c] = uno8 << c;
    }

	////////////////////////////
	//mask_low[65]
	mask_low.fill(0); //necesario para la operacion de mask_low |= mask[j]
	//BITBOARD aux=0;

	for (int c=0; c < 64; ++c)
	{
		for (int j=0; j<c /* not included the element itself*/; j++)
		{
			mask_low[c] |= mask[j];

		}
	}
	//mask_low[0]=bitgraph::constants::ALL_ZEROS;
	mask_low[64]=bitgraph::constants::ALL_ONES;

	////////////////////////////
	//mask_high[64] (from mask_low)
	for (int c=0; c<64; c++)
	{
		mask_high[c]= ~mask_low[c] ^ mask[c]/*erase the element itself*/;
	}
	
	mask_high[64]=bitgraph::constants::ALL_ZEROS;
	mask_high[MASK_LIM]=bitgraph::constants::ALL_ONES;

	
	////////////////////////////
	//mask_mid[64][64]

	  for (int c = 0; c < 64; ++c) {
        for (int j = 0; j < 64; ++j) {
            if (j < c) continue;
            
            if (j == c) {
                mask_mid[c][j] = mask[c];
            } else {
                mask_mid[c][j] = mask_low[j] & mask_high[c] | mask[c] | mask[j];
            }
        }
    }

	/////////////////////////////
	//m�scara de 0s
	
	mask0_1W= bitgraph::constants::ALL_ONES << 16;
	mask0_2W= (mask0_1W<<16) | (~mask0_1W);
	mask0_3W= (mask0_2W<<16) | (~mask0_1W);
	mask0_4W= (mask0_3W<<16) | (~mask0_1W);
}

/////////////////////////////////
// Inicio Tabla PopCount (poblacion 8 bits)

void Tables::init_popc8(){
	
	pc8[0] = 0;  // null bit population
    
    for (size_t c = 1; c < pc8.size(); ++c) {
        int n = 0;
        for (int k = 0; k <= 7; ++k) {
            if (c & (1 << k)) {
                ++n;
            }
        }
        pc8[c] = n;
    }
}

/////////////////////////////////
// Table begin PopCount (16 bits population)

void Tables::init_popc(){
////////////////////
//
	pc.fill(0);  
    
	size_t c;
	int n, k;

    for (c = 1; c < pc.size(); ++c) {
        n = 0;
        for (k = 0; k <= 15; ++k) {
            if (c & (1 << k)) {
                ++n;
            }
        }
        pc[c] = n;
    }


	//Implementacion Shift+Add: 
	pc_sa.fill(0);					//null bits population

	for (c=1; c < pc.size(); c++)	{
		n=0;
		for(k=0; k<13; k+=4)
				n+=0xF & (c>>k);  //Sum of the number of bits every 4
		
		pc_sa[c]=n;
	}
}

////////////////////////////////////////
//  Tables for MSB / LSB

void Tables::init_mlsb(){
////////////////////////
// builds tables for most significant bit and least signifincat bit fast bitscan
	int k;
	size_t c;

	msb[0] = EMPTY_ELEM;				//no 1-bit
	for (c=1; c < msb.size(); c++){
		for (k=15;k>=0;k--){
			 if (c & (1/*int*/ << k)) {
				 msb[c] = k;
				break;
			}
		}
	}

	lsb[0]= EMPTY_ELEM;					//no 1-bit
	for (c=1; c < lsb.size(); c++){
		for (k=0;k<16;k++)	{
			 if (c & (1/*int*/ << k)) {
				 lsb[c] = k;	//0-15
				 break;
			}
		}
	}

	//lsb with position index
    for (size_t k = 0; k < lsba.size(); ++k) {
        for (size_t c = 1; c < lsba[k].size(); ++c) {
            lsba[k][c] = lsb[c] + k * 16;
        }
        lsba[k][0] = EMPTY_ELEM;
    }			

	//msb with position index
    for (size_t k = 0; k < msba.size(); ++k) {
        for (size_t c = 1; c < msba[k].size(); ++c) {
            msba[k][c] = msb[c] + k * 16;
        }
        msba[k][0] = EMPTY_ELEM;
    }		
}

void Tables::init_lsb_l(){
////////////////////
// 16 bits conversion to a list of numbers
//
#ifdef EXTENDED_LOOKUPS

	for (size_t c = 0; c < lsb_l.size(); ++c) {
        for (size_t k = 0; k < lsb_l[c].size(); ++k) {
            if (c & mask[k]) {
                lsb_l[c][k] = k;
            } else {
                lsb_l[c][k] = 100;  // upper bound
            }
        }
    }

    // Aqui la ganancia en expresividad respescto a lo anterior es muy grande
    for (auto& row : lsb_l) {
        std::sort(row.begin(), row.end());
    }

    for (auto& row : lsb_l) {
        std::replace(row.begin(), row.end(), 100, EMPTY_ELEM);
    }

#endif 	
}



void Tables::init_cached_index()
{
//index tables

#ifdef  CACHED_INDEX_OPERATIONS 
	for(int i=0; i<MAX_CACHED_INDEX; i++) 
						Tables::t_wdindex[i]=i/WORD_SIZE;


	for(int i=0; i<MAX_CACHED_INDEX; i++) 
						Tables::t_wxindex[i]=i*WORD_SIZE;


	for(int i=0; i<MAX_CACHED_INDEX; i++) 
						Tables::t_wmodindex[i]=i%WORD_SIZE;
#endif
}


//boot tables in RAM

int Tables::InitAllTables(){
	init_mlsb();
    init_popc();
    init_popc8();
    init_masks(); 
	init_lsb_l();
	init_cached_index();

return 1;
}
