/**
 * @file bbsentinel.h
 * @brief Watched bitset with sentinel-bounded operations for optimization
 * @author Pablo San Segundo
 * @version 1.0
 * @date 2025
 * @since 13/02/2025
 * 
 * @details Implements BBSentinel class that optimizes bitset operations by
 * maintaining sentinel bounds [low_sentinel, high_sentinel] that mark the
 * range of non-zero bitblocks. This significantly reduces computation time
 * for sparse bitsets by skipping empty regions.
 * 
 * **Sentinel System:**
 * - Low sentinel: First non-zero bitblock in the bitstring
 * - High sentinel: Last non-zero bitblock in the bitstring
 * - Operations constrained to sentinel range for efficiency
 * 
 * **Research Application:**
 * Enhanced clique algorithms with published performance improvements
 * 
 * @todo Complete full refactoring implementation (13/02/2025)
 **/

#ifndef __BB_SENTINEL_H__
#define __BB_SENTINEL_H__

#include "bitscan/bbscan.h"
#include "bitscan/bbalgorithm.h"			//MIN, MAX
#include <utility>							//std::move
#include <numeric>							//std::accumulate

namespace bitgraph {
	namespace _impl {


		/**
		 * @class BBSentinel
		 * @brief Watched bitset with sentinel-bounded operations
		 * @details Extends BBScan with sentinel tracking that maintains bounds
		 * on the active (non-zero) range of bitblocks. This optimization technique
		 * dramatically improves performance for sparse bitsets by limiting operations
		 * to the [low_sentinel, high_sentinel] range.
		 * 
		 * **Optimization Principle:**
		 * - Track first and last non-zero blocks (sentinels)
		 * - Constrain all operations to sentinel range
		 * - Skip empty regions automatically
		 * - Update sentinels dynamically as bitset changes
		 * 
		 * **Performance Gains:**
		 * - O(k) operations where k = sentinel range, not full bitset size
		 * - Particularly effective for algorithms with locality patterns
		 * - Significant speedup in graph algorithms and combinatorial optimization
		 * 
		 * **Inheritance:** Extends BBScan with sentinel tracking capabilities
		 */
		class BBSentinel : public BBScan {
			/**
			 * @brief Friend function for optimized AND operation with sentinel update
			 * @param lhs Left operand (any BitSet type)
			 * @param rhs Right operand (BBSentinel)
			 * @param[out] res Result BBSentinel with updated sentinels
			 * @return Reference to res
			 * @post res contains lhs AND rhs, sentinels updated to actual range
			 * @par Complexity: O(min(lhs.size, rhs.sentinel_range))
			 */
			friend BBSentinel& AND(const BitSet& lhs, const BBSentinel& rhs, BBSentinel& res);

		public:
			/**
			 * @brief Default constructor
			 * @post Sentinels initialized to EMPTY_ELEM
			 */
			BBSentinel() : m_BBH(EMPTY_ELEM), m_BBL(EMPTY_ELEM) { 
				init_sentinels(false); 
			}
			
			/**
			 * @brief Constructs sentinel bitset with specified capacity
			 * @param popsize Maximum number of bits
			 * @post Sentinels initialized to full range [0, nBB_-1]
			 */
			explicit BBSentinel(int popsize) : BBScan(popsize) { 
				init_sentinels(false); 
			}
			
			/**
			 * @brief Copy constructor
			 * @param bbN Source sentinel bitset
			 * @post Copies both bitset data and sentinel bounds
			 */
			BBSentinel(const BBSentinel& bbN) : BBScan(bbN) { 
				m_BBH = bbN.m_BBH; 
				m_BBL = bbN.m_BBL; 
			}
			
			/**
			 * @brief Destructor
			 */
			~BBSentinel() = default;
			//NUEVA IMPLEMENTACION
			/**
			 * @brief Move constructor
			 * @param other Source sentinel bitset to move from
			 * @post other's sentinels reset to EMPTY_ELEM
			 * @par Complexity: O(1)
			 */
			BBSentinel(BBSentinel&& other) noexcept
				: BBScan(std::move(other)),
				  m_BBH(other.m_BBH),
				  m_BBL(other.m_BBL) {
				// Reset moved-from object's sentinels
				other.m_BBH = EMPTY_ELEM;
				other.m_BBL = EMPTY_ELEM;
			}

			////////////
			// Sentinel Management
			// añadidos noexcept y const a varios
			/**
			 * @brief Sets high sentinel (last non-zero block)
			 * @param i Block index for high sentinel
			 * @warning Does not validate i against actual data
			 */
			void set_sentinel_H(unsigned int i) noexcept { m_BBH = i; }
			
