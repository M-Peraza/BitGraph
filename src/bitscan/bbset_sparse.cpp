 /**
  * @file bbset_sparse.cpp
  * @brief implementations for the sparse BitSetSp class wrapper for sparse bitstrings (header bbset_sparse.h)
  * @author pss
  * @details created 10/02/2015?, @last_update 20/02/2025
  *
  * TODO refactoring and testing 15/02/2025 - follow the interface of the refactored BitSet
  **/

#include "bbset_sparse.h"
#include "utils/logger.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unordered_set>  // Added for flip() implementation 
//uncomment undef in bbconfig.h to avoid assertions
#include <cassert>
 
using namespace std;

using namespace bitgraph;


///////////////////////////////////////
int BitSetSp::DEFAULT_CAPACITY = 2;
///////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BitSetSp::BitSetSp(int nPop, bool is_popsize ){

	(is_popsize)? nBB_ = INDEX_1TO1(nPop) : nBB_ = nPop;
	vBB_.reserve(DEFAULT_CAPACITY);							
}

BitSetSp::BitSetSp(int nPop, const vint& lv):
	nBB_(INDEX_1TO1(nPop))
{
	try {
		
		//VBB_ initially empty.. with default capacity

		//sets bit conveniently
		for (auto& bit : lv) {

			//////////////////
			assert(bit >= 0 && bit < nPop);
			/////////////////

			//sets bits - adds pBlocks in place
			set_bit(bit);

		}

	}
	catch (...) {
		LOG_ERROR("Error during construction - BitSet::BitSetSp()");
		LOG_ERROR("exiting...");
		std::exit(-1);
	}
}

BitSetSp::BitSetSp(int nPop, std::initializer_list<int> lv) :
	nBB_(INDEX_1TO1(nPop))
{
	try {

		//VBB_ initially empty.. with default capacity

		//sets bit conveniently
		for (auto& bit : lv) {

			//////////////////
			assert(bit >= 0 && bit < nPop);
			/////////////////

			//sets bits - adds pBlocks in place
			set_bit(bit);

		}

	}
	catch (...) {
		LOG_ERROR("Error during construction - BitSet::BitSetSp()");
		LOG_ERROR("exiting...");
		std::exit(-1);
	}
}


void BitSetSp::reset(int size, bool is_popsize)
{
	try {
		(is_popsize) ? nBB_ = INDEX_1TO1(size) : nBB_ = size;
		decltype(vBB_)().swap(vBB_);
		vBB_.reserve(DEFAULT_CAPACITY);
	}
	catch (...) {
		LOG_ERROR("Error during reset - BitSet::reset");
		LOG_ERROR("exiting...");
		std::exit(-1);
	}
}

void BitSetSp::reset(int nPop, const vint& lv)
{

	try {
		
		nBB_ = INDEX_1TO1(nPop);
		decltype(vBB_)().swap(vBB_);
		vBB_.reserve(DEFAULT_CAPACITY);

		//sets bit conveniently
		for (auto& bit : lv) {

			//////////////////
			assert(bit >= 0 && bit < nPop);
			/////////////////

			//sets bits - adds pBlocks in place
			set_bit(bit);

		}
	}
	catch (...) {
		LOG_ERROR("Error during reset - BitSet::reset");
		LOG_ERROR("exiting...");
		std::exit(-1);
	}
}

void BitSetSp::init (int size, bool is_popsize){
	try {
		(is_popsize) ? nBB_ = INDEX_1TO1(size) : nBB_ = size;
		vBB_.clear();
		vBB_.reserve(DEFAULT_CAPACITY);
	}
	catch (...) {
		LOG_ERROR("Error during initialization - BitSet::init");
		LOG_ERROR("exiting...");
		std::exit(-1);
	}	
}

