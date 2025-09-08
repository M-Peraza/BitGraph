/**  
 * @file bbobject.h
 * @brief Base class and scanning interfaces for the BITSCAN library hierarchy
 * @author Pablo San Segundo
 * @version 1.0
 * @date 2025
 * @since 14/02/2025
 * 
 * @details This file defines the foundational abstract class BBObject that serves
 * as the base for all bitset classes in BITSCAN. It provides:
 * 
 * - Common interface for I/O operations
 * - Scan type enumeration for different bitscanning modes  
 * - Template-based scanning helper classes (iterator-like pattern)
 * - Internal scan state management structures
 * 
 * The scanning system supports both destructive and non-destructive modes,
 * forward and reverse directions, making it highly flexible for different 
 * algorithmic needs in combinatorial optimization.
 * 
 * @note The template scanning classes follow an iterator-like pattern but are
 * specifically optimized for bitset operations rather than generic containers.
 **/

#ifndef  __BB_OBJECT_H__
#define  __BB_OBJECT_H__

#include <iostream>
#include "bitscan/bbtypes.h"
#include "bitscan/bbconfig.h"

namespace bitgraph {
	namespace _impl {
		/**
		 * @class BBObject  
		 * @brief Abstract base class for the BITSCAN library hierarchy
		 * @details BBObject serves as the foundation for all bitset classes, providing:
		 * - Common constants and type definitions
		 * - Scan type enumeration for bitscanning operations
		 * - Pure virtual I/O interface that derived classes must implement
		 * - Template-based scanning helper classes
		 * 
		 * This design allows for polymorphic behavior while maintaining high performance
		 * through template specialization in derived classes.
		 */
		class BBObject {
		public:
			/**
			 * @brief Sentinel value indicating no valid bit position
			 * @details Used throughout BITSCAN to indicate:
			 * - End of bitscanning operations (no more bits found)
			 * - Invalid bit positions in error conditions  
			 * - Uninitialized state in scanning structures
			 */
			static const int noBit = -1;

			/**
			 * @enum scan_types
			 * @brief Enumeration of bitscanning operation modes
			 * @details Defines the four fundamental scanning patterns supported:
			 * 
			 * **Non-Destructive modes**: Preserve the original bitset
			 * - NON_DESTRUCTIVE: Forward scan (LSB to MSB)
			 * - NON_DESTRUCTIVE_REVERSE: Reverse scan (MSB to LSB)
			 * 
			 * **Destructive modes**: Remove bits as they are scanned
			 * - DESTRUCTIVE: Forward scan with bit removal
			 * - DESTRUCTIVE_REVERSE: Reverse scan with bit removal
			 */
			enum scan_types { 
				NON_DESTRUCTIVE,        ///< Forward scan preserving bits
				NON_DESTRUCTIVE_REVERSE,///< Reverse scan preserving bits  
				DESTRUCTIVE,            ///< Forward scan removing bits
				DESTRUCTIVE_REVERSE     ///< Reverse scan removing bits
			};

			/**
			 * @brief Stream output operator for bitset objects
			 * @param o Output stream
			 * @param bb BBObject-derived bitset to output
			 * @return Reference to the output stream
			 * @details Delegates to the pure virtual print() method implemented by derived classes
			 */
			friend std::ostream& operator<< (std::ostream& o, const BBObject& bb) { bb.print(o); return o; }

			/**
			 * @defgroup ScanningStructures Bitscanning Data Structures
			 * @brief Internal structures for efficient bitset scanning operations
			 * @{
			 */

			/**
			 * @struct scan_t
			 * @brief Scan state cache for efficient bitscanning operations
			 * @details This structure maintains the current scanning position to avoid
			 * recomputing bit and block indices on each scan operation. It significantly
			 * improves performance for iterative bitscanning by caching:
			 * - Current bitblock index 
			 * - Current bit position within the block
			 * 
			 * The cache enables O(1) continuation of scans rather than O(n) restart.
			 */
			struct scan_t {
				/** @brief Current bitblock index being scanned */
				int bbi_;								
				
				/** @brief Bit position within current block [0-63] */
				int pos_;								

				/**
				 * @brief Default constructor initializing to invalid scan state
				 * @details Sets bbi_ to noBit (-1) and pos_ to MASK_LIM, indicating
				 * no active scan is in progress.
				 */
				scan_t() :bbi_(noBit), pos_(MASK_LIM) {}

