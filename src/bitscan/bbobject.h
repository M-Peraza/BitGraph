/**  
 * @file bbobject.h
 * @brief Interface for the BITSCAN library hierarchy of classes
 * @version 1.0
 * @author pss
 * @date 2014
 * @last_update 2025-02-14
 **/

#ifndef  __BB_OBJECT_H__
#define  __BB_OBJECT_H__

#include <iostream>
#include "bbtypes.h"
#include "bbconfig.h"

//////////////////
//
// BBObject class
// (abstract class - base of the bitstring hierarchy)
//
//////////////////

/**
 * @class BBObject
 * @brief Abstract base class for the bitstring hierarchy
 * @details Provides the interface for all bitset types in the BITSCAN library
 */
class BBObject{
public:
	/**
	 * @brief Constant representing no bit (invalid bit position)
	 */
	static const int noBit = -1;

	/**
	 * @brief Enumeration of different bit scan types
	 */
	enum scan_types	{
		/**
		 * @brief Non-destructive forward scan 
		 * @details Scans without modifying the bitset
		 */
		NON_DESTRUCTIVE, 
		
		/**
		 * @brief Non-destructive reverse scan
		 * @details Scans in reverse without modifying the bitset
		 */
		NON_DESTRUCTIVE_REVERSE, 
		
		/**
		 * @brief Destructive forward scan
		 * @details Removes bits as they are scanned
		 */
		DESTRUCTIVE, 
		
		/**
		 * @brief Destructive reverse scan
		 * @details Removes bits as they are scanned in reverse
		 */
		DESTRUCTIVE_REVERSE
	};				

	/**
	 * @brief Stream output operator
	 * @param o Output stream
	 * @param bb BBObject to output
	 * @return Modified output stream
	 */
	friend std::ostream& operator<< (std::ostream& o , const BBObject& bb)		{ bb.print(o); return o;}

//////////////////
// Efficient nested data structures for bitscanning 
	
	//cache for bitscanning
	/**
	 * @class scan_t
	 * @brief Cache structure for efficient bitscanning
	 * @details Maintains state information during bitscanning operations
	 */
	struct scan_t {																			
		
		int bbi_;								//bitblock index 	
		int pos_;								//bit index [0...63] 

		/**
		 * @brief Default constructor
		 */
		scan_t() :bbi_(noBit), pos_(MASK_LIM)		{}
				
		/**
		 * @brief Sets the block index
		 * @param block Block index to set
		 */
		void set_block(int block)					{ bbi_ = block; }
		/**
		 * @brief Sets the bit position
		 * @param bit Bit position to set
		 */
		void set_pos(int bit)						{ pos_ = bit; }
		/**
		 * @brief Sets both block and position from a global bit index
		 * @param bit Global bit index
		 */
		void set_bit(int bit) {
			int bbh = WDIV(bit);
			bbi_ = bbh;
			pos_ = bit - WMUL(bbh);					/* equiv. to WMOD(bit)*/
		}
	};
	
	/////////////////////////////
	//
	// BitScanning Friendly classes
	// (similar to iterator models)
	// 
	// Currently, BitSet_t can only be an object derived from BBScan class (14/02/25)
	//
	////////////////////////////
	/**
	 * @class ScanRev
	 * @brief Reverse bitscanning iterator
	 * @tparam BitSet_t Type of bitset to scan (must be derived from BBScan)
	 * @details Provides an iterator-like interface for reverse bitscanning
	 */
	template< class BitSet_t >
	struct ScanRev {
		using basic_type = BitSet_t;
		using type = ScanRev<BitSet_t>;

	public:

		/**
		 * @brief Constructor
		 * @param bb Bitset to scan
		 * @param firstBit Starting bit for the scan (-1 means scan from the end)
		 */
		ScanRev(BitSet_t& bb, int firstBit = -1) : bb_(bb)	{ init_scan(firstBit);	}

		/**
		 * @brief Gets the current block index being scanned
		 * @return Current block index
		 */
		int get_block() { return  bb_.scan_.bbi_;}
		
		/**
		 * @brief Initializes a reverse scan from the given bit position
		 * @param firstBit Starting position for the scan (-1 means scan from the end)
		 * @return First bit found, or noBit if none found
		 */
		int init_scan(int firstBit = -1) { return bb_.init_scan(firstBit, BBObject::NON_DESTRUCTIVE_REVERSE ); }

		/**
		 * @brief Gets the next (previous) bit in the reverse scan
		 * @return Next bit found, or noBit if no more bits
		 */
		int next_bit() { return bb_.prev_bit(); }
		
		/**
		 * @brief Gets the next (previous) bit and deletes it from the specified bitset
		 * @param bitSet Bitset to delete the bit from
		 * @return Next bit found, or noBit if no more bits
		 */
		int next_bit(BitSet_t& bitSet) { return bb_.prev_bit(bitSet); }

	private:		
		BitSet_t& bb_;		
	};

	
	/**
	 * @class Scan
	 * @brief Forward bitscanning iterator
	 * @tparam BitSet_t Type of bitset to scan (must be derived from BBScan)
	 * @details Provides an iterator-like interface for forward bitscanning
	 */
	template< class BitSet_t >
	struct Scan {
		using basic_type = BitSet_t;
		using type = Scan<BitSet_t>;