			/**
			 * @brief Sets low sentinel (first non-zero block)
			 * @param i Block index for low sentinel
			 * @warning Does not validate i against actual data
			 */
			void set_sentinel_L(unsigned int i) noexcept { m_BBL = i; }
			
			/**
			 * @brief Sets both sentinels simultaneously
			 * @param low Block index for low sentinel
			 * @param high Block index for high sentinel
			 * @pre low <= high
			 */
			void set_sentinels(int low, int high);
			
			/**
			 * @brief Initializes sentinels to span entire bitset or actual data range
			 * @param update If true, scans bitset to find actual non-zero range
			 * @post Sentinels set to [0, nBB_-1] if !update, or actual range if update
			 * @par Complexity: O(1) if !update, O(n/64) if update
			 */
			void init_sentinels(bool update = false);
			
			/**
			 * @brief Resets sentinels to EMPTY_ELEM
			 * @post m_BBL == m_BBH == EMPTY_ELEM
			 */
			void clear_sentinels() noexcept;
			
			/**
			 * @brief Gets low sentinel value
			 * @return Index of first non-zero block or EMPTY_ELEM
			 */
			int get_sentinel_L() const noexcept { return m_BBL; }
			
			/**
			 * @brief Gets high sentinel value
			 * @return Index of last non-zero block or EMPTY_ELEM
			 */
			int get_sentinel_H() const noexcept { return m_BBH; }
			/////////////
			// Sentinel Update Operations
			
			/**
			 * @brief Updates both sentinels by scanning entire bitset
			 * @return 0 if successful, EMPTY_ELEM if bitset is empty
			 * @post Sentinels set to actual range of non-zero blocks
			 * @par Complexity: O(n/64)
			 */
			int update_sentinels();
			
			/**
			 * @brief Updates sentinels within specified range
			 * @param bbl Starting block for search
			 * @param bbh Ending block for search
			 * @return 0 if successful, EMPTY_ELEM if range is empty
			 * @pre bbl <= bbh
			 * @par Complexity: O(bbh - bbl)
			 */
			int update_sentinels(int bbl, int bbh);
			
			/**
			 * @brief Updates only high sentinel by backward scan
			 * @return Updated high sentinel value or EMPTY_ELEM
			 * @par Complexity: O(n/64) worst case
			 */
			int update_sentinels_high();
			
			/**
			 * @brief Updates only low sentinel by forward scan
			 * @return Updated low sentinel value or EMPTY_ELEM
			 * @par Complexity: O(n/64) worst case
			 */
			int update_sentinels_low();
			
			/**
			 * @brief Sets sentinels to include specific vertex
			 * @param v Vertex/bit position to include
			 * @post Sentinels expanded if necessary to include block containing v
			 */
			void update_sentinels_to_v(int v);

			//////////////
			// Bit Manipulation Operations
			
			/**
			 * @brief Clears all bits within sentinel range
			 * @post All bits in [m_BBL, m_BBH] blocks cleared
			 * @warning Does not update sentinels
			 * @par Complexity: O(m_BBH - m_BBL + 1)
			 */
			virtual void erase_bit();
			
			/**
			 * @brief Clears specific bit
			 * @param nBit Bit position to clear
			 * @warning Does not update sentinels
			 * @par Complexity: O(1)
			 */
			virtual void erase_bit(int nBit) { BitSet::erase_bit(nBit); }
			
			/**
			 * @brief Clears bit and updates sentinels if necessary
			 * @param nBit Bit position to clear
			 * @post Bit cleared and sentinels updated if block becomes empty
			 * @par Complexity: O(1) average, O(n/64) if sentinel update needed
			 */
			void erase_bit_and_update(int nBit);
			
			/**
			 * @brief Clears all bits present in another bitset
			 * @param bs Bitset containing bits to clear
			 * @return Reference to this bitset
			 * @par Application: Used in SEQ coloring algorithm
			 */
			BBSentinel& erase_bit(const BitSet& bs);

			/**
			 * @brief Checks if bitset is empty within sentinel range
			 * @return true if no bits set in [m_BBL, m_BBH]
			 * @par Complexity: O(m_BBH - m_BBL + 1)
			 */
			virtual bool is_empty() const;
			
