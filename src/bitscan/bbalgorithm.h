/**
 * @file bbalgorithm.h
 * @brief Algorithm templates and utility functions for bitset operations
 * @author Pablo San Segundo
 * @version 1.1
 * @date 2025
 * @since 27/02/2025
 *
 * @details Provides template algorithms and utility data structures for
 * bitstring and bitblock operations. Contains the bbalg namespace with
 * stateless functions for BitSet manipulation and specialized wrapper
 * classes for performance optimization.
 * 
 * **Key Components:**
 * - **bbalg namespace**: Stateless algorithms for bitset conversion and analysis
 * - **bbSize_t**: Wrapper class with cached population count for O(1) size queries
 * - **bbStack_t**: Hybrid stack/bitset container with synchronized operations
 * - **bbCol_t**: Fixed-size collection of bitsets for parallel operations
 * 
 * **Design Principles:**
 * - Header-only implementation for template instantiation
 * - Template-based algorithms for type flexibility
 * - Stateless function design for thread safety
 * - Cached metadata for performance optimization
 * 
 * @par Thread Safety
 * All functions in bbalg namespace are thread-safe as they are stateless.
 * Wrapper classes (bbSize_t, bbStack_t, bbCol_t) are NOT thread-safe.
 * 
 * @par Performance Notes
 * - bbSize_t provides O(1) size() queries vs O(n/64) for standard bitsets
 * - bbStack_t enables dual-view access patterns for graph algorithms
 * - bbCol_t optimizes bulk operations on multiple bitsets
 **/

#ifndef  _BBALG_H_
#define  _BBALG_H_

#include "utils/common.h"
#include "utils/logger.h"
#include "bitscan/bbset.h"
#include <array>
#include <functional>
#include <algorithm>  // For std::all_of

//aliases
namespace bitgraph {

	using bbo = BBObject;

}

namespace bitgraph {
	/**
	 * @namespace bitgraph::bbalg
	 * @brief Stateless algorithms for bitset operations
	 * @details Contains template functions for common bitset algorithms
	 * including conversion, random generation, and bit extraction operations.
	 * All functions are stateless and thread-safe, designed for use with
	 * any bitset type in the BBObject hierarchy.
	 * 
	 * @par Algorithm Categories
	 * - **Conversion**: to_vector() - Extract bit positions as integer vector
	 * - **Generation**: gen_random_block() - Create random bit patterns
	 * - **Extraction**: first_k_bits() - Find first k set bits
	 * 
	 * @par Usage Example
	 * @code{.cpp}
	 * BBScan bs(1000);
	 * bs.set_bit(10); bs.set_bit(20); bs.set_bit(30);
	 * 
	 * // Convert to vector of bit positions
	 * std::vector<int> bits = bbalg::to_vector(bs);
	 * // bits = {10, 20, 30}
	 * 
	 * // Generate random bitblock with 50% density
	 * BITBOARD random = bbalg::gen_random_block(0.5);
	 * @endcode
	 */
	namespace bbalg {

		/**
		 * @brief Converts a bitset to a vector of bit positions
		 * @tparam BitSet_t Any bitset type from BBObject hierarchy (BBScan, BBScanSp, etc.)
		 * @param bbn Input bitset (not modified)
		 * @return Vector containing positions of all set bits in ascending order
		 * 
		 * @par Complexity
		 * O(k) where k is the number of set bits
		 * 
		 * @par Requirements
		 * BitSet_t must support next_bit() operation for efficient scanning
		 * 
		 * @par Example
		 * @code{.cpp}
		 * BBScan bs(100);
		 * bs.set_bit(5); bs.set_bit(42); bs.set_bit(99);
		 * auto positions = bbalg::to_vector(bs);
		 * // positions = {5, 42, 99}
		 * @endcode
		 * 
		 * @note Future enhancement: Use SFINAE to enforce BitSet_t requirements
		 */
		template<class BitSet_t>
		std::vector<int> to_vector(BitSet_t& bbn);