BitSetSp& BitSetSp::set_bit(int firstBit, int lastBit)
{
	auto bbl = WDIV(firstBit);					//block index firstBit
	auto bbh = WDIV(lastBit);					//block index lastBit

	/////////////////////////////////////////////////////////
	assert(firstBit >= 0 && firstBit <= lastBit && bbh<nBB_ );
	////////////////////////////////////////////////////////

	const auto sizeL = vBB_.size();				//stores the original size of *this since it will be enlarged
	auto offsetl = firstBit - WMUL(bbl);		//offset in the lower block
	auto offseth = lastBit - WMUL(bbh);			//offset in the upper block
	bool flag_sort = false;						//flag to sort the collection

	//finds position of block closest to blockID (equal or greater index)
	auto posTHIS = 0;							//block position *this	
	auto itbl = find_block(bbl, posTHIS);
			

	/////////////////////
	// ALGORITHM 	 
	// 1) special case: all existing blocks are outside the range
	// 2) special case: singleton range
	// 3) first block
	// 4) blocks in between analysis
	// 5) last block
	// 6) exit because last block has been reached
	// 7) exit because no more blocks in *this
	////////////////////
 
	//I. special case: all existing blocks are outside the range
	if (posTHIS == BBObject::noBit) {

		if (bbl == bbh) {
			vBB_.emplace_back(pBlock_t(bbl, bblock::MASK_1(offsetl, offseth)));
		}
		else {

			//add blocks trimming both ends 
			vBB_.emplace_back(pBlock_t(bbl, bblock::MASK_1_HIGH(offsetl)));
			vBB_.emplace_back(pBlock_t(bbh, bblock::MASK_1_LOW(offseth)));
			for (int i = bbl + 1; i < bbh; i++) {
				vBB_.emplace_back(pBlock_t(i, bitgraph::constants::ALL_ONES));
			}
		}
		
		return *this;
	}

	//II. special case - singleton range
	if (bbl == bbh) {

		if (vBB_[posTHIS].idx_ == bbl) {
			vBB_[posTHIS].bb_ |= bblock::MASK_1(offsetl, offseth);
		}
		else {
						
			if (vBB_[posTHIS].idx_ > bbl) {
						
				//insert block to avoid sorting - special case
				vBB_.insert(itbl, pBlock_t(bbl, bblock::MASK_1(offsetl, offseth)));
			}
			else {

				//places at the end - no sorting required
				vBB_.emplace_back(pBlock_t(bbl, bblock::MASK_1(offsetl, offseth)));
			}			
		}

		return *this;
	}

	//III. first block
	if (vBB_[posTHIS].idx_ == bbl) {
		vBB_[posTHIS].bb_ |= bblock::MASK_1_HIGH(offsetl);		
	}
	else {
		vBB_.emplace_back(pBlock_t(bbl, bblock::MASK_1_HIGH(offsetl)));
		if (vBB_[posTHIS].idx_ > bbl) {
			flag_sort = true;
		}
	}

	////////////////////////////////////////////
	//main loop
	auto block = bbl + 1;
	posTHIS++;
	while (posTHIS < sizeL && block < bbh) {

		if (vBB_[posTHIS].idx_ < block) {

			//add block
			vBB_.emplace_back(pBlock_t(block, bitgraph::constants::ALL_ONES));			
			
			posTHIS++;
		}
		else if (vBB_[posTHIS].idx_ > block) {
			
			//add block
			vBB_.emplace_back(pBlock_t(block, bitgraph::constants::ALL_ONES));
			flag_sort = true;
	
			block++;
		}
		else {

			//index match - overwrite
			vBB_[posTHIS].bb_ = bitgraph::constants::ALL_ONES;				
			
			posTHIS++;
			block++;
		}
		
	}//end while

	/////////////////////
	// Treatment depending on exit condition of MAIN LOPP
	 
	//I. exit because last block has been reached
	if (block == bbh) {
		if (vBB_[posTHIS].idx_ == bbh) {
			vBB_[posTHIS].bb_ |= bblock::MASK_1_LOW(offseth);
		}
		else {
			vBB_.emplace_back(pBlock_t(block, bblock::MASK_1_LOW(offseth)));
			if (vBB_[posTHIS].idx_ > block) {
				flag_sort = true;
			}
		}
	}

	//II. Exit because no more blocks in *this
	//	  Note: no sorting required in this case if blocks are appended at the end
	if (posTHIS == sizeL) {

		//[block, bbh[
		for (int i = block; i < bbh; i++) {
			vBB_.emplace_back(pBlock_t(i, bitgraph::constants::ALL_ONES));
		}
		
		//last block - bbh
		vBB_.emplace_back(pBlock_t(bbh, bblock::MASK_1_LOW(offseth)));
	}
	
	//keep the collection sorted if required
	if (flag_sort) {
		sort();
	}

	return *this;
}

BitSetSp& BitSetSp::reset_bit(int firstBit, int lastBit){

	auto bbh = WDIV(lastBit);
	auto bbl = WDIV(firstBit);

	///////////////////////////////// 
	assert(bbl >= 0 && bbh < nBB_);
	/////////////////////////////////

	vBB_.clear();
	
	//special case: same bitblock
	if(bbh == bbl){	
		vBB_.emplace_back( pBlock_t( bbl, bblock::MASK_1(firstBit - WMUL(bbl), lastBit - WMUL(bbh))));
	}
	else {

		//first and last blocks
		vBB_.emplace_back(pBlock_t(bbl, bblock::MASK_1_HIGH(firstBit - WMUL(bbl))));
		vBB_.emplace_back(pBlock_t(bbh, bblock::MASK_1_LOW(lastBit - WMUL(bbh))));

		//in-between blocks
		for (auto block = bbl + 1; block < bbh; ++block) {
			vBB_.emplace_back(pBlock_t(block, bitgraph::constants::ALL_ONES));
		}
	}

	return *this;
}

