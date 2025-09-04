/*
 *info_clq.cpp: implementation of result logs of clique and clique-based algorithms
 *@date 12/12/2012
 *@last_update 13/23/2024
 *@dev pss
 */

#include "info_clq.h"
#include "utils/logger.h"
#include <iostream>

using namespace std;

namespace com {

	///////////////////////////
	//
	// DERIVED CLASS

	template<class W_t>
	std::ostream& infoCLQ<W_t>::printParams(std::ostream& o ) const  {

		o << "*****************************\n";
		o << "DATA:" << data_.name.c_str() << "\t N:" << data_.N << "\t M:" << data_.M << "\t D:" << 2 * data_.M / (float)((data_.N - 1) * data_.N) << endl;
		if (K_) {
			o << "K-CLIQUE SEARCH:" << K_ << endl;
		}
		o << "TIME_LIMIT:" << data_.TIME_OUT << endl;
		o << "TIME_LIMIT_HEUR:" << data_.TIME_OUT_HEUR << endl;
		o << "ALG:" << algSearch_ << endl;
		o << "SORTING:" << algSort_ << endl;
		o << "HEUR:" << algHeur_ << endl;
		o << "*****************************" << endl;

		return o;

	}

	template<class W_t>
	std::ostream& infoCLQ<W_t>::printSummary(std::ostream& o) const
	{
		o << "*****************************" << endl;
		o << "w:" << lb_ << "\tt_par: " << timeParse_ << "\tt_pp: " << timePreproc_ << "\tt_search: " << timeSearch_ << "\t#steps: " << nSteps_ << endl;
		o << "*****************************" << endl;
		return o;
	}

	template<class W_t>
	std::ostream& infoCLQ<W_t>::printReport(std::ostream& o, bool is_endl) const
	{
		o << data_.name.c_str() << "\t" << data_.N << "\t" << data_.M << "\t";
		if (K_) {
			o << K_ << '\t';
		}
		o	<< data_.TIME_OUT << "\t" << data_.TIME_OUT_HEUR << "\t"
			<< algSearch_ << "\t"
			<< algSort_ << "\t" << algSortReal_ << "\t" << isAlgSortRealDeg_ << "\t"
			<< algHeur_ << "\t"
			<< branchingFactorRoot_ << "\t" 
			<< lbRootBasicHeur_ << "\t" << lbRootStrongHeur_ << "\t" 
			<< lb_ << "\t" << ub_ << "\t" << ubRoot_ << "\t"
			<< timeParse_ << "\t" << timePreproc_ << "\t" << timeIncumbent_ << "\t" << timeSearch_ << "\t" << isTimeOut_ << "\t"
			<< nSteps_ << "\t";
		/*<< nLastIsetCalls_ << "\t" << nsLastIsetCalls_ << "\t" << nLastIsetPreFilterCalls_ << "\t" << nsLastIsetPreFilterCalls_ << "\t"
		<< nCurrIsetCalls_ << "\t" << nsCurrIsetCalls_ << "\t"
		<< nVertexCalls_ << "\t" << nsVertexCalls_ << "\t" << nUBpartCalls_ << "\t" << nsUBpartCalls_ << "\t" << K_
		<< std::endl;*/

		if (is_endl) {
			o << std::endl;
		}

		return o;
	}

	template<class W_t>
	void infoCLQ<W_t>::clearPreprocInfo()
	{
		lbRootBasicHeur_ = 0.0; 
		lbRootStrongHeur_ = 0.0;
		branchingFactorRoot_ = 0;
		algSortReal_ = -1;
		isAlgSortRealDeg_ = false;

		clearTimer(com::infoBase::phase_t::PREPROC);
		sol_.clear();									//candidate solution found during preprocessing
	}

	template<class W_t>
	void infoCLQ<W_t>::clearSearchInfo()
	{
		lb_ = 0;
		ub_ = 0;
		nSteps_ = 0;
		isTimeOut_ = false;
		sol_.clear();

		clearTimer(com::infoBase::phase_t::SEARCH);
		clearTimer(com::infoBase::phase_t::LAST_INCUMBENT);
	}

	template<class W_t>
	void infoCLQ<W_t>::clear(bool lazy)
	{
		clearPreprocInfo();
		clearSearchInfo();

		//additional config params
		if (!lazy) { K_ = 0; }

		::com::infoBase::clear(lazy);

		
	}



	/////////////////
	// infoCliSAT


	void infoCliSAT::clearSearchInfo()
	{
		infoCLQ<int>::clearSearchInfo();

		nLastIsetCalls_ = 0;
		nUBpartCalls_ = 0;
		nCurrIsetCalls_ = 0;
		nVertexCalls_ = 0;
		nLastIsetPreFilterCalls_ = 0;
		nsLastIsetCalls_ = 0;
		nsUBpartCalls_ = 0;
		nsCurrIsetCalls_ = 0;
		nsVertexCalls_ = 0;
		nsLastIsetPreFilterCalls_ = 0;
	}


	std::ostream& infoCliSAT::printReport(std::ostream& o, bool is_endl) const
	{
		infoCLQ<int>::printReport(o, false);

		o << nLastIsetCalls_ << "\t" << nsLastIsetCalls_ << "\t" << nLastIsetPreFilterCalls_ << "\t" << nsLastIsetPreFilterCalls_ << "\t"
			<< nCurrIsetCalls_ << "\t" << nsCurrIsetCalls_ << "\t"
			<< nVertexCalls_ << "\t" << nsVertexCalls_ << "\t" << nUBpartCalls_ << "\t" << nsUBpartCalls_ << "\t" << K_;

		if (is_endl) {
			o << std::endl;
		}


		return o;
	}
}//namespace com	

////////////////////////////////////////////
//list of valid types for infoCLQ to allow generic code in *.cpp files 

template struct  com::infoCLQ<int>;
template struct  com::infoCLQ<double>;


















