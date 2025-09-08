/**
 * @file bbscan_sparse.h
 * @brief Sparse bitset with optimized scanning for low-density patterns
 * @author Pablo San Segundo
 * @version 1.0
 * @date 2025
 * @since 25/02/2025
 *
 * @details Provides BBScanSp class combining sparse bitset storage with
 * efficient cached bitscanning. Optimized for sparse bit patterns where
 * scanning performance matters and memory efficiency is critical.
 * 
 * **Optimization Target:** Low-density bitsets requiring frequent scanning
 * 
 * @todo Complete refactoring and comprehensive testing (25/02/2025)
 **/

#ifndef __BBSCAN_SPARSE_H__
#define __BBSCAN_SPARSE_H__

#include "bitscan/bbset_sparse.h"
#include "bbexcep_hand.h"
#include <cassert>

using namespace std;

namespace bitgraph {

	namespace _impl {

		/**
		 * @class BBScanSp
		 * @brief Sparse bitset with high-performance scanning capabilities
		 * @details Combines the memory efficiency of sparse bitset storage (BitSetSp)
		 * with cached scanning state for optimal performance in low-density scenarios.
		 * Uses specialized optimizations for sparse bit pattern scanning.
		 * 
		 * **Key Optimizations:**
		 * - Sparse storage model for memory efficiency
		 * - Cached scan state for performance
		 * - Skip-ahead algorithms for sparse patterns
		 * - Template integration with BBObject scanning classes
		 * 
		 * **Best Use Case:** Large bitsets with < 10% density requiring frequent scanning
		 * 
		 * **Inheritance:** Extends BitSetSp with scanning-specific functionality
		 */
		class BBScanSp : public BitSetSp {

		public:

			/**
			 * @defgroup SparseScanManagement Sparse Scan State Management and Construction
			 * @brief Functions for managing sparse scan state, construction, and initialization
			 * @details This group encompasses all operations related to sparse scan state management,
			 * object construction/destruction, and scanning initialization optimized for sparse bitsets.
			 * Includes specialized initialization that handles empty sparse blocks efficiently.
			 * 
			 * **Sparse-Specific Features:**
			 * - Empty sparse bitset handling in initialization
			 * - Block collection position management (not bitstring index)
			 * - Skip-ahead algorithms for sparse pattern optimization
			 * - Template friend integration with BBObject scanning classes
			 * @{
			 */

			template <class U>
			friend struct BBObject::Scan;
			template <class U>
			friend struct BBObject::ScanDest;
			template <class U>
			friend struct BBObject::ScanRev;
			template <class U>
			friend struct BBObject::ScanDestRev;

			//aliases for bitscanning 
			using scan = typename BBObject::Scan<BBScanSp>;
			using scanR = typename BBObject::ScanRev<BBScanSp>;
			using scanD = typename BBObject::ScanDest<BBScanSp>;
			using scanDR = typename BBObject::ScanDestRev<BBScanSp>;

			//inherit constructors
			using BitSetSp::BitSetSp;

			//TODO...check copy and move assignments - should be forbidden->Done
			//OJO si los borramos no pasan algunos tests-> Preguntar a pablo
			/* Explicitly delete copy and move assignment operators
			BBScanSp(const BBScanSp&) = delete;
			BBScanSp& operator=(const BBScanSp&) = delete;
			BBScanSp(BBScanSp&&) = delete;
			BBScanSp& operator=(BBScanSp&&) = delete;*/  

			~BBScanSp() = default;

			///////////////
			// setters and getters
			void scan_block(int blockID) { scan_.bbi_ = blockID; }			//refers to the position in the collection (not in the bitstring)
			void scan_bit(int bit) { scan_.pos_ = bit; }

			int  scan_block()	 const { return scan_.bbi_; }
			int  scan_bit()	 const { return scan_.pos_; }

			//////////////////////////////
			// Bitscanning (with cached info)

			   /**
			   * @brief Configures the initial block and bit position for bitscanning
			   *		 according to one of the 4 scan types passed as argument
			   * @param sct: type of scan
			   * @returns 0 if successful, -1 otherwise (now exits (08/07/2025))
			   * @details : sparse bitsets may have no blocks, in which case the scan is not possible and
			   *		     the function returns -1 -
			   * @details : (08/07/2025) throws  BitScanError for empty sparse bitset 
			   **/
			int init_scan(scan_types sct);