BitSetSp& BitSetSp::set_bit (const BitSetSp& rhs){

	///////////////////////////////////////////////
	assert(rhs.capacity() == this->capacity());
	//////////////////////////////////////////////

	//special case
	if (rhs.is_empty()) {
		return *this;
	}

	///////////////////////////////////	
	auto rIt = rhs.vBB_.cbegin();				//iterator to rhs	
	const auto sizeL = vBB_.size();			//stores the original size of *this since it will be enlarged
	auto posL = 0;								//position of bitblocks in  *this
	auto flag_sort = false;
	///////////////////////////////////
			
	//main loop
	while( posL != sizeL && rIt != rhs.vBB_.end()) {
		
		if (vBB_[posL].idx_ < rIt->idx_) {
			++posL;
		}
		else if (vBB_[posL].idx_ > rIt->idx_) {

			vBB_.push_back(*rIt);					
			flag_sort = true;
			++rIt;
		}
		else {
			/////////////////////////
			vBB_[posL].bb_ |= rIt->bb_;
			////////////////////////

			++posL,
			++rIt;
		}
	}
	//end while


	//I. exit condition
	if (posL == sizeL)			
	{
		//append rhs blocks at the end
		vBB_.insert(vBB_.end(), rIt, rhs.vBB_.end());
	}

	//always keep array sorted
	if (flag_sort) {
		//CODIGO ORIGINAL std::sort(vBB_.begin(), vBB_.end(), pBlock_less());
		std::sort(vBB_.begin(), vBB_.end(), 
		         [](const pBlock_t& a, const pBlock_t& b) { 
		             return a.idx_ < b.idx_; 
		         });
	}

	return *this;		
}

//DUPLICATE - FUNCTION DELETED IN HEADER
/*
// NUEVA IMPLEMENTACION
BitSetSp& BitSetSp::set_bit(int firstBit, int lastBit, const BitSetSp& rhs) {
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	assert(firstBit >= 0 && firstBit <= lastBit && lastBit < (this->capacity() << 6) && lastBit < (rhs.capacity() << 6));
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Convert bit range to block range
	int firstBlock = WDIV(firstBit);
	int lastBlock = WDIV(lastBit);
	
	auto posR = BBObject::noBit;
	auto itR = rhs.find_block(firstBlock, posR);
	
	// Special case - no bits to set in the range
	if (itR == rhs.vBB_.end()) {
		return *this;
	}
	
	// Process blocks in the range
	while (itR != rhs.vBB_.end() && itR->idx_ <= lastBlock) {
		
		BITBOARD bits_to_add;
		
		// Determine which bits to add based on block position
		if (itR->idx_ == firstBlock && itR->idx_ == lastBlock) {
			// Range is within a single block
			int offsetL = firstBit - WMUL(firstBlock);
			int offsetH = lastBit - WMUL(lastBlock);
			bits_to_add = itR->bb_ & bitgraph::bblock::MASK_1(offsetL, offsetH);
		}
		else if (itR->idx_ == firstBlock) {
			// First block - apply high mask
			int offsetL = firstBit - WMUL(firstBlock);
			bits_to_add = itR->bb_ & bitgraph::bblock::MASK_1_HIGH(offsetL);
		}
		else if (itR->idx_ == lastBlock) {
			// Last block - apply low mask
			int offsetH = lastBit - WMUL(lastBlock);
			bits_to_add = itR->bb_ & bitgraph::bblock::MASK_1_LOW(offsetH);
		}
		else {
			// Middle block - use full block
			bits_to_add = itR->bb_;
		}
		
		if (bits_to_add) {
			// Find or insert the block in *this
			auto pL = find_block_ext(itR->idx_);
			
			if (pL.first) {
				// Block exists - OR the bits
				pL.second->bb_ |= bits_to_add;
			}
		else {
				// Block doesn't exist - insert it
				if (pL.second == vBB_.end()) {
					// Add at the end
					vBB_.emplace_back(itR->idx_, bits_to_add);
				}
				else {
					// Insert in the middle
					vBB_.insert(pL.second, pBlock_t(itR->idx_, bits_to_add));
				}
			}
		}
		
		++itR;
	}
	
	return *this;
}
*/