				/**
				 * @brief Set the current bitblock for scanning
				 * @param block Block index to set as current scanning position
				 */
				void set_block(int block) { bbi_ = block; }
				
				/**
				 * @brief Set the bit position within the current block
				 * @param bit Bit position [0-63] within current block
				 */
				void set_pos(int bit) { pos_ = bit; }
				
				/**
				 * @brief Set scan position from absolute bit index
				 * @param bit Absolute bit index in the bitset
				 * @details Computes both block index and bit position from absolute bit index.
				 * Uses optimized WDIV/WMUL operations for efficiency.
				 */
				void set_bit(int bit) {
					int bbh = WDIV(bit);
					bbi_ = bbh;
					pos_ = bit - WMUL(bbh);					/* equiv. to WMOD(bit)*/
				}
			};

			/** @} */ // end ScanningStructures group

			/**
			 * @defgroup ScanningTemplates Template-Based Scanning Classes  
			 * @brief templates for different bitscanning patterns
			 * @details These template classes provide a convenient, iterator-like interface
			 * for bitset scanning operations. They encapsulate the four scanning modes
			 * (destructive/non-destructive × forward/reverse) in type-safe templates.
			 * 
			 * @note Currently, BitSetT template parameter must be derived from BBScan class.
			 * @since 14/02/2025
			 * @{
			 */

			/**
			 * @class ScanRev
			 * @brief Reverse (MSB to LSB) non-destructive bitset scanner
			 * @tparam BitSetT Bitset type derived from BBScan class
			 * @details Provides iterator-like interface for scanning bits from most significant
			 * to least significant bit without modifying the original bitset. Useful for
			 * algorithms that need to process bits in reverse order while preserving the data.
			 */
			template< class BitSetT >
			struct ScanRev {
				using basic_type = BitSetT;  ///< Type alias for template parameter
				using type = ScanRev<BitSetT>; ///< Type alias for this scanner type

			public:
								
				/**
				 * @brief Constructor for reverse bitscanning
				 * @param bb Reference to bitset to scan
				 * @param firstBit Starting bit index (-1 for full scan from MSB)
				 * @throws May throw for sparse bitsets if empty 
				 * @details Initializes reverse scanning from specified bit position or from
				 * the most significant bit if firstBit is -1.
				 */
				ScanRev(BitSetT& bb, int firstBit = -1) : bb_(bb) { init_scan(firstBit); }

				/**
				 * @brief Get current bitblock index being scanned
				 * @return Current bitblock index from scan state
				 */
				int get_block() { return  bb_.scan_.bbi_; }

				/**
				 * @brief Initialize reverse bitscanning from specified position
				 * @param firstBit Starting bit index (-1 for full bitset scan)
				 * @return 0 if successful, -1 otherwise
				 * @details Sets up reverse scanning from firstBit towards bit 0.
				 * If firstBit = -1, scans from MSB to LSB of entire bitset.
				 * @note May throw for sparse bitsets if empty
				 */
				int init_scan(int firstBit = -1) { return bb_.init_scan(firstBit, BBObject::NON_DESTRUCTIVE_REVERSE); }

				/**
				 * @brief Get next bit in reverse scanning order
				 * @return Next bit index in reverse order, noBit if scan complete
				 * @details Returns bits in decreasing order (MSB to LSB) without
				 * modifying the original bitset.
				 */
				int next_bit() { return bb_.prev_bit(); }

				/**
				 * @brief Get next bit and remove it from specified bitset
				 * @param bitSet Bitset to remove the found bit from
				 * @return Next bit index in reverse order, noBit if scan complete
				 * @details Scans in reverse order but removes found bits from bitSet parameter,
				 * not from the bitset being scanned. Useful for set operations.
				 */
				int next_bit(BitSetT& bitSet) { return bb_.prev_bit(bitSet); }

			private:
				BitSetT& bb_;
			};


			template< class BitSetT >
			struct Scan {
				using basic_type = BitSetT;
				using type = Scan<BitSetT>;

			public:

				/**
				* @brief: constructor for bitscanning - may throw for sparse bitsets if empty
				**/
				Scan(BitSetT& bb, int firstBit = -1) : bb_(bb) { init_scan(firstBit); }

				int get_block() { return  bb_.scan_.bbi_; }

