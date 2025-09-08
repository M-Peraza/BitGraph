/**
 * @file bbsentinel.cpp file
 * @brief Source file of the BBSentinel class (header bbsentinel.h)
 *		   Manages efficient bitset operations by circumscribing them to the range [low_sentinel, high_sentinel]
 * @details The lower sentinel is the first non-zero bitblock in the bitstring
 * @detials The higher sentinel is the last non-zero bitblock in the bitstring
 * @details created?,  last_updated 13/02/2025
 * @author pss
 *
 * TODO- EXPERIMENTAL - NOT CHECKED (13/02/2025)
 **/

#include "bbsentinel.h"
#include <algorithm>			//std::max, std::any_of, std::transform, std::copy
#include <iostream>

using namespace std;

using namespace bitgraph;

namespace bitgraph {

	namespace _impl {

		BBSentinel& AND(const BitSet& lhs, const BBSentinel& rhs, BBSentinel& res) {
			res.m_BBL = rhs.m_BBL;
			res.m_BBH = rhs.m_BBH;
	
			for (int i = rhs.m_BBL; i <= rhs.m_BBH; i++) {
				res.vBB_[i] = lhs.block(i) & rhs.vBB_[i];
			}
			return res;
		}

	}

	using _impl::AND;

}

void BBSentinel::init_sentinels(bool update) {
	// Sets sentinels to maximum scope of current bit string
	m_BBL = 0; 
	m_BBH = nBB_ - 1;
	if (update) {
		update_sentinels();
	}
}

void BBSentinel::set_sentinels(int low, int high) {
	m_BBL = low;
	m_BBH = high;
}

void BBSentinel::clear_sentinels() noexcept {
	m_BBL = EMPTY_ELEM;
	m_BBH = EMPTY_ELEM;
}

int BBSentinel::update_sentinels() {
	//CODIGO ORIGINAL
	// //empty check 
	// if(m_BBL==EMPTY_ELEM || m_BBH==EMPTY_ELEM) return EMPTY_ELEM;
	// 
	// //Low sentinel
	// if(!vBB_[m_BBL]){
	// 	for(m_BBL=m_BBL+1; m_BBL<=m_BBH; m_BBL++){
	// 		if(vBB_[m_BBL])
	// 				goto high;
	// 	}
	// 	m_BBL=EMPTY_ELEM;
	// 	m_BBH=EMPTY_ELEM;
	// 	return EMPTY_ELEM;
	// }
	// 
	// //High sentinel
	// high:	;
	// if(!vBB_[m_BBH]){
	// 	for(m_BBH=m_BBH-1; m_BBH>=m_BBL; m_BBH--){
	// 		if(vBB_[m_BBH])
	// 				return 0;
	// 		}
	// 	m_BBL=EMPTY_ELEM;
	// 	m_BBH=EMPTY_ELEM;
	// 	return EMPTY_ELEM;
	// }
	// return 0;

	// Updates sentinels - returns EMPTY_ELEM or 0 (ok)
	if (m_BBL == EMPTY_ELEM || m_BBH == EMPTY_ELEM) {
		return EMPTY_ELEM;
	}

	// Update low sentinel - find first non-zero block
	if (!vBB_[m_BBL]) {
		// Use STL to find first non-zero block
		auto it = std::find_if(vBB_.data() + m_BBL + 1, vBB_.data() + m_BBH + 1,
			[](BITBOARD block) { return block != 0; });
		
		if (it == vBB_.data() + m_BBH + 1) {
			// No non-zero blocks found
			m_BBL = EMPTY_ELEM;
			m_BBH = EMPTY_ELEM;
			return EMPTY_ELEM;
		}
		m_BBL = std::distance(vBB_.data(), it);
	}

	// Update high sentinel - find last non-zero block
	if (!vBB_[m_BBH]) {
		// Search backwards for last non-zero block
		for (int i = m_BBH - 1; i >= m_BBL; --i) {
			if (vBB_[i]) {
				m_BBH = i;
				return 0;
			}
		}
		// No non-zero blocks found
		m_BBL = EMPTY_ELEM;
		m_BBH = EMPTY_ELEM;
		return EMPTY_ELEM;
	}
	
	return 0;
}