BitSetSp&  BitSetSp::set_block (int firstBlock, int lastBlock, const BitSetSp& rhs){
			
	//special case - the full range
	if (lastBlock == -1) {
		return set_block(firstBlock, rhs);
	}

	//////////////////////////////////////////////////////////////////////////////////
	assert(firstBlock >= 0 && firstBlock <= lastBlock && lastBlock < rhs.capacity());
	//////////////////////////////////////////////////////////////////////////////////

	////////////////////////
	// Initialization

	//this		
	auto posL = BBObject::noBit;										//position of firstBlock in THIS or closest block
	auto itL = find_block(firstBlock, posL);							//iterator for THIS O(log n)
	(itL != vBB_.end()) ? posL = itL - vBB_.begin() : 1;				//sets posL to itL
	
	//stores the original size of THIS since it can be enlarged
	const auto sizeL = vBB_.size();									

	///////////
	//rhs
	auto pRlow = rhs.find_block_ext(firstBlock);						//O(log n)

	//flag to sort the collection
	bool flag_sort = false;

	////////////////////////////////////////////

	//I) special case - bitset rhs has no information in the range
	if( pRlow.second == rhs.vBB_.end()){ 
		return *this;
	}

	//II) special case  - this bitset has no information to mask in the range
	if (itL == vBB_.end())
	{
		//append rhs at the end
		for (; pRlow.second != rhs.vBB_.end() && pRlow.second->idx_ <= lastBlock; ++pRlow.second) {
			vBB_.emplace_back(*pRlow.second);
		}
		return *this;
	}

	//III) special case first block = last block and the block exists in rhs
	if (firstBlock == lastBlock && pRlow.second->idx_ == firstBlock) {
		if (itL->idx_ != firstBlock) {

			//block does not exist in THIS, add and sort
			vBB_.emplace_back(*pRlow.second);
			///////////////////////////////////

			//necessary because itL must have an index greatert than firstBlock
			sort();			
		}
		else {

			//block exists in THIS, overwrite
			vBB_[posL].bb_ |= pRlow.second->bb_;
		}			
		return *this;
	}	

	///////////////////
	//main loop
	while (posL < sizeL				&&
			vBB_[posL].idx_ <= lastBlock &&
			pRlow.second != rhs.vBB_.end()  &&
			pRlow.second->idx_ <= lastBlock		)
	{
	
		//update before either of the bitstrings has reached its end
		if(vBB_[posL].idx_ < pRlow.second->idx_)
		{
			++posL;
		}
		else if (vBB_[posL].idx_ > pRlow.second->idx_)
		{
			///////////////////////////////////
			vBB_.emplace_back(*pRlow.second);	
			//////////////////////////////////

			++pRlow.second;

			//sorting is necessary since the block added has less index
			flag_sort = true;
		}
		else {			//must have same indexes		

			//////////////////////////////////////
			vBB_[posL].bb_ |= pRlow.second->bb_;
			//////////////////////////////////////

			++posL;
			++pRlow.second;
		}	

	}//end while
		
	//exit conditions   
	if (posL == sizeL || vBB_[posL].idx_ > lastBlock)
	{
		//add remaining blocks in rhs to *this
		for (; pRlow.second != rhs.vBB_.end() && pRlow.second->idx_ <= lastBlock; ++pRlow.second) {
			vBB_.emplace_back(*pRlow.second);
		}	

		if (vBB_[posL].idx_ > lastBlock) {
			flag_sort = true;
		}	
		
	}

	//sort if required
	if (flag_sort) {
		// CODIGO ORIGINAL std::sort(vBB_.begin(), vBB_.end(), pBlock_less());
		std::sort(vBB_.begin(), vBB_.end(), 
						[](const pBlock_t& a, const pBlock_t& b) { 
							return a.idx_ < b.idx_; 
						});
	}
	
	return *this;		
}

//DUPLICATE - FUNCTION DELETED IN HEADER
/*
// NUEVA IMPLEMENTACIO
BitSetSp& BitSetSp::AND_block(int firstBlock, int lastBlock, const BitSetSp& rhs) {
	
	///////////////////////////////////////////////////////////////////////////////////
	assert(firstBlock >= 0 && firstBlock <= lastBlock && lastBlock < rhs.capacity());
	///////////////////////////////////////////////////////////////////////////////////
	
	auto posL = BBObject::noBit;
	auto posR = BBObject::noBit;
	auto itL = this->find_block(firstBlock, posL);
	auto itR = rhs.find_block(firstBlock, posR);
	
	// Process blocks in the range
	while (itL != vBB_.end() && itL->idx_ <= lastBlock) {
		
		if (itR != rhs.vBB_.end() && itR->idx_ == itL->idx_ && itR->idx_ <= lastBlock) {
			// Both blocks exist AND them
			itL->bb_ &= itR->bb_;
			++itL;
			++itR;
		}
		else if (itR == rhs.vBB_.end() || itR->idx_ > itL->idx_ || itR->idx_ > lastBlock) {
			// Block exists in *this but not in rhs - set to zero
			itL->bb_ = 0;
			++itL;
		}
		else {
			// Block exists in rhs but not in *this advance rhs
			++itR;
		}
	}
	
	return *this;
}
*/