			/**
			* @brief Configures the initial block and bit position for bitscanning
			*		 starting from the bit 'firstBit' onwards, excluding 'firstBit'
			*		 according to one of the 4 scan types passed as argument
			*		 (currently ONLY for the NON-DESTRUCTIVE cases).
			*		 If firstBit is -1 (BBObject::noBit), the scan starts from the beginning.
			* @param firstBit: starting bit
			* @param sct: type of scan
			* @returns 0 if successful, -1 otherwise (now exits (08/07/2025))
			* @details : sparse bitsets may have no blocks, in which case the scan is not possible and
			*		     the function returns -1 
			* @details : (08/07/2025) throws  BitScanError for empty sparse bitset
			*
			* TODO - extend to NON-DESTRUCTIVE cases
			**/
			int init_scan(int firstBit, scan_types sct);

			/** @} */ // end SparseScanManagement group

			/**
			 * @defgroup SparseForwardScanning Forward Bit Scanning Operations for Sparse Bitsets
			 * @brief Functions for forward bit enumeration optimized for sparse patterns
			 * @details This group contains all forward scanning operations (next_bit variants)
			 * that traverse sparse bitsets from low to high bit positions. Operations are
			 * optimized to skip empty blocks and leverage the sparse storage structure.
			 * 
			 * **Sparse Scanning Optimizations:**
			 * - Skip empty blocks automatically during traversal
			 * - Block collection indexing vs. bitstring indexing awareness
			 * - Efficient handling of sparse pattern discontinuities
			 * - Both destructive and non-destructive scanning modes
			 * @{
			 */

			/**
			* @brief next bit in the bitstring, starting from the block
			*		 in the last call to next_bit.
			*		 Scan type: destructive
			*
			*		 I. caches the current block for the next call
			*		II. erases the current scanned bit
			*		III. First call requires initialization with init_scan(DESTRUCTIVE)
			*
			* @returns the next bit in the bitstring, BBObject::noBit if there are no more bits
			* @details created 2015, last update 25/02/2025
			**/
			inline int next_bit_del();
			inline int next_bit_del(BBScanSp& bitset);

			/**
			* @brief next bit in the bitstring, starting from the bit retrieved
			*		 in the last call to next_bit.
			*		 Scan type: non-destructive
			*
			*		 I. caches the current block for the next call
			*		II. DOES NOT erase the current scanned bit
			*		III. caches the scanned bit for the next call
			* 		IV. First call requires initialization with init_scan(NON-DESTRUCTIVE)
			*
			* @returns the next bit in the bitstring, BBObject::noBit if there are no more bits
			* @details Created 5/9/2014, last update 09/02/2025
			* @details Since the scan does not delete the scanned bit from the bitstring,
			*		   it has to cache the last scanned bit for the next call
			**/
			inline int next_bit();
			inline int next_bit(BBScanSp& bitset);

			/**
			* @brief for basic bitscanning - they are hidden by next_bit()
			**/
			using BitSetSp::next_bit;

			/** @} */ // end SparseForwardScanning group

			/**
			 * @defgroup SparseReverseScanning Reverse Bit Scanning Operations for Sparse Bitsets  
			 * @brief Functions for reverse bit enumeration optimized for sparse patterns
			 * @details This group contains all reverse scanning operations (prev_bit variants)
			 * that traverse sparse bitsets from high to low bit positions. Operations are
			 * optimized to skip empty blocks and handle sparse pattern discontinuities efficiently.
			 * 
			 * **Sparse Scanning Optimizations:**
			 * - Reverse skip-ahead through empty blocks
			 * - Efficient backward traversal of block collections
			 * - Sparse pattern discontinuity handling in reverse
			 * - Both destructive and non-destructive scanning modes
			 * @{
			 */