				/**
				* @brief Scans the bitset from [firstBit , end of the bitset)
				*		 If firstBit = -1 scans the whole bitset
				* @param firstBit: starting position of the scan
				* @details: may throw for sparse bitsets if empty
				**/
				int init_scan(int firstBit = -1) { return bb_.init_scan(firstBit, BBObject::NON_DESTRUCTIVE); }

				/**
				* @brief returns the next bit in the bitset during a reverse bitscanning operation
				**/
				int next_bit() { return bb_.next_bit(); }

				/**
				* @brief scans the next bit in the bitset and deletes it from the
				*		 bitstring bitSet
				* @param bbdel: bitset to delete the bit from
				**/
				int next_bit(BitSetT& bitSet) { return bb_.next_bit(bitSet); }

			private:
				BitSetT& bb_;
			};


			template< class BitSetT >
			struct ScanDest {
				using basic_type = BitSetT;
				using type = ScanDest<BitSetT>;

			public:

				/**
				* @brief: constructor for destructive bitscanning - may throw for sparse bitsets if empty
				**/
				ScanDest(BitSetT& bb) : bb_(bb) { init_scan(); }

				int get_block() { return bb_.scan_.bbi_; }

				/**
				* @brief Scans the bitset in the range [0 , end of the bitset)
				*		 Removes bits as they are scanned
				* @details: may throw for sparse bitsets if empty
				**/
				int init_scan() { return bb_.init_scan(BBObject::DESTRUCTIVE); }

				/**
				* @brief returns the next bit in the bitset during a reverse bitscanning operation
				**/
				int next_bit() { return bb_.next_bit_del(); }

				/**
				* @brief scans the next bit in the bitset and deletes it from the
				*		 bitstring bitSet
				* @param bbdel: bitset to delete the bit from
				**/
				int next_bit(BitSetT& bitSet) { return bb_.next_bit_del(bitSet); }

			private:
				BitSetT& bb_;
			};


			template< class BitSetT >
			struct ScanDestRev {
				using basic_type = BitSetT;
				using type = ScanDest<BitSetT>;

			public:

				/**
				* @brief: constructor for destructive reverse bitscanning - may throw for sparse bitsets if empty
				**/
				ScanDestRev(BitSetT& bb) : bb_(bb) { init_scan(); }

				int get_block() { return bb_.scan_.bbi_; }

				/**
				* @brief Scans the bitset in the range (end_of_bitset, 0]
				*		 Removes bits as they are scanned
				* @details: may throw for sparse bitsets if empty
				**/
				int init_scan() { return bb_.init_scan(BBObject::DESTRUCTIVE_REVERSE); }

				/**
				* @brief returns the next bit in the bitset during a reverse bitscanning operation
				**/
				int next_bit() { return bb_.prev_bit_del(); }

				/**
				* @brief scans the next bit in the bitset during a reverse bitscanning operation
				*		 and deletes it from the bitstring  bitSet
				* @param bbdel: bitset to delete the bit from
				**/
				int next_bit(BitSetT& bitSet) { return bb_.prev_bit_del(bitSet); }

			private:
				BitSetT& bb_;
			};


			/** @} */ // end ScanningTemplates group

			/**
			 * @defgroup BaseInterface Base Class Interface
			 * @brief Core interface methods that derived classes must implement
			 * @{
			 */

			/**
			 * @brief Default constructor for BBObject
			 * @details Initializes base class with default values. Derived classes
			 * should call this constructor and initialize their specific members.
			 */
			BBObject() = default;

			/**
			 * @brief Virtual destructor for proper polymorphic cleanup
			 * @details Ensures derived classes are properly destructed when accessed
			 * through BBObject pointers. Critical for avoiding resource leaks.
			 */
			virtual ~BBObject() = default;

			/**
			 * @brief Pure virtual method for bitset output formatting
			 * @param o Output stream to write to (default: std::cout)
			 * @param show_pc Whether to show population count (default: true)
			 * @param endl Whether to add newline at end (default: true)
			 * @return Reference to output stream for method chaining
			 * @details Derived classes must implement this method to provide
			 * formatted output of their bitset contents. Common format includes
			 * bit pattern and population count.
			 */
			virtual std::ostream& print(std::ostream& o = std::cout,
				bool show_pc = true,
				bool endl = true) const = 0;

			/** @} */ // end BaseInterface group
		};

	}// namespace _impl

	using _impl::BBObject;

	namespace _impl {
		std::ostream& operator<< (std::ostream& o, const BBObject& bb);
	}	
	using _impl::operator<<;

}// namespace bitgraph

#endif