		/**
		 * @brief Generates a random 64-bit block with specified bit density
		 * @param p Probability of each bit being set (0.0 to 1.0)
		 * @return BITBOARD with approximately p*64 bits set randomly
		 * 
		 * @pre 0.0 <= p <= 1.0
		 * @post Population count ≈ p * 64 (expected value)
		 * 
		 * @par Complexity
		 * O(64) - constant time for single block
		 * 
		 * @par Statistical Properties
		 * Each bit is independently set with probability p
		 * Expected popcount = 64 * p, Variance = 64 * p * (1-p)
		 * 
		 * @par Example
		 * @code{.cpp}
		 * // Generate block with ~32 bits set (50% density)
		 * BITBOARD half_dense = bbalg::gen_random_block(0.5);
		 * 
		 * // Generate sparse block with ~6 bits set (10% density)
		 * BITBOARD sparse = bbalg::gen_random_block(0.1);
		 * @endcode
		 */
		BITBOARD gen_random_block(double p) noexcept;

		/**
		 * @brief Extracts the first k set bits from a bitset
		 * @tparam BitSet_t Any bitset type supporting init_scan() and next_bit()
		 * @param k Maximum number of bits to extract
		 * @param bb Input bitset (not modified)
		 * @param[out] lv Output vector to store bit positions (cleared first)
		 * @return Actual number of bits found (min(k, popcount(bb)))
		 * 
		 * @pre k >= 0
		 * @post lv.size() == return value <= k
		 * @post lv contains first return_value bit positions in ascending order
		 * 
		 * @par Complexity
		 * O(min(k, popcount(bb))) - stops after k bits or when bitset exhausted
		 * 
		 * @par Application
		 * Used in Maximum Weight Clique Problem (MWCP) for computing upper bounds
		 * by selecting the k heaviest vertices from a candidate set.
		 * 
		 * @par Example
		 * @code{.cpp}
		 * BBScan candidates(1000);
		 * // ... populate candidates ...
		 * 
		 * std::vector<int> top_vertices;
		 * int found = bbalg::first_k_bits(10, candidates, top_vertices);
		 * // top_vertices contains up to 10 vertex indices
		 * @endcode
		 */
		template<class BitSet_t>
		int first_k_bits(int k, BitSet_t& bb, std::vector<int>& lv);

	}//end namespace bbalg

}//end namespace bitgraph

namespace bitgraph {

	namespace _impl {

		/**
		 * @class bbSize_t
		 * @brief Bitset wrapper with cached population count for O(1) size queries
		 * @tparam BitSet_t Any bitset type from BBObject hierarchy
		 * 
		 * @details Maintains an explicit population count (pc_) synchronized with
		 * the underlying bitset, eliminating the O(n/64) cost of computing size.
		 * This optimization is critical for algorithms that frequently query bitset
		 * cardinality, such as branch-and-bound procedures.
		 * 
		 * **Performance Characteristics:**
		 * - size(): O(1) instead of O(n/64)
		 * - set_bit()/erase_bit(): O(1) with counter update
		 * - Memory overhead: 8 bytes for counter
		 * 
		 * **Synchronization:**
		 * - Manual sync via sync_pc() if bitset modified directly
		 * - is_sync_pc() verifies counter accuracy
		 * 
		 * @warning Direct manipulation of bb_ member requires sync_pc() call
		 * 
		 * @par Example
		 * @code{.cpp}
		 * bbSize_t<BBScan> cached_bs(1000);
		 * cached_bs.set_bit(42);     // O(1), updates counter
		 * cached_bs.set_bit(100);
		 * 
		 * if (cached_bs.size() > 1) {  // O(1) check
		 *     int first = cached_bs.lsb();
		 * }
		 * @endcode
		 */
		template <class BitSet_t>
		struct bbSize_t {

			/**
			 * @brief Constructs wrapper with specified capacity
			 * @param MAX_SIZE Maximum number of bits
			 */
			explicit bbSize_t(int MAX_SIZE) : pc_(0), bb_(MAX_SIZE) {}
			