			/**
			 * @brief Checks if specific range is empty
			 * @param nBBL Starting block index
			 * @param nBBH Ending block index
			 * @return true if no bits set in [nBBL, nBBH]
			 * @pre nBBL <= nBBH
			 * @par Complexity: O(nBBH - nBBL + 1)
			 */
			virtual bool is_empty(int nBBL, int nBBH) const;

#ifdef POPCOUNT_INTRINSIC_64
			/**
			 * @brief Counts set bits within sentinel range using hardware intrinsics
			 * @return Population count in [m_BBL, m_BBH]
			 * @par Complexity: O(m_BBH - m_BBL + 1) with hardware support
			 * @note Only available when POPCOUNT_INTRINSIC_64 is defined
			 */
			int popcn64() const;
#endif

			////////////////
			// Operators
			
			/**
			 * @brief Copy assignment operator
			 * @param other Source sentinel bitset
			 * @return Reference to this bitset
			 * @post Copies both data and sentinels
			 */
			BBSentinel& operator=(const BBSentinel& other);
			
			//NUEVA IMPLEMENTACION
			/**
			 * @brief Move assignment operator
			 * @param other Source sentinel bitset to move from
			 * @return Reference to this bitset
			 * @post other's sentinels reset to EMPTY_ELEM
			 * @par Complexity: O(1)
			 */
			BBSentinel& operator=(BBSentinel&& other) noexcept {
				if (this != &other) {
					BBScan::operator=(std::move(other));
					m_BBH = other.m_BBH;
					m_BBL = other.m_BBL;
					// Reset moved-from object's sentinels
					other.m_BBH = EMPTY_ELEM;
					other.m_BBL = EMPTY_ELEM;
				}
				return *this;
			}
			
			/**
			 * @brief Bitwise AND assignment with another bitset
			 * @param bs Bitset to AND with
			 * @return Reference to this bitset
			 * @post Updates sentinels after operation
			 * @par Complexity: O(n/64)
			 */
			BBSentinel& operator&=(const BitSet& bs);

			//////////////
			// I/O

			std::ostream& print(std::ostream& o = std::cout, bool show_pc = true, bool endl = true) const override;

			/////////////////
			// Bit Scanning Operations
			
			/**
			 * @brief Initializes scanning within sentinel range
			 * @param sct Scan type (DESTRUCTIVE or NON_DESTRUCTIVE)
			 * @return First bit position or EMPTY_ELEM if empty
			 * @post Scanner positioned at m_BBL for forward scan
			 * @par Complexity: O(1) to O(m_BBH - m_BBL) depending on bit distribution
			 * @par Performance Note
			 * Sentinel bounds provide major speedup for sparse bitsets by skipping
			 * empty regions entirely. Particularly effective in graph algorithms
			 * where vertex sets become progressively smaller.
			 */
			int init_scan(scan_types sct) noexcept override;

			/**
			 * @brief Destructively scans backward within sentinels
			 * @return Previous bit position or EMPTY_ELEM if none
			 * @post Bit removed and high sentinel updated
			 * @par Complexity: O(1) with hardware support
			 * @warning Updates m_BBH during scan
			 */
			inline int prev_bit_del() override;
			
			/**
			 * @brief Destructively scans forward within sentinels
			 * @return Next bit position or EMPTY_ELEM if none
			 * @post Bit removed and low sentinel updated
			 * @par Complexity: O(1) with hardware support
			 * @warning Updates m_BBL during scan
			 */
			inline int next_bit_del() override;
			
			/**
			 * @brief Destructively scans forward, clearing in two bitsets
			 * @param bbN_del Second bitset to clear bit from
			 * @return Next bit position or EMPTY_ELEM if none
			 * @post Bit removed from both this and bbN_del
			 * @note Does not update bbN_del's sentinels
			 */
			inline int next_bit_del(BBSentinel& bbN_del);
			
			/**
			 * @brief Non-destructively scans forward within sentinels
			 * @return Next bit position or EMPTY_ELEM if none
			 * @par Complexity: O(1) with hardware support
			 * @note Constrained to [m_BBL, m_BBH] range
			 */
			inline int next_bit() override;

			//inline	int next_bit(int& nBB) override;

			//////////////////////////////////
			// Data Members
		protected:
			/**
			 * @brief High sentinel - index of last non-zero block
			 * @details Tracks the highest block index containing set bits.
			 * All blocks above m_BBH are guaranteed to be zero.
			 */
			int m_BBH;
			
			/**
			 * @brief Low sentinel - index of first non-zero block
			 * @details Tracks the lowest block index containing set bits.
			 * All blocks below m_BBL are guaranteed to be zero.
			 */
			int m_BBL;
		};

	}//namespace _impl

	using _impl::BBSentinel;

	using _impl::AND;

}//namespace bitgraph