			/**
			* @brief previous bit (next less significant bit) in the bitstring, starting from the bit retrieved
			*		 in the last call to next_bit.
			*		 Scan type: destructive, reverse
			*
			*		 I. caches the current block for the next call
			*		II. erases the current scanned bit
			* 		III. First call requires initialization with init_scan(DESTRUCTIVE, REVERSE)
			*
			* @returns the next lsb bit in the bitstring, BBObject::noBit if there are no more bits
			* @details Created 23/3/12, last update 09/02/2025
			**/
			inline int prev_bit_del();
			inline int prev_bit_del(BBScanSp& bitset);
			/**
			* @brief previous bit (next less significant bit)) in the bitstring, starting from the bit retrieved
			*		 in the last call to next_bit.
			*		 Scan type: non-destructive, reverse
			*
			*		 I. caches the current block for the next call
			*		II. caches the scanned bit for the next call
			* 		III. First call requires initialization with init_scan(NON-DESTRUCTIVE, REVERSE)
			*
			* @returns the previous bit (next less significant bit) in the bitstring, BBObject::noBit if there are no more bits
			* @details Created 5/9/2014, last update 09/02/2025
			* @details Since the scan does not delete the scanned bit from the bitstring,
			*		   it has to cache the last scanned bit for the next call
			**/
			inline int prev_bit();
			inline int prev_bit(BBScanSp& bitset);

			/////////////////
			//	DEPRECATED

			//inline int next_bit_del				(int& nBB);								//nBB: index of bitblock in the bitstring	(not in the collection)	
			//inline int next_bit_del				(int& nBB, BBScanSp& );					//EXPERIMENTAL! 
			//inline int next_bit_del_pos			(int& posBB);							//posBB: position of bitblock in the collection (not the index of the element)		
			//inline int next_bit					(int& nBB);								//nBB: index of bitblock in the bitstring	(not in the collection)				
			//inline int prev_bit_del				(int& nBB);

			/** @} */ // end SparseReverseScanning group

			/**
			 * @brief Internal sparse scan state storage
			 * @details Protected member containing cached scan position for efficient
			 * continuation of sparse bitscanning operations. Manages both block collection
			 * position and bit position within blocks for optimal sparse traversal.
			 */
		protected:
			scan_t scan_;
		};

	}//end namespace _impl

	using _impl::BBScanSp;	

}//end namespace bitgraph

///////////////////////
// Inline implementations, necessary in header file for generic code

namespace bitgraph {

	inline int BBScanSp::next_bit() {

		U32 posInBB;

		//search for next bit in the last block
		if (_BitScanForward64(&posInBB, vBB_[scan_.bbi_].bb_ & Tables::mask_high[scan_.pos_])) {

			//stores the current bit for next call
			scan_.pos_ = posInBB;

			return (posInBB + WMUL(vBB_[scan_.bbi_].idx_));

		}
		else {

			//Searches for next bit in the remaining blocks
			for (auto i = scan_.bbi_ + 1; i < (int)vBB_.size(); ++i) {
				if (_BitScanForward64(&posInBB, vBB_[i].bb_)) {
					//stores the current block and bit for next call
					scan_.bbi_ = i;
					scan_.pos_ = posInBB;

					return (posInBB + WMUL(vBB_[i].idx_));
				}
			}
		}

		return BBObject::noBit;

	}

	inline int BBScanSp::next_bit(BBScanSp& bitset)
	{
		U32 posInBB;

		//search for next bit in the last block
		if (_BitScanForward64(&posInBB, vBB_[scan_.bbi_].bb_ & Tables::mask_high[scan_.pos_])) {

			//stores the current bit for next call
			scan_.pos_ = posInBB;

			//delete the bit from the input bitset
			bitset.erase_bit(posInBB);

			return (posInBB + WMUL(vBB_[scan_.bbi_].idx_));

		}
		else {

			//Searches for next bit in the remaining blocks
			for (auto i = scan_.bbi_ + 1; i < (int)	vBB_.size(); ++i) {
				if (_BitScanForward64(&posInBB, vBB_[i].bb_)) {
					//stores the current block and bit for next call
					scan_.bbi_ = i;
					scan_.pos_ = posInBB;

					//delete the bit from the input bitset
					bitset.erase_bit(posInBB);

					return (posInBB + WMUL(vBB_[i].idx_));
				}
			}
		}

		return BBObject::noBit;
	}