// NUEVA IMPLEMENTACION
BitSetSp& BitSetSp::flip() {
	// Flipping a sparse bitset requires converting to dense representation
	// This is expensive but necessary for NOT operation
	
	// Lo necesitamos para crear un set de indices de blocks para buscar(me ha dado mejor resultado que otras implementaciones)
	std::unordered_set<int> existing_blocks;
	for (const auto& block : vBB_) {
		existing_blocks.insert(block.idx_);
	}
	
	// Create new vector with flipped blocks
	vPB new_blocks;
	new_blocks.reserve(nBB_);
	
	// Process all possible blocks
	for (int i = 0; i < nBB_; ++i) {
		auto it = existing_blocks.find(i);
		
		if (it != existing_blocks.end()) {
			// Block exists - find it and flip its bits
			auto block_it = std::find_if(vBB_.begin(), vBB_.end(),
			                             [i](const pBlock_t& b) { return b.idx_ == i; });
			if (block_it != vBB_.end()) {
				BITBOARD flipped = ~block_it->bb_;
				if (flipped != 0) {  // Only store non-zero blocks
					new_blocks.emplace_back(i, flipped);
				}
			}
		}
		else {
			// Block doesn't exist - create it with all bits set
			new_blocks.emplace_back(i, bitgraph::constants::ALL_ONES);
		}
	}
	
	// Replace old blocks with new flipped blocks
	vBB_ = std::move(new_blocks);
	
	return *this;
}

int BitSetSp::clear_bit (int low, int high){
	
	int bbl = EMPTY_ELEM, bbh = EMPTY_ELEM; 
	pair<bool, BitSetSp::vPB_it> pl;
	pair<bool, BitSetSp::vPB_it> ph;

////////////////////////
//special cases
	if(high == EMPTY_ELEM && low == EMPTY_ELEM){
		vBB_.clear();
		return 0;
	}

	if(high == EMPTY_ELEM){
		bbl=WDIV(low);
		pl = find_block_ext(bbl);
		if(pl.second==vBB_.end()) return 0;

		if(pl.first){	//lower block exists
			pl.second->bb_&=Tables::mask_low[low-WMUL(bbl)];
			++pl.second;
		}

		//remaining
		vBB_.erase(pl.second, vBB_.end());
		return 0;
	}else if(low == EMPTY_ELEM){
		bbh=WDIV(high); 
		ph=find_block_ext(bbh);
		if(ph.first){	//upper block exists
			ph.second->bb_&=Tables::mask_high[high-WMUL(bbh)];
		}

		//remaining
		vBB_.erase(vBB_.begin(), ph.second);
		return 0;
	}

////////////////
// general cases

	//check consistency
	if(low>high){
		cerr<<"Error in set bit in range"<<endl;
		return -1;
	}
		

	bbl=WDIV(low);
	bbh=WDIV(high); 
	pl=find_block_ext(bbl);
	ph=find_block_ext(bbh);	


	//tratamiento
	if(pl.second!=vBB_.end()){
		//updates lower bitblock
		if(pl.first){	//lower block exists
			if(bbh==bbl){		//case update in the same bitblock
				BITBOARD bb_low=pl.second->bb_ & Tables::mask_high[high-WMUL(bbh)];
				BITBOARD bb_high=pl.second->bb_ &Tables::mask_low[low-WMUL(bbl)];
				pl.second->bb_=bb_low | bb_high;
				return 0;
			}

			//update lower block
			pl.second->bb_&=Tables::mask_low[low-WMUL(bbl)];
			++pl.second;
		}

		//updates upper bitblock
		if(ph.first){	//lower block exists
			if(bbh==bbl){		//case update in the same bitblock
				BITBOARD bb_low=pl.second->bb_ & Tables::mask_high[high-WMUL(bbh)];
				BITBOARD bb_high=pl.second->bb_ &Tables::mask_low[low-WMUL(bbl)];
				pl.second->bb_=bb_low | bb_high;
				return 0;
			}

			//update lower block
			ph.second->bb_&=Tables::mask_high[high-WMUL(bbh)];
		}

		//remaining
		vBB_.erase(pl.second, ph.second);
	}


return 0;
}

//DUPLICATE - SECOND OCCURRENCE OF SAME FUNCTION
/*
// NUEVA IMPLEMENTACIO
BitSetSp& BitSetSp::AND_block(int firstBlock, int lastBlock, const BitSetSp& rhs) {
	
	///////////////////////////////////////////////////////////////////////////////////
	assert(firstBlock >= 0 && firstBlock <= lastBlock && lastBlock < rhs.capacity());
	///////////////////////////////////////////////////////////////////////////////////
	
	auto posL = BBObject::noBit;
	auto posR = BBObject::noBit;
	auto itL = this->find_block(firstBlock, posL);
	auto itR = rhs.find_block(firstBlock, posR);
	
	// Process blocks in the range
	while (itL != vBB_.end() && itL->idx_ <= lastBlock) {
		
		if (itR != rhs.vBB_.end() && itR->idx_ == itL->idx_ && itR->idx_ <= lastBlock) {
			// Both blocks exist - AND them
			itL->bb_ &= itR->bb_;
			++itL;
			++itR;
		}
		else if (itR == rhs.vBB_.end() || itR->idx_ > itL->idx_ || itR->idx_ > lastBlock) {
			// Block exists in *this but not in rhs - set to zero
			itL->bb_ = 0;
			++itL;
		}
		else {
			// Block exists in rhs but not in *this - advance rhs
			++itR;
		}
	}
	
	return *this;
}
*/