	public:

		/**
		 * @brief Constructor
		 * @param bb Bitset to scan
		 * @param firstBit Starting bit for the scan (-1 means scan from the beginning)
		 */
		Scan(BitSet_t& bb, int firstBit = -1) : bb_(bb) { init_scan(firstBit); }

		/**
		 * @brief Gets the current block index being scanned
		 * @return Current block index
		 */
		int get_block() { return  bb_.scan_.bbi_; }

		/**
		 * @brief Initializes a forward scan from the given bit position
		 * @param firstBit Starting position for the scan (-1 means scan from the beginning)
		 * @return First bit found, or noBit if none found
		 */
		int init_scan(int firstBit = -1) { return bb_.init_scan(firstBit, BBObject::NON_DESTRUCTIVE); }

		/**
		 * @brief Gets the next bit in the forward scan
		 * @return Next bit found, or noBit if no more bits
		 */
		int next_bit() { return bb_.next_bit(); }

		/**
		 * @brief Gets the next bit and deletes it from the specified bitset
		 * @param bitSet Bitset to delete the bit from
		 * @return Next bit found, or noBit if no more bits
		 */
		int next_bit(BitSet_t& bitSet) { return bb_.next_bit(bitSet); }

	private:
		BitSet_t& bb_;
	};

	
	/**
	 * @class ScanDest
	 * @brief Destructive forward bitscanning iterator
	 * @tparam BitSet_t Type of bitset to scan (must be derived from BBScan)
	 * @details Provides an iterator-like interface for destructive forward bitscanning
	 */
	template< class BitSet_t >
	struct ScanDest {
		using basic_type = BitSet_t;
		using type = ScanDest<BitSet_t>;

	public:

		/**
		 * @brief Constructor
		 * @param bb Bitset to scan
		 */
		ScanDest(BitSet_t& bb) : bb_(bb) { init_scan(); }

		/**
		 * @brief Gets the current block index being scanned
		 * @return Current block index
		 */
		int get_block() { return bb_.scan_.bbi_;}

		/**
		 * @brief Scans the bitset in the range [0 , end of the bitset). Removes bits as they are scanned. Initializes a destructive forward scan
		 * @return First bit found, or noBit if none found
		 */
		int init_scan() { return bb_.init_scan(BBObject::DESTRUCTIVE); }

		/**
		 * @brief Gets the next bit in the scan and removes it from the bitset
		 * @return Next bit found, or noBit if no more bits
		 */
		int next_bit() { return bb_.next_bit_del(); }

		/**
		 * @brief Gets the next bit and deletes it from the specified bitset
		 * @param bitSet Bitset to delete the bit from
		 * @return Next bit found, or noBit if no more bits
		 */
		int next_bit(BitSet_t& bitSet) { return bb_.next_bit_del(bitSet); }

	private:		
		BitSet_t& bb_;
	};

	
	/**
	 * @class ScanDestRev
	 * @brief Destructive reverse bitscanning iterator
	 * @tparam BitSet_t Type of bitset to scan (must be derived from BBScan)
	 * @details Provides an iterator-like interface for destructive reverse bitscanning
	 */
	template< class BitSet_t >
	struct ScanDestRev {
		using basic_type = BitSet_t;
		using type = ScanDest<BitSet_t>;

	public:

		/**
		 * @brief Constructor
		 * @param bb Bitset to scan
		 */
		ScanDestRev(BitSet_t& bb) : bb_(bb) { init_scan(); }

		/**
		 * @brief Gets the current block index being scanned
		 * @return Current block index
		 */
		int get_block() { return bb_.scan_.bbi_;}

		/**
		 * @brief Scans the bitset in the range (end_of_bitset, 0]. Removes bits as they are scanned. Initializes a destructive reverse scan
		 * @return First bit found, or noBit if none found
		 */
		int init_scan() { return bb_.init_scan(BBObject::DESTRUCTIVE_REVERSE); }

		/**
		 * @brief Gets the next (previous) bit in the reversescan and removes it from the bitset
		 * @return Next bit found, or noBit if no more bits
		 */
		int next_bit() { return bb_.prev_bit_del(); }

		/**
		 * @brief Gets the next (previous) bit and deletes it from the specified bitset
		 * @param bitSet Bitset to delete the bit from
		 * @return Next bit found, or noBit if no more bits
		 */
		int next_bit(BitSet_t& bitSet) { return bb_.prev_bit_del(bitSet); }

	private:		
		BitSet_t& bb_;
	};


//////////////
//construction / destruction
	/**
	 * @brief Default constructor
	 */
	BBObject() = default;

	/**
	 * @brief Virtual destructor
	 */
	virtual ~BBObject() = default;

///////////////////
//I/O

	/**
	 * @brief Print the bitset to a stream
	 * @param o Output stream (default is std::cout)
	 * @param show_pc Whether to show the population count (default is true)
	 * @param endl Whether to end with a newline (default is true)
	 * @return Reference to the output stream
	 */
	virtual std::ostream& print(	std::ostream& o = std::cout, 
									bool show_pc = true,
									bool endl = true					) const = 0;
};



#endif