	inline int BBScanSp::prev_bit() {

		U32 posInBB;

		//searches for previous bit in the last scanned block
		if (_BitScanReverse64(&posInBB, vBB_[scan_.bbi_].bb_ & Tables::mask_low[scan_.pos_])) {
			//stores the current bit for next call
			scan_.pos_ = posInBB;

			return (posInBB + WMUL(vBB_[scan_.bbi_].idx_));
		}
		else {
			//Searches for previous bit in the remaining blocks
			for (auto i = scan_.bbi_ - 1; i >= 0; --i) {

				if (_BitScanReverse64(&posInBB, vBB_[i].bb_)) {

					//stores the current block and bit for next call
					scan_.bbi_ = i;
					scan_.pos_ = posInBB;

					return (posInBB + WMUL(vBB_[i].idx_));
				}
			}
		}

		return BBObject::noBit;

	}

	inline int BBScanSp::prev_bit(BBScanSp& bitset)
	{
		U32 posInBB;

		//searches for previous bit in the last scanned block
		if (_BitScanReverse64(&posInBB, vBB_[scan_.bbi_].bb_ & Tables::mask_low[scan_.pos_])) {
			//stores the current bit for next call
			scan_.pos_ = posInBB;

			//delete the bit from the input bitset
			bitset.erase_bit(posInBB);

			return (posInBB + WMUL(vBB_[scan_.bbi_].idx_));
		}
		else {
			//Searches for previous bit in the remaining blocks
			for (auto i = scan_.bbi_ - 1; i >= 0; --i) {

				if (_BitScanReverse64(&posInBB, vBB_[i].bb_)) {

					//stores the current block and bit for next call
					scan_.bbi_ = i;
					scan_.pos_ = posInBB;

					//delete the bit from the input bitset
					bitset.erase_bit(posInBB);

					return (posInBB + WMUL(vBB_[i].idx_));
				}
			}
		}

		return BBObject::noBit;
	}


	inline int BBScanSp::next_bit_del() {

		U32 posInBB;

		for (auto i = scan_.bbi_; i < (int)vBB_.size(); ++i) {

			if (_BitScanForward64(&posInBB, vBB_[i].bb_)) {

				//stores the current block
				scan_.bbi_ = i;

				//deletes the current bit before returning
				vBB_[i].bb_ &= ~Tables::mask[posInBB];

				return (posInBB + WMUL(vBB_[i].idx_));
			}

		}

		return BBObject::noBit;

	}

	inline int BBScanSp::next_bit_del(BBScanSp& bitset)
	{
		U32 posInBB;

		for (auto i = scan_.bbi_; i < (int)vBB_.size(); ++i) {

			if (_BitScanForward64(&posInBB, vBB_[i].bb_)) {

				//stores the current block
				scan_.bbi_ = i;

				//deletes the current bit before returning
				vBB_[i].bb_ &= ~Tables::mask[posInBB];

				//delete the bit from the input bitset
				bitset.erase_bit(posInBB);

				return (posInBB + WMUL(vBB_[i].idx_));
			}

		}

		return BBObject::noBit;
	}

	inline int BBScanSp::prev_bit_del() {

		U32 posInBB;

		for (int i = scan_.bbi_; i >= 0; --i) {

			if (_BitScanReverse64(&posInBB, vBB_[i].bb_)) {

				//stores the current block for the next call
				scan_.bbi_ = i;

				//deletes the current bit from the bitset before returning
				vBB_[i].bb_ &= ~Tables::mask[posInBB];

				return (posInBB + WMUL(vBB_[i].idx_));
			}
		}

		return BBObject::noBit;
	}

	inline int BBScanSp::prev_bit_del(BBScanSp& bitset)
	{
		U32 posInBB;

		for (int i = scan_.bbi_; i >= 0; --i) {

			if (_BitScanReverse64(&posInBB, vBB_[i].bb_)) {

				//stores the current block for the next call
				scan_.bbi_ = i;

				//deletes the current bit from the bitset before returning
				vBB_[i].bb_ &= ~Tables::mask[posInBB];

				//delete the bit from the input bitset
				bitset.erase_bit(posInBB);

				return (posInBB + WMUL(vBB_[i].idx_));
			}
		}

		return BBObject::noBit;
	}