			/**
			 * @brief Default constructor
			 */
			bbSize_t() : pc_(0) {}

			// Copy and move semantics preserved from underlying BitSet_t
			bbSize_t(const bbSize_t&) = default;
			bbSize_t& operator=(const bbSize_t&) = default;
			bbSize_t(bbSize_t&&) noexcept = default;
			bbSize_t& operator=(bbSize_t&&) noexcept = default;

		//allocation

			/**
			 * @brief Resets bitset with new capacity and clears counter
			 * @param MAX_SIZE New maximum number of bits
			 * @post pc_ == 0 and bb_ reallocated
			 */
			void reset(int MAX_SIZE) { bb_.reset(MAX_SIZE); pc_ = 0; }

			/**
			 * @brief Equivalent to reset() for backward compatibility
			 * @deprecated Use reset() instead
			 */
			void init(int MAX_SIZE) { reset(MAX_SIZE); }

			/**
			 * @brief Returns cached population count
			 * @return Number of set bits
			 * @par Complexity: O(1)
			 */
			BITBOARD size() const noexcept { return pc_; }

			/**
			 * @brief Sets bit and increments counter
			 * @param bit Position to set
			 * @pre bit < capacity and !is_bit(bit)
			 * @post is_bit(bit) == true and pc_ incremented
			 */
			void set_bit(int bit) { bb_.set_bit(bit); ++pc_; }
			
			/**
			 * @brief Erases bit and decrements counter
			 * @param bit Position to clear
			 * @return Updated population count
			 * @pre is_bit(bit) == true
			 * @post is_bit(bit) == false and pc_ decremented
			 */
			int erase_bit(int bit) { bb_.erase_bit(bit); return(--pc_); }

			/**
			 * @brief Clears all bits and resets counter
			 * @param lazy If true, only resets counter without clearing bitset
			 * @post pc_ == 0, bb_ cleared unless lazy == true
			 * @warning lazy=true causes desynchronization, use with caution
			 */
			void erase_bit(bool lazy = false) { if (!lazy) { bb_.erase_bit(); } pc_ = 0; }


			/**
			 * @brief Finds least significant bit
			 * @return Position of first set bit or BBObject::noBit if empty
			 * @par Complexity: O(1) with hardware support, O(n/64) otherwise
			 */
			int lsb() const noexcept { 
				return (pc_ > 0) ? bb_.lsb() : BBObject::noBit; 
			}
			
			/**
			 * @brief Finds most significant bit
			 * @return Position of last set bit or BBObject::noBit if empty
			 * @par Complexity: O(1) with hardware support, O(n/64) otherwise
			 */
			int msb() const noexcept { 
				return (pc_ > 0) ? bb_.msb() : BBObject::noBit; 
			}

			/**
			 * @brief Removes and returns most significant bit
			 * @return Position of removed bit or BBObject::noBit if empty
			 * @post pc_ decremented if bit was found
			 * @par Complexity: O(1) with hardware support
			 */
			int pop_msb() {
				if (pc_ > 0) { 
					int bit = bb_.msb(); 
					bb_.erase_bit(bit); 
					--pc_; 
					return bit; 
				}
				return BBObject::noBit;
			}
			
			/**
			 * @brief Removes and returns least significant bit
			 * @return Position of removed bit or BBObject::noBit if empty
			 * @post pc_ decremented if bit was found
			 * @par Complexity: O(1) with hardware support
			 */
			int pop_lsb() {
				if (pc_ > 0) { 
					int bit = bb_.lsb(); 
					bb_.erase_bit(bit); 
					--pc_; 
					return bit; 
				}
				return BBObject::noBit;
			}

			/**
			 * @brief Synchronizes counter with actual bitset population
			 * @return Updated population count
			 * @post pc_ == bb_.size()
			 * @par Complexity: O(n/64)
			 */
			int sync_pc() { pc_ = bb_.size(); return pc_; }