//DUPLICATE - SECOND OCCURRENCE OF SAME FUNCTION
/*
// NUEVA IMPLEMENTACION
BitSetSp& BitSetSp::flip() {
	// Flipping a sparse bitset requires converting to dense representation. This is expensive but necessary for NOT operation
	
	// Lo necesitamos para crear un set de indices de blocks para buscar(me ha dado mejor resultado que otras implementaciones)
	std::unordered_set<int> existing_blocks;
	for (const auto& block : vBB_) {
		existing_blocks.insert(block.idx_);
	}
	
	// Create new vector with flipped blocks
	vPB new_blocks;
	new_blocks.reserve(nBB_);
	
	// Process all possible blocks
	for (int i = 0; i < nBB_; ++i) {
		auto it = existing_blocks.find(i);
		
		if (it != existing_blocks.end()) {
			// Block exists - find it and flip its bits
			auto block_it = std::find_if(vBB_.begin(), vBB_.end(),
			                             [i](const pBlock_t& b) { return b.idx_ == i; });
			if (block_it != vBB_.end()) {
				BITBOARD flipped = ~block_it->bb_;
				if (flipped != 0) {  // Only store non-zero blocks
					new_blocks.emplace_back(i, flipped);
				}
			}
		}
		else {
			// Block doesn't exist - create it with all bits set
			new_blocks.emplace_back(i, bitgraph::constants::ALL_ONES);
		}
	}
	
	// Replace old blocks with new flipped blocks
	vBB_ = std::move(new_blocks);
	
	return *this;
}
*/

//DUPLICATE - SECOND OCCURRENCE OF SAME FUNCTION
/*
int BitSetSp::clear_bit (int low, int high){
	
	int bbl = EMPTY_ELEM, bbh = EMPTY_ELEM; 
	pair<bool, BitSetSp::vPB_it> pl;
	pair<bool, BitSetSp::vPB_it> ph;

////////////////////////
//special cases
	if(high == EMPTY_ELEM && low == EMPTY_ELEM){
		vBB_.clear();
		return 0;
	}

	if(high == EMPTY_ELEM){
		bbl=WDIV(low);
		pl = find_block_ext(bbl);
		if(pl.second==vBB_.end()) return 0;

		if(pl.first){	//lower block exists
			pl.second->bb_&=Tables::mask_low[low-WMUL(bbl)];
			++pl.second;
		}

		//remaining
		vBB_.erase(pl.second, vBB_.end());
		return 0;
	}else if(low == EMPTY_ELEM){
		bbh=WDIV(high); 
		ph=find_block_ext(bbh);
		if(ph.first){	//upper block exists
			ph.second->bb_&=Tables::mask_high[high-WMUL(bbh)];
		}

		//remaining
		vBB_.erase(vBB_.begin(), ph.second);
		return 0;
	}

////////////////
// general cases

	//check consistency
	if(low>high){
		cerr<<"Error in set bit in range"<<endl;
		return -1;
	}
		

	bbl=WDIV(low);
	bbh=WDIV(high); 
	pl=find_block_ext(bbl);
	ph=find_block_ext(bbh);	


	//tratamiento
	if(pl.second!=vBB_.end()){
		//updates lower bitblock
		if(pl.first){	//lower block exists
			if(bbh==bbl){		//case update in the same bitblock
				BITBOARD bb_low=pl.second->bb_ & Tables::mask_high[high-WMUL(bbh)];
				BITBOARD bb_high=pl.second->bb_ &Tables::mask_low[low-WMUL(bbl)];
				pl.second->bb_=bb_low | bb_high;
				return 0;
			}

			//update lower block
			pl.second->bb_&=Tables::mask_low[low-WMUL(bbl)];
			++pl.second;
		}

		//updates upper bitblock
		if(ph.first){	//lower block exists
			if(bbh==bbl){		//case update in the same bitblock
				BITBOARD bb_low=pl.second->bb_ & Tables::mask_high[high-WMUL(bbh)];
				BITBOARD bb_high=pl.second->bb_ &Tables::mask_low[low-WMUL(bbl)];
				pl.second->bb_=bb_low | bb_high;
				return 0;
			}

			//update lower block
			ph.second->bb_&=Tables::mask_high[high-WMUL(bbh)];
		}

		//remaining
		vBB_.erase(pl.second, ph.second);
	}

	
return 0;
}
*/

BitSetSp&  BitSetSp::erase_bit (const BitSetSp& rhs ){

	auto itL = vBB_.begin();		//iterator to *this
	auto itR = rhs.vBB_.cbegin();	//iterator to rhs
	
	
	while (itL != vBB_.end() && itR != rhs.vBB_.end()) {
				
		//update before either of the bitstrings has reached its end
		if (itL->idx_ < itR->idx_)
		{
			++itL;
		}
		else if (itL->idx_ > itR->idx_)
		{
			++itR;
		}
		else{

			//equal indexes - erase bits
			itL->bb_ &= ~itR->bb_;
			
			++itL;
			++itR;
		}
	}

	return *this;
}