	inline
		int BBScanSp::init_scan(scan_types sct)  {

		//necessary check since sparse bitstrings have empty semantics (i.e. sparse graphs)
		if (vBB_.empty()) {
			//LOG_ERROR("empty sparse bitstring, cannot be scanned - BBScanSp::init_scan...exiting");
			throw BitScanError("empty sparse bitstring, cannot be scanned - BBScanSp::init_scan");		
		}			

		switch (sct) {
		case NON_DESTRUCTIVE:
			scan_block(0);
			scan_bit(MASK_LIM);
			break;
		case NON_DESTRUCTIVE_REVERSE:
			scan_block(vBB_.size() - 1);
			scan_bit(WORD_SIZE);
			break;
		case DESTRUCTIVE:
			scan_block(0);
			break;
		case DESTRUCTIVE_REVERSE:
			scan_block(vBB_.size() - 1);
			break;
		default:
			LOG_ERROR("unknown scan type in BBScanSp::init_scan");
			assert(false);
			//throw BitScanError("unknown scan type in BBScanSp::init_scan");		//will not be handled - terminates the program
	
		}

		return 0;
	}

	inline
		int BBScanSp::init_scan(int firstBit, scan_types sct)  {

		//necessary check 
		if (vBB_.empty()) {
			//LOG_ERROR("empty sparse bitstring, cannot be scanned - BBScanSp::init_scan...exiting");
			throw BitScanError("empty sparse bitstring, cannot be scanned - BBScanSp::init_scan");		
		}

		//special case - first bitscan
		if (firstBit == BBObject::noBit) {
			return init_scan(sct);
		}

		//determine the index of the starting block (not its ID)
		auto bbL = WDIV(firstBit);

		////////////////////////////////////////////////////
		pair<bool, int> p = find_block_pos(bbL);
		///////////////////////////////////////////////////

		//no blocks with index greater or equal to bbL, nothing to scan
		if (p.second == BBObject::noBit) {
			return -1;
		}

		switch (sct) {
		case NON_DESTRUCTIVE:
		case NON_DESTRUCTIVE_REVERSE:
			scan_block(p.second);
			(p.first) ? scan_bit(firstBit - WMUL(bbL)) : scan_bit(MASK_LIM);
			break;
		case DESTRUCTIVE:
		case DESTRUCTIVE_REVERSE:
			//scan_block(p.second);
			LOG_ERROR("destructive scan type currently not available in BBScanSp::init_scan...exiting");
			std::exit(EXIT_FAILURE);
			//throw BitScanError("incorrect destructive scan type in BBScanSp::init_scan");		
			break;
		default:
			LOG_ERROR("unknown scan type in BBScanSp::init_scan...exiting");
			std::exit(EXIT_FAILURE);
			//throw BitScanError("unknown scan type in BBScanSp::init_scan");		
		}

		//nothing to scan or error
		return 0;
	}