			/**
			 * @brief Checks if bitset is empty using cached counter
			 * @return true if no bits are set
			 * @par Complexity: O(1)
			 */
			bool is_empty() const noexcept { return (pc_ == 0); }
			
			/**
			 * @brief Verifies counter synchronization
			 * @return true if cached count matches actual population
			 * @par Complexity: O(n/64)
			 */
			bool is_sync_pc() const { return (pc_ == bb_.size()); }

			//operators
			friend bool operator ==	(const bbSize_t& lhs, const bbSize_t& rhs) { return (lhs.pc_ == rhs.pc_) && (lhs.bb_ == rhs.bb_); }
			friend bool operator !=	(const bbSize_t& lhs, const bbSize_t& rhs) { return !(lhs == rhs); }

			//I/O
			std::ostream& print(std::ostream& o = std::cout, bool show_pc = true, bool eofl = true)	const;

			/////////////////
			// data members

			BITBOARD pc_;																	//number of 1-bits
			BitSet_t bb_;																	//any type of the BBObject hierarchy	
		};

		template <class BitSet_t>
		std::ostream& bbSize_t<BitSet_t>::print(std::ostream& o, bool show_pc, bool eofl) const {

			bb_.print(o, true, false);
			if (show_pc) { o << "[" << pc_ << "]"; }
			if (eofl) { o << std::endl; }
			return o;
		}

		//////////////////////
		// 
		// bbStack_t class	 
		// 
		// @brief: a very simple wrapper for a composite vector / bitset with stack interface
		// 
		// @details: created 28/13/17, refactored 27/01/2025
		// 
		//////////////////////

		template <class BitSet_t>
		struct bbStack_t {

			enum print_t { STACK = 0, BITSET };

			//construction / destruction
			bbStack_t(int MAX_SIZE) : bb_(MAX_SIZE) {}
			bbStack_t() {}

			//move and copy semantics - copy and move semantics forbidden
			bbStack_t(const bbStack_t& rhs) = delete;
			bbStack_t& operator =	(const bbStack_t& rhs) noexcept = delete;

			//allocation

			void reset(int MAX_SIZE);

			//setters and getters
			int  size() { return stack_.size(); }

			//stack interface (bits are added / removed according to the stack)
			void push(int elem);
			int pop();

			//basic operations

				//clears bits from the bitset according to stack
			void erase_bit();

			//synchro stack according to bitset (ordered from lsb to msb)
			void sync_stack();

			//synchro bitset according to stack
			void sync_bb();

			//boolean
				/* may occur if data is manipulated directly */
				//checks if the contents in the STACK is in BB - currently the opposite is not checked
			bool is_sync();

			bool is_empty() { return (stack_.empty()); }

			//I/O
			std::ostream& print(print_t t = STACK, std::ostream& o = std::cout, bool eofl = true);

			/////////////////
			// data members

			BitSet_t bb_;
			std::vector<int> stack_;

		}; //end bbStack_t class

		//////////////////////
		// 
		// bbCol_t class 
		// 
		// @brief  a very simple wrapper for a collection of bitsets of a fixed size
		// @details: created 9/8/17 for MWCP upper bound computation, refactored 27/02/2025
		// 
		///////////////////////

		template <class BitSet_t, int SIZE>
		struct bbCol_t {

			using basic_type = BitSet_t;
			using type = bbCol_t<BitSet_t, SIZE>;

			//contructors / destructors
			bbCol_t(int popCount) {
				reset(popCount);
			}

			//TODO move and copy semantics...->Done

			// NUEVa implementacionn
			/* En principio la propia clase BitSet_t manejaria la construccion
			bbCol_t(const bbCol_t&) = default;
			bbCol_t& operator=(const bbCol_t&) = default;
			bbCol_t(bbCol_t&&) noexcept = default;
			bbCol_t& operator=(bbCol_t&&) noexcept = default;*/