BitSetSp& BitSetSp::operator &= (const BitSetSp& rhs){
	
	auto itL = vBB_.begin();		//iterator to *this	
	auto itR = rhs.vBB_.cbegin();	//iterator to rhs	

	while( itL != vBB_.end() && itR != rhs.vBB_.end()  ){
	
		//update before either of the bitstrings has reached its end
		if(itL->idx_ < itR->idx_)
		{
			///////////////////////
			itL->bb_ = 0;
			///////////////////////

			++itL;

		}else if (itL->idx_ > itR->idx_ )
		{
			++itR;
		}else
		{
			//equal indexes

			//////////////////////////////////////
			itL->bb_ &= itR->bb_;
			/////////////////////////////////////
			
			++itL;
			++itR;
		}
			

	}//end while

	//if loop terminates because rhs has reached its end then
	//delete remaining blocks in THIS 
	if (itR == rhs.vBB_.end()) {
		
		// CODIGO ORIGINAL
		// for (; itL != vBB_.end(); ++itL) {

		// 	////////////////////////////
		// 	itL->bb_ = bitgraph::constants::ALL_ZEROS;
		// 	///////////////////////////
		// }
		std::for_each(itL, vBB_.end(), 
		             [](pBlock_t& block) { 
		                 block.bb_ = bitgraph::constants::ALL_ZEROS; 
		             });
	}


	return *this;
}

BitSetSp& BitSetSp::operator |= (const BitSetSp& rhs){

	auto posL = 0;					//position *this
	auto itR = rhs.cbegin();		//iterator to rhs
	auto sizeL = vBB_.size();		//stores the original size of *this since it will be modified
	bool flag_sort = false;			//flag to sort the collection
		
	//OR before all the blocks of one of the bitsets have been examined
	while (posL < sizeL && itR != rhs.vBB_.end()) {
			
		if(vBB_[posL].idx_ < itR->idx_)
		{
			posL++;
		}else if(vBB_[posL].idx_ > itR->idx_ )
		{
			//////////////////////////////////
			vBB_.emplace_back(*itR);
			///////////////////////////////////
			flag_sort = true;
			itR++;
		}else{

			//equal indexes

			///////////////////////////////////////////
			vBB_[posL].bb_ |= itR->bb_;
			///////////////////////////////////////////

			posL++;
			itR++;
		}
	}

	//rhs unfinished with index below the last block of *this
	if (posL == sizeL) {

		for (; itR != rhs.vBB_.end(); ++itR) {

			//////////////////////////////////
			vBB_.emplace_back(*itR);
			///////////////////////////////////
		}
	}

	//keep the collection sorted
	if (flag_sort) {
		sort();
	}	
	
	return *this;
}

BitSetSp& BitSetSp::operator ^= (const BitSetSp& rhs) {

	auto posL = 0;					//position *this	
	auto itR = rhs.cbegin();		//iterator to rhs
	auto sizeL = vBB_.size();		//stores the original size of *this since it will be modified
	bool flag_sort = false;			//flag to sort the collection
	
	//XOR before all the blocks of one of the bitsets have been examined
	while ((posL < sizeL) && (itR !=  rhs.vBB_.end())) {

		if (vBB_[posL].idx_ < itR->idx_)
		{
			posL++;
		}
		else if (vBB_[posL].idx_ > itR->idx_)
		{
			//////////////////////////////////
			vBB_.emplace_back(*itR);
			///////////////////////////////////
			flag_sort = true;
			itR++;
		}
		else {

			//equal indexes

			/////////////////////////////
			vBB_[posL].bb_ ^= itR->bb_;
			/////////////////////////////

			posL++;
			itR++;
		}
	}

	//rhs unfinished with index below the last block of *this
	if (posL == sizeL) {

		for (; itR != rhs.vBB_.end(); ++itR) {

			//////////////////////////////////
			vBB_.emplace_back(*itR);
			///////////////////////////////////
		}
	}

	//keep the collection sorted
	if (flag_sort) {
		sort();
	}	

	return *this;
}


BITBOARD BitSetSp::find_block (int blockID) const{

	////////////////////////////////////////////////////////////////////////////////////////////
	// CODIGO ORIGINAL
	// auto it = lower_bound(vBB_.cbegin(), vBB_.cend(), pBlock_t(blockID), pBlock_less());
	////////////////////////////////////////////////////////////////////////////////////////////
	auto it = std::lower_bound(vBB_.cbegin(), vBB_.cend(), pBlock_t(blockID),
	                           [](const pBlock_t& a, const pBlock_t& b) { 
	                               return a.idx_ < b.idx_; 
	                           });

	if(it != vBB_.end() && it->idx_ == blockID){
		return it->bb_;
	}
	else {
		return BBObject::noBit;
	}
}