int BBSentinel::update_sentinels(int bbl, int bbh) {
	//CODIGO ORIGINAL
	// //empty check 
	// if(bbl==EMPTY_ELEM || bbh==EMPTY_ELEM) return EMPTY_ELEM;
	// 
	// m_BBL=bbl;
	// m_BBH=bbh;
	// 
	// //Low
	// if(!vBB_[m_BBL]){
	// 	for(m_BBL=m_BBL+1; m_BBL<=m_BBH; m_BBL++){
	// 		if(vBB_[m_BBL])
	// 				goto high;
	// 	}
	// 	m_BBL=EMPTY_ELEM;
	// 	m_BBH=EMPTY_ELEM;
	// 	return EMPTY_ELEM;
	// }
	// 
	// //High
	// high:	;
	// 	if(!vBB_[m_BBH]){
	// 	for(m_BBH=m_BBH-1; m_BBH>=m_BBL; m_BBH--){
	// 		if(vBB_[m_BBH])
	// 				return 0;
	// 	}
	// 	m_BBL=EMPTY_ELEM;
	// 	m_BBH=EMPTY_ELEM;
	// 	return EMPTY_ELEM;
	// }
	// return 0;

	// Updates sentinels within specified range
	if (bbl == EMPTY_ELEM || bbh == EMPTY_ELEM) {
		return EMPTY_ELEM;
	}

	m_BBL = bbl;
	m_BBH = bbh;

	// Update low sentinel - find first non-zero block
	if (!vBB_[m_BBL]) {
		// Use STL to find first non-zero block
		auto it = std::find_if(vBB_.data() + m_BBL + 1, vBB_.data() + m_BBH + 1,
			[](BITBOARD block) { return block != 0; });
		
		if (it == vBB_.data() + m_BBH + 1) {
			// No non-zero blocks found
			m_BBL = EMPTY_ELEM;
			m_BBH = EMPTY_ELEM;
			return EMPTY_ELEM;
		}
		m_BBL = std::distance(vBB_.data(), it);
	}

	// Update high sentinel - find last non-zero block
	if (!vBB_[m_BBH]) {
		// Search backwards for last non-zero block
		for (int i = m_BBH - 1; i >= m_BBL; --i) {
			if (vBB_[i]) {
				m_BBH = i;
				return 0;
			}
		}
		// No non-zero blocks found
		m_BBL = EMPTY_ELEM;
		m_BBH = EMPTY_ELEM;
		return EMPTY_ELEM;
	}
	
	return 0;
}

int BBSentinel::update_sentinels_high() {
	//CODIGO ORIGINAL
	// //empty check 
	// if(m_BBH==EMPTY_ELEM) return EMPTY_ELEM;
	// 
	// //Update High
	// if(!vBB_[m_BBH]){
	// 	for(m_BBH=m_BBH-1; m_BBH>=m_BBL; m_BBH--){
	// 		if(vBB_[m_BBH])
	// 				return 0;
	// 	}
	// 
	// 	m_BBL=EMPTY_ELEM;
	// 	m_BBH=EMPTY_ELEM;
	// 	return EMPTY_ELEM;
	// }
	// return 0;

	if (m_BBH == EMPTY_ELEM) {
		return EMPTY_ELEM;
	}

	// Update high sentinel if current block is empty
	if (!vBB_[m_BBH]) {
		// Search backwards for last non-zero block
		for (int i = m_BBH - 1; i >= m_BBL; --i) {
			if (vBB_[i]) {
				m_BBH = i;
				return 0;
			}
		}
		// No non-zero blocks found
		m_BBL = EMPTY_ELEM;
		m_BBH = EMPTY_ELEM;
		return EMPTY_ELEM;
	}
	return 0;
}