		//allocation
			void reset(int popCount) {
				try {
					for (int i = 0; i < SIZE; i++) {
						bb_[i].reset(popCount);
					}
				}
				catch (std::exception& e) {
					LOG_ERROR(e.what());
					LOG_ERROR("bbCol_t::bbCol_t()");
					std::exit(-1);
				}
			}

			//setters and getters
			int number_of_bitblocks(int bitsetID) { return bb_[bitsetID].number_of_bitblocks(); }
			int size(int bitsetID) { return bb_[bitsetID].size(); }
			int capacity() { return SIZE; }

			//basic operations
			BitSet& set_bit(int bitsetID, int bit) { return std::ref((bb_[bitsetID].set_bit(bit))); }

			/**
			* @brief sets the bit in the position pos of the bitset at bitsetID
			*		 in the collection
			* @param bitsetID: the position of the bitset in the array
			* @param bit: the bit to set
			* @param is_first_bit: true if bit is now the first bit of the bitset
			* @returns a reference to the bitset modified
			**/
			BitSet& set_bit(int bitsetID, int bit, bool& is_first_bit);

			BitSet& erase_bit(int bitsetID, int bit) { return std::ref((bb_[bitsetID].erase_bit(bit))); }

			/**
			* @brief clears all 1-bits in the bitset bb_[bitsetID]
			**/
			BitSet& erase_bit(int bitsetID) { return std::ref((bb_[bitsetID].erase_bit())); }

			//bool
			bool is_bit(int pos, int bit) { return bb_[pos].is_bit(bit); }


			//I/O
			std::ostream& print(std::ostream& o = std::cout, bool show_pc = true, bool eofl = true)	 const;
			//TODO... others


		/////////////
		// data members
			static const int capacity_ = SIZE;
			std::array<BitSet_t, SIZE> bb_;
		};

	}//end namespace _impl

	using _impl::bbSize_t;
	using _impl::bbStack_t;
	using _impl::bbCol_t;	

	using _impl::operator==;
	using _impl::operator!=;

}//end namespace bitgraph

///////////////////////////////////////////////////////////
// Necessary header implementations for generic code


namespace bitgraph {

		template <class BitSet_t, int SIZE>
		inline
			BitSet& bbCol_t<BitSet_t, SIZE>::set_bit(int bitsetID, int bit, bool& is_first_bit) {

			//adds bit
			bb_[bitsetID].set_bit(bit);

			//checks if the bit added is the first bit of the bitset
			is_first_bit = (bit == bb_[bitsetID].lsb());

			//returns a reference to the modified bitset
			return 	std::ref(bb_[bitsetID]);
		}


		template <class BitSet_t, int SIZE>
		inline
			std::ostream& bbCol_t<BitSet_t, SIZE>::print(std::ostream& o, bool show_pc, bool eofl)  const {
			for (auto i = 0; i < bb_.size(); ++i) {
				if (!bb_[i].is_empty()) {
					bb_[i].print(o, show_pc, true);
				}
			}
			if (eofl) { o << std::endl; }
			return o;
		}

		template <class BitSet_t>
		inline
			std::ostream& bbStack_t<BitSet_t>::print(print_t t, std::ostream& o, bool eofl) {

			switch (t) {
			case STACK:
				o << "[";
				for (auto i = 0; i < stack_.size(); ++i) {
					o << stack_[i] << " ";
				}
				o << "]" << std::endl;
				break;
			case BITSET:
				bb_.print(o, true, false);		//size info and no end of line
				break;
			default:
				; //error
			}

			if (eofl) { o << std::endl; }
			return o;
		}

		template <class BitSet_t>
		inline
			bool bbStack_t<BitSet_t>::is_sync() {
				if (bb_.size() != stack_.size()) { 
					return false; 
				}

				//CODIGO ORIGINAL
				//checks if exactly the population of bb_ is in the STACK  
				// for (auto i = 0; i < stack_.size(); ++i) {
				// 	if (!bb_.is_bit(stack_[i])) {
				// 		return false;
				// 	}
				// }

				return std::all_of(stack_.begin(), stack_.end(),
					[this](int bit) { return bb_.is_bit(bit); });
		}