std::pair<bool, int>
BitSetSp::find_block_pos (int blockID) const{

	std::pair<bool, int> res(false, EMPTY_ELEM);

	////////////////////////////////////////////////////////////////////////////////////////////
	// CODIGO ORIGINAL
	// auto it = lower_bound(vBB_.cbegin(), vBB_.cend(), pBlock_t(blockID), pBlock_less());
	////////////////////////////////////////////////////////////////////////////////////////////

	auto it = std::lower_bound(vBB_.cbegin(), vBB_.cend(), pBlock_t(blockID),
	                           [](const pBlock_t& a, const pBlock_t& b) { 
	                               return a.idx_ < b.idx_; 
	                           });

	if(it != vBB_.end()){
		res.second = it - vBB_.begin();
		if(it->idx_ == blockID){
			res.first = true;
		}
	}

	return res;
}



////////////////////////////
////
//// I/O FILES
////
////////////////////////////

ostream& BitSetSp::print (std::ostream& o, bool show_pc, bool endl ) const  {
	
	/////////////
	o << '[';
	/////////////
		
	int nBit = BBObject::noBit;
	while( (nBit = next_bit(nBit)) != BBObject::noBit) {
		o << nBit << ' ';
	}

	if(show_pc){
		int pc = popcn64();
		if (pc) {
			o << '(' << popcn64() << ')';
		}
	}
	
	/////////////
	o << ']';
	//////////////

	//add eol if requires
	if (endl) {
		o << std::endl;
	}

	return o;
}

string BitSetSp::to_string ()  const{

	ostringstream sstr;

	/////////////
	sstr << '[';
	/////////////
		
	int nBit = BBObject::noBit;
	while( (nBit = next_bit(nBit)) != BBObject::noBit) {
		sstr << nBit << ' ';
	}

	sstr << '(' << size() << ')';

	///////////////
	sstr << ']';
	///////////////

	return sstr.str();
}

void BitSetSp::to_vector (std::vector<int>& lb)const{

	lb.clear();

	int nBit = BBObject::noBit;
	while((nBit = next_bit(nBit)) != BBObject::noBit ){
		lb.push_back(nBit);
	}
}

BitSetSp::operator vint() const
{
	vint lb;
	to_vector(lb);
	return lb;
}

///////////////////////////////
// 
// DEPRECATED -  CODE
//
///////////////////////////////

///////////////////
// Bit scanning with cached BitSetSp::block_scanned
// (UNSAFE)


//int BitSetSp::prev_bit(int lastBit) {
//
//	//special case - first bitscan
//	if (lastBit == BBObject::noBit) {
//
//		//finds msb AND caches next block to scan
//		return msb(BitSetSp::block_scanned);
//	}
//
//	//if block of firstBit exists it MUST be  BitSetSp::block_scanned - compute lsb
//	int npos = bblock::msb(vBB_[BitSetSp::block_scanned].bb_ & Tables::mask_low[lastBit - WMUL(BitSetSp::block_scanned)]);
//	if (npos != BBObject::noBit) {
//		return (WMUL(BitSetSp::block_scanned) + npos);
//	}
//
//	//BitSetSp::block_scanned does not exist - finds closest block to BitSetSp::block_scanned
//	for (int i = BitSetSp::block_scanned - 1; i >= 0; --i) {  //new bitblock
//		if (vBB_[i].bb_) {
//			BitSetSp::block_scanned = i;
//			return bblock::msb(vBB_[i].bb_) + WMUL(vBB_[i].idx_);
//		}
//	}
//
//	return BBObject::noBit;
//}
//
//int BitSetSp::next_bit(int firstBit) {
//
//	//special case - first bitscan
//	if (firstBit == BBObject::noBit) {
//
//		//finds lsb AND caches next block to scan
//		return lsb(BitSetSp::block_scanned);
//	}
//
//	//if block of firstBit exists it MUST be  BitSetSp::block_scanned - compute lsb
//	int npos = bblock::lsb(vBB_[BitSetSp::block_scanned].bb_ & Tables::mask_high[firstBit - WMUL(BitSetSp::block_scanned)]);
//	if (npos != BBObject::noBit) {
//		return (npos + WMUL(BitSetSp::block_scanned));
//	}
//
//	//bbL does not exist - finds closest block to bbL
//	for (auto i = BitSetSp::block_scanned + 1; i < vBB_.size(); ++i) {
//		//new bitblock
//		if (vBB_[i].bb_) {
//
//			//update cached block
//			BitSetSp::block_scanned = i;
//			return bblock::lsb64_de_Bruijn(vBB_[i].bb_) + WMUL(vBB_[i].idx_);
//		}
//	}
//
//	return BBObject::noBit;
//}

ostream& BitSetSp::pBlock_t::print(ostream& o, bool eofl) const
{

	o << "[";
	o << idx_ << " : "; 

	////////////////////////////////
	bblock::print(bb_, o, false);
	/////////////////////////////
	
	o << "]";
	if (eofl) {	o << std::endl; }
	return o;

}