int BBSentinel::update_sentinels_low() {
	//CODIGO ORIGINAL
	// //empty check
	// if(m_BBL==EMPTY_ELEM) return EMPTY_ELEM ;
	// 
	// //Update High
	// if(!vBB_[m_BBL]){
	// 	for(m_BBL=m_BBL+1; m_BBL<=m_BBH; m_BBL++){
	// 		if(vBB_[m_BBL])
	// 				return 0;
	// 	}
	// 	m_BBL=EMPTY_ELEM;
	// 	m_BBH=EMPTY_ELEM;
	// 	return EMPTY_ELEM;
	// }
	// return 0;

	if (m_BBL == EMPTY_ELEM) {
		return EMPTY_ELEM;
	}

	// Update low sentinel if current block is empty
	if (!vBB_[m_BBL]) {
		// Use STL to find first non-zero block
		auto it = std::find_if(vBB_.data() + m_BBL + 1, vBB_.data() + m_BBH + 1,
			[](BITBOARD block) { return block != 0; });
		
		if (it != vBB_.data() + m_BBH + 1) {
			m_BBL = std::distance(vBB_.data(), it);
			return 0;
		}
		// No non-zero blocks found
		m_BBL = EMPTY_ELEM;
		m_BBH = EMPTY_ELEM;
		return EMPTY_ELEM;
	}
	return 0;
}

void BBSentinel::update_sentinels_to_v(int v) {
	//CODIGO ORIGINAL
	// int bb_index=WDIV(v);
	// if(m_BBL==EMPTY_ELEM || m_BBH==EMPTY_ELEM){
	// 	m_BBL=m_BBH=bb_index;
	// }else if(m_BBL>bb_index){
	// 	m_BBL=bb_index;
	// }else if(m_BBH<bb_index){
	// 	m_BBH=bb_index;
	// }

	// Adapts sentinels to include vertex v
	int bb_index = WDIV(v);
	
	if (m_BBL == EMPTY_ELEM || m_BBH == EMPTY_ELEM) {
		// Initialize sentinels to block containing v
		m_BBL = m_BBH = bb_index;
	} else {
		// Expand sentinels if necessary using std::min/max
		m_BBL = std::min(m_BBL, bb_index);
		m_BBH = std::max(m_BBH, bb_index);
	}
}

std::ostream& BBSentinel::print(std::ostream& o, bool show_pc, bool endl) const {
	BitSet::print(o, show_pc, endl);
	o << "(" << m_BBL << "," << m_BBH << ")";
	return o;
}

int BBSentinel::init_scan(scan_types sct) noexcept {
	switch(sct){
	case NON_DESTRUCTIVE:
		update_sentinels();
		scan_.bbi_=m_BBL;
		scan_.pos_=MASK_LIM; 
		break;
	case NON_DESTRUCTIVE_REVERSE:
		update_sentinels();
		scan_.bbi_=m_BBH;
		scan_.pos_=WORD_SIZE;		//mask_right[WORD_SIZE]=bitgraph::constants::ALL_ONES
		break;
	case DESTRUCTIVE:				//uses sentinels to iterate and updates them on the fly
		update_sentinels();
		break;
	case DESTRUCTIVE_REVERSE:		//uses sentinels to iterate and updates them on the fly
		update_sentinels();
		break;
	default:
		cerr<<"bad scan type"<<endl;
		std::exit(EXIT_FAILURE);
	}
return 0;
}


void BBSentinel::erase_bit() {
	//CODIGO ORIGINAL
	// if(m_BBL==EMPTY_ELEM || m_BBH==EMPTY_ELEM) return; 
	// 
	// for(int i=m_BBL; i<=m_BBH; i++)	
	// 			vBB_[i]=bitgraph::constants::ALL_ZEROS;

	// Clears all bits in the sentinel range
	if (m_BBL == EMPTY_ELEM || m_BBH == EMPTY_ELEM) {
		return;
	}

	// Use std::fill for cleaner code
	std::fill(vBB_.data() + m_BBL, vBB_.data() + m_BBH + 1, bitgraph::constants::ALL_ZEROS);
}


BBSentinel& BBSentinel::erase_bit(const BitSet& bbn) {
	//CODIGO ORIGINAL
	// for(int i=m_BBL; i<=m_BBH; i++)
	// 	vBB_[i] &= ~ bbn.block(i);		//**access
	// 
	// return *this;

	// Deletes 1-bits in bbn within current sentinel range
	if (m_BBL == EMPTY_ELEM || m_BBH == EMPTY_ELEM) {
		return *this;
	}
	
	// Apply AND-NOT operation within sentinel range
	for (int i = m_BBL; i <= m_BBH; ++i) {
		vBB_[i] &= ~bbn.block(i);
	}
	
	return *this;
}