#ifdef POPCOUNT_INTRINSIC_64

namespace bitgraph {

	inline int BBSentinel::popcn64() const {
		//BITBOARD pc = 0;
		//CODIGO ORIGINAL
		//for (int i = m_BBL; i <= m_BBH; i++) {
		//	pc += __popcnt64(vBB_[i]);
		//}
	
		return std::accumulate(vBB_.data() + m_BBL, vBB_.data() + m_BBH + 1, BITBOARD(0), 
			[](BITBOARD sum, BITBOARD block) { return sum + __popcnt64(block); });
	}

}

#endif

namespace bitgraph {

	//specializes the only bitscan function used

	int BBSentinel::prev_bit_del() {
		//////////////
		// BitScan reverse order and distructive
		//
		// COMMENTS
		// 1- update sentinels at the start of loop

		unsigned long posInBB;

		for (int i = m_BBH; i >= m_BBL; i--) {
			if (_BitScanReverse64(&posInBB, vBB_[i])) {
				m_BBH = i;
				vBB_[i] &= ~Tables::mask[posInBB];			//erase before the return
				return (posInBB + WMUL(i));
			}
		}
		return EMPTY_ELEM;
	}


	int BBSentinel::next_bit_del() {
		//////////////
		// Bitscan distructive between sentinels
		//
		// COMMENTS
		// 1- update sentinels at the start of loop

		unsigned long posInBB;

		for (int i = m_BBL; i <= m_BBH; i++) {
			if (_BitScanForward64(&posInBB, vBB_[i])) {
				m_BBL = i;
				vBB_[i] &= ~Tables::mask[posInBB];					//erase before the return
				return (posInBB + WMUL(i));
			}
		}
		return EMPTY_ELEM;
	}


	int BBSentinel::next_bit_del(BBSentinel& bbN_del) {
		//////////////
		// Bitscan distructive between sentinels
		//
		// COMMENTS
		// 1- update sentinels at the start of loop (experimental, does not use sentinesl of bbN_del)

		unsigned long posInBB;

		for (int i = m_BBL; i <= m_BBH; i++) {
			if (_BitScanForward64(&posInBB, vBB_[i])) {
				m_BBL = i;
				vBB_[i] &= ~Tables::mask[posInBB];					//erase before the return
				bbN_del.vBB_[i] &= ~Tables::mask[posInBB];
				return (posInBB + WMUL(i));
			}
		}

		return EMPTY_ELEM;
	}



	int BBSentinel::next_bit() {
		////////////////////////////
		// last update:31/12/2013
		// BitScan non destructive
		//
		// COMMENTS
		// 1- update sentinels, set scan_.bbi_ to m_BBL and set scan_.pos_ to MASK_LIM at the start of loop

		unsigned long posInBB;

		if (_BitScanForward64(&posInBB, vBB_[scan_.bbi_] & Tables::mask_high[scan_.pos_])) {
			scan_.pos_ = posInBB;
			return (posInBB + WMUL(scan_.bbi_));
		}
		else {
			for (int i = scan_.bbi_ + 1; i <= m_BBH; i++) {
				if (_BitScanForward64(&posInBB, vBB_[i])) {
					scan_.bbi_ = i;
					scan_.pos_ = posInBB;
					return (posInBB + WMUL(i));
				}
			}
		}
		return EMPTY_ELEM;
	}

	//inline
	//int BBSentinel::next_bit(int& nBB){
	//////////////////////////////
	//// last update:31/12/2013
	//// BitScan non destructive
	////
	//// COMMENTS
	//// 1- update sentinels, set scan_.bbi_ to m_BBL and set scan_.pos_ to MASK_LIM at the start of loop
	//
	//	unsigned long posInBB;
	//			
	//	//look uo in the last table
	//	if(_BitScanForward64(&posInBB, vBB_[scan_.bbi_] & Tables::mask_high[scan_.pos_])){
	//		scan_.pos_ =posInBB;
	//		nBB=scan_.bbi_;
	//		return (posInBB + WMUL(scan_.bbi_));
	//	}else{											//not found in the last table. look up in the rest
	//		for(int i=(scan_.bbi_+1); i<=m_BBH; i++){
	//			if(_BitScanForward64(&posInBB,vBB_[i])){
	//				scan_.bbi_=i;
	//				scan_.pos_=posInBB;
	//				nBB=i;
	//				return (posInBB+ WMUL(i));
	//			}
	//		}
	//	}
	//return EMPTY_ELEM;
	//}

}//end namespace bitgraph

#endif 