		template <class BitSet_t>
		inline
			void bbStack_t<BitSet_t>::sync_bb() {
				bb_.erase_bit();
				//CODIGO ORIGINAL
				// 	for (auto i = 0; i < stack_.size(); i++) {
				// 	bb_.set_bit(stack_[i]);
				// }
				for (int bit : stack_) {
					bb_.set_bit(bit);
				}
		}

		template <class BitSet_t>
		inline
			void bbStack_t<BitSet_t>::sync_stack() {

			//cleans stack
			stack_.clear();

			//bitscanning with nested data structure
			typename BitSet_t::scan sc(bb_);
			if (sc.init_scan() != -1) {
				int bit = BBObject::noBit;
				while ((bit = bb_.next_bit()) != BBObject::noBit) {
					stack_.emplace_back(bit);
				}
			}

		}

		template <class BitSet_t>
		inline
			void  bbStack_t<BitSet_t>::reset(int MAX_SIZE) {

			//cleans stack
			stack_.clear();

			//allocates memory	
			try {
				/////////////////////////////
				bb_.reset(MAX_SIZE);
				stack_.clear();
				////////////////////////////
			}
			catch (std::exception& e) {
				LOG_ERROR(e.what());
				LOG_ERROR("bbStack_t<BitSet_t>::-reset()");
				std::exit(-1);
			}

		}; //end struct


		template <class BitSet_t>
		inline
			void bbStack_t<BitSet_t>::push(int bit) {

			if (!bb_.is_bit(bit)) {
				bb_.set_bit(bit);
				stack_.emplace_back(bit);
			}
		}

		template <class BitSet_t>
		inline
			int bbStack_t<BitSet_t>::pop() {

			if (stack_.size() > 0) {
				int bit = stack_.back();
				stack_.pop_back();
				bb_.erase_bit(bit);
				return bit;
			}
			else return BBObject::noBit;
		}

		template <class BitSet_t>
		inline
			void bbStack_t<BitSet_t>::erase_bit() {
				//CODIGO ORIGINAL
				for (int i = 0; i < stack_.size(); i++) {
				bb_.erase_bit(stack_[i]);
				}
				for (int bit : stack_) {
					bb_.erase_bit(bit);
				}
		}


}//end namespace bitgraph



namespace bitgraph {

	///////////////////////
	//
	// Stateless functions for BitSets
	// (namespace bbalg)
	//
	//////////////////////

	namespace bbalg {

		using _impl::Tables;

		template<class BitSet_t>
		inline
			std::vector<int> to_vector(BitSet_t& bbn) {

			std::vector<int> res;
			res.reserve(bbn.size());

			//uses primitive bitscanning which is compatible for all bitsets in the hierarchy
			int v = BBObject::noBit;
			while ((v = bbn.next_bit(v)) != BBObject::noBit) {
				res.emplace_back(v);
			}

			return res;
		}


		inline
			BITBOARD gen_random_block(double p) noexcept {

			BITBOARD bb = 0;

			for (auto i = 0; i < WORD_SIZE; i++) {
				if (com::_rand::uniform_dist(p)) {
					bb |= Tables::mask[i];
				}
			}
			return bb;
		}

		template<class BitSet_t>
		inline
			int first_k_bits(int k, BitSet_t& bb, std::vector<int>& lv) {

			lv.clear();

			///////////////////////////////////////////////////////////////
			if (bb.init_scan(bbo::NON_DESTRUCTIVE) == -1) { return 0; }
			///////////////////////////////////////////////////////////////

			int nBits = 0;
			int bit = BBObject::noBit;
			// Fixed bug: condition should be AND not OR
			while ((bit = bb.next_bit()) != BBObject::noBit && nBits < k) {
				lv.emplace_back(bit);
				++nBits;
			}

			//number of bits found (ideally the first-k)
			return nBits;
		}

	}//end namespace bbalg


}//end namespace bitgraph


#endif