	////////////////////////////////
	//
	// DEPRECATED
	//
	//////////////////////////////////
	//inline 
	//int BBScanSp::next_bit_del(int& block_index) {
	//	////////////////////////////
	//	//
	//	// date: 23/3/12
	//	// Destructive bitscan for sparse bitstrings using intrinsics
	//	//
	//	// COMMENTS
	//	// 1-Requires previous assignment scan_.bbi=0 
	//
	//	unsigned long posbb;
	//
	//	for (int i = scan_.bbi_; i < vBB_.size(); i++) {
	//		if (_BitScanForward64(&posbb, vBB_[i].bb_)) {
	//			scan_.bbi_ = i;
	//			block_index = vBB_[i].idx_;
	//			vBB_[i].bb_ &= ~Tables::mask[posbb];			//deleting before the return
	//			return (posbb + WMUL(vBB_[i].idx_));
	//		}
	//	}
	//
	//	return EMPTY_ELEM;
	//}
	//
	//
	//inline int BBScanSp::next_bit_del(int& block_index, BBScanSp& bbN_del) {
	//	////////////////////////////
	//	//
	//	// date: 06/07/2019 (EXPERIMENTAL)
	//	// Destructive bitscan for sparse bitstrings using intrinsics
	//	//
	//	// COMMENTS
	//	// 1-added for compatibility with BBIntrin
	//	// 1-Requires previous assignment scan_.bbi=0 
	//
	//	unsigned long posbb;
	//
	//	for (int i = scan_.bbi_; i < vBB_.size(); i++) {
	//		if (_BitScanForward64(&posbb, vBB_[i].bb_)) {
	//			scan_.bbi_ = i;
	//			block_index = vBB_[i].idx_;
	//			vBB_[i].bb_ &= ~Tables::mask[posbb];			//deleting before the return
	//			bbN_del.vBB_[i].bb_ &= ~Tables::mask[posbb];
	//			return (posbb + WMUL(vBB_[i].idx_));
	//		}
	//	}
	//
	//	return EMPTY_ELEM;
	//}
	//
	//inline int BBScanSp::next_bit_del_pos(int& posBB) {
	//	////////////////////////////
	//	//
	//	// date: 29/10/14
	//	// Destructive bitscan which returns the position of the bitblock scanned in the collection
	//	// (not the index attribute)
	//	//
	//	// COMMENTS
	//	// 1-Requires previous assignment scan_.bbi=0 
	//
	//	unsigned long posbb;
	//
	//	for (int i = scan_.bbi_; i < vBB_.size(); i++) {
	//		if (_BitScanForward64(&posbb, vBB_[i].bb_)) {
	//			posBB = scan_.bbi_ = i;
	//			vBB_[i].bb_ &= ~Tables::mask[posbb];			//deleting before the return
	//			return (posbb + WMUL(vBB_[i].idx_));
	//		}
	//	}
	//
	//	return EMPTY_ELEM;
	//}

	//inline int BBScanSp::next_bit(int& block_index) {
	//	////////////////////////////
	//	// date:5/9/2014
	//	// non destructive bitscan for sparse bitstrings using intrinsics
	//	// caches index in the collection and pos inside the bitblock
	//	//
	//	// comments
	//	// 1-require previous assignment scan_.bbi=0 and scan_.pos_=mask_lim
	//
	//	unsigned long posbb;
	//
	//	//search for next bit in the last block
	//	if (_BitScanForward64(&posbb, vBB_[scan_.bbi_].bb_ & Tables::mask_high[scan_.pos_])) {
	//		scan_.pos_ = posbb;
	//		block_index = vBB_[scan_.bbi_].idx_;
	//		return (posbb + WMUL(vBB_[scan_.bbi_].idx_));
	//	}
	//	else {											//search in the remaining blocks
	//		for (int i = scan_.bbi_ + 1; i < vBB_.size(); i++) {
	//			if (_BitScanForward64(&posbb, vBB_[i].bb_)) {
	//				scan_.bbi_ = i;
	//				scan_.pos_ = posbb;
	//				block_index = vBB_[i].idx_;
	//				return (posbb + WMUL(vBB_[i].idx_));
	//			}
	//		}
	//	}
	//
	//	return EMPTY_ELEM;
	//}

	//inline int BBScanSp::prev_bit_del(int& bb_index) {
	//	////////////////////////////
	//	//
	//	// date: 23/3/12
	//	// Destructive bitscan for sparse bitstrings using intrinsics
	//	//
	//	// COMMENTS
	//	// 1-Requires previous assignment scan_.bbi=number of bitblocks-1
	//
	//	unsigned long posbb;
	//
	//	for (int i = scan_.bbi_; i >= 0; i--) {
	//		if (_BitScanReverse64(&posbb, vBB_[i].bb_)) {
	//			scan_.bbi_ = i;
	//			bb_index = vBB_[i].idx_;
	//			vBB_[i].bb_ &= ~Tables::mask[posbb];			//deleting before the return
	//			return (posbb + WMUL(vBB_[i].idx_));
	//		}
	//	}
	//
	//	return BBObject::noBit;
	//}

}//end namespace bitgraph


#endif 