void BBSentinel::erase_bit_and_update(int nBit) {
	//CODIGO ORIGINAL
	// if(m_BBL==EMPTY_ELEM || m_BBH==EMPTY_ELEM ) return;		//empty bitsring
	// 
	// int bb=WDIV(nBit);
	// vBB_[bb] &= ~Tables::mask[WMOD(nBit)];
	// 
	// //update watched literals if necessary
	// if(!vBB_[bb]){
	// 	if(m_BBL==bb){
	// 		for(m_BBL=m_BBL+1; m_BBL<=m_BBH; m_BBL++){
	// 			if(vBB_[m_BBL])
	// 					return;
	// 		}
	// 		m_BBL=EMPTY_ELEM;
	// 		m_BBH=EMPTY_ELEM;
	// 		return;
	// 	}
	// 	
	// 	if(m_BBH==bb){
	// 		for(m_BBH=m_BBH-1; m_BBH>=m_BBL; m_BBH--){
	// 			if(vBB_[m_BBH])
	// 					return;
	// 		}
	// 		m_BBL=EMPTY_ELEM;
	// 		m_BBH=EMPTY_ELEM;
	// 		return;
	// 	}
	// }

	if (m_BBL == EMPTY_ELEM || m_BBH == EMPTY_ELEM) {
		return;  // Empty bitstring
	}
	
	int bb = WDIV(nBit);
	vBB_[bb] &= ~Tables::mask[WMOD(nBit)];

	// Update sentinels if the block became empty
	if (!vBB_[bb]) {
		if (m_BBL == bb) {
			// Update low sentinel
			update_sentinels_low();
		} else if (m_BBH == bb) {
			// Update high sentinel
			update_sentinels_high();
		}
	}
}	



bool BBSentinel::is_empty() const {
	// New definition of emptiness with sentinels
	return (m_BBL == EMPTY_ELEM || m_BBH == EMPTY_ELEM);
}

bool BBSentinel::is_empty(int nBBL, int nBBH) const {
	// Check emptiness in the intersection of ranges
	int bbl = std::max(nBBL, m_BBL);
	int bbh = std::min(nBBH, m_BBH);
	
	// Already uses STL - just improve formatting
	return !std::any_of(vBB_.data() + bbl, vBB_.data() + bbh + 1, 
		[](BITBOARD block) { return block != 0; });
}

BBSentinel& BBSentinel::operator=(const BBSentinel& bbs) {
	//CODIGO ORIGINAL
	// m_BBL=bbs.m_BBL;
	// m_BBH=bbs.m_BBH;
	// //for(int i=m_BBL; i<=m_BBH; i++){
	// //	this->vBB_[i]=bbs.vBB_[i];
	// //}
	// 
	// std::copy(bbs.vBB_.data() + m_BBL, bbs.vBB_.data() + m_BBH + 1, this->vBB_.data() + m_BBL);
	// 
	// return *this;

	// Copy assignment: copies sentinels and blocks in sentinel range
	m_BBL = bbs.m_BBL;
	m_BBH = bbs.m_BBH;
	
	// Copy blocks within sentinel range using STL
	if (m_BBL != EMPTY_ELEM && m_BBH != EMPTY_ELEM) {
		std::copy(bbs.vBB_.data() + m_BBL, bbs.vBB_.data() + m_BBH + 1, 
			      vBB_.data() + m_BBL);
	}
	
	return *this;
}

BBSentinel& BBSentinel::operator&=(const BitSet& bbn) {
	//CODIGO ORIGINAL
	// for(int i=m_BBL; i<=m_BBH; i++){
	// 	this->vBB_[i] &= bbn.block(i);
	// }
	// 
	// return  *this;

	// AND operation within sentinel range
	if (m_BBL == EMPTY_ELEM || m_BBH == EMPTY_ELEM) {
		return *this;
	}
	
	// Apply AND operation within sentinel range
	for (int i = m_BBL; i <= m_BBH; ++i) {
		vBB_[i] &= bbn.block(i);
	}
	
	return *this;
}

