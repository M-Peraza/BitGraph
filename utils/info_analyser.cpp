/*
 * @file info_analyser.cpp
 * @brief implementation of the InfoAnalyser class (tests_analyser.h) to manage benchmarking of graph algorithms
 * @date 2013
 * @last_update 20/01/2025
 * @author pss
 */

#include "common.h"
#include "logger.h"
#include "info_analyser.h"
#include <iomanip>
#include <math.h>
#include <string>

#include "utils/info/info_clq.h"

using namespace std;


template<class AlgInfo_t>
std::ostream& operator << (std::ostream& o, const InfoAnalyser<AlgInfo_t>& t)
{
	try {
		//general information
		if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::NAME) {					//assumes the same instance for all tests
			//o<<left<<setw(30)<<t.arrayOfTests_[0][0].get_name()<<" ";
			o << left << setw(30) << t.arrayOfTests_[0][0].name();
		}

		if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::SIZE) {
			//o<<setw(5)<<right<<setprecision(0)<<t.arrayOfTests_[0][0].get_d1()<<" ";  
			o << right << "\t" << t.arrayOfTests_[0][0].number_of_vertices();
		}

		if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::EDGES) {
			//o<<setw(5)<<right<<setprecision(0)<<t.arrayOfTests_[0][0].get_d2()<<" ";  
			o << right << "\t" << t.arrayOfTests_[0][0].number_of_edges();
		}

		if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::TIMEOUT) {
			o << right << setw(10) << "\t" << (int)t.arrayOfTests_[0][0].time_out();
		}

		if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::ALG) {
			o << right << "\t" << t.arrayOfTests_[0][0].search_algorithm();
		}

		if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::SORT) {
			//TODO@decode SORT_TYPE!
			//o << setw(5) << right << setprecision(0) << t.arrayOfTests_[0][0].get_d3() << " ";
			o << right << "\t" << t.arrayOfTests_[0][0].sorting_algorithm();
		}

		//TODO - ADD TIMEOUT (check this comment 20/01/2025)

		//information common to all tests
		o.setf(ios::fixed);
		for (auto i = 0; i < t.nAlg_; ++i) {

			if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::LOWER_BOUND) {
				//o << setw(4) << right << setprecision(2) << t.arrayOfAvLB[i] << " ";
				o << right << setw(7) << setprecision(2) << "\t" << t.arrayOfAvLB[i];

			}

			if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::SOL) {
				//o << setw(5) << right << setprecision(2) << t.arrayOfAvSol[i] << " ";
				o << right << setw(7) << setprecision(2) << "\t" << t.arrayOfAvSol[i];
			}

			//if(t.print_mode_& InfoAnalyser::STDDEV_SOL){
			//	o<<setw(5)<<right<<setprecision(2)<<t.arrayOfSdSol[i]<<" ";  
			//}

			//if(t.print_mode_ & InfoAnalyser::MAX_SOL){
			//	//o<<setw(5)<<right<<setprecision(0)<<t.arrayOfMaxSol[i]<<" ";  
			//	o << setprecision(0) << "\t" << t.arrayOfMaxSol[i];				
			//}


			if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::STEPS) {
				//o<<setw(15)<<right<<setprecision(0)<<t.arrayOfAvSteps[i]<<" ";  
				o << right << setw(10) << setprecision(0) << "\t" << t.arrayOfAvSteps[i];

			}

			if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::TIME) {
				//o<<setw(12)<<right<<setprecision(3)<<t.arrayOfAvTimes[i]<<" "; 
				o << right << setw(7) << setprecision(3) << "\t" << t.arrayOfAvTimes[i];

			}

			if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::TIMEPRE) {
				//o<<setw(12)<<right<<setprecision(3)<<t.arrayOfAvTimes[i]<<" "; 
				o << right << setw(7) << setprecision(3) << "\t" << t.arrayOfAvPreProcTimes[i];

			}

			if (t.print_mode_ & InfoAnalyser<AlgInfo_t>::NFAIL) {
				//o << setw(5) << t.arrayOfFails[i] << " ";
				o << right << "\t" << t.arrayOfFails[i];
			}

			/*if(t.print_mode_ & InfoAnalyser<AlgInfo_t>::NCONT){
				for(auto j = 0; j < t.arrayOfCounters[i].size(); ++j){
					o << right << setw(10) << setprecision(4) << t.arrayOfCounters[i][j]<<" ";
				}
			}	*/


			//separator for diffent algs of same instance
			if (i < (t.nAlg_ - 1)) {
				o << "| ";
			}
		}
		o << endl;

	}
	catch (exception e) {
		LOG_ERROR("Error when printing data", e.what(), "- Test_Analyser::operator << ");
		o << "Error when printing data" << e.what() << "- Test_Analyser::operator<< " << endl;
	}

	return o;
}

template<class AlgInfo_t>
void InfoAnalyser< AlgInfo_t>::clear()
{
	arrayOfTests_.clear();				//[nRep][nAlg]
	arrayOfAvTimes.clear();
	arrayOfAvPreProcTimes.clear();
	arrayOfAvSol.clear();				//[nAlg]	
	arrayOfFails.clear();
	arrayOfAvLB.clear();
	arrayOfAvSteps.clear();
	arrayOfCounters.clear();			//[nAlg][nCounters] - currently not used
	arrayOfMaxSol.clear();
	nAlg_ = 0;
	nRep_ = 0;
	print_mode_ = DEFAULT_PRINT_MODE;

	//arrayOfSdTime.clear();			//std dev reports - currently not implemented		
	//arrayOfSdSol.clear();
}

template<class AlgInfo_t>
void InfoAnalyser<AlgInfo_t>::add_test(bool isNewRep, AlgInfo_t res){
			
	if(isNewRep || arrayOfTests_.empty() ){					//new repetition/test

		vInfo_t v;
		v.push_back(std::move(res));
		arrayOfTests_.push_back(std::move(v));		

	}else{

		arrayOfTests_.back().push_back(std::move(res));		//new result in a current repetition/test

	}
}

template<class AlgInfo_t>
int InfoAnalyser<AlgInfo_t>::analyser(info_t* info){

	//updates nRep_, nAlg_ values / checks consistency
	if(make_consistent() == -1){
		if (nRep_ <= 0) {
			LOGG_ERROR("Error in number of repetitions: ", nRep_, "InfoAnalyser<AlgInfo_t>::analyser");
		}
		if(nAlg_ <= 0){
			LOGG_ERROR("Error in number of algorithms: ", nAlg_, "InfoAnalyser<AlgInfo_t>::analyser");
		}
		return -1;
	}

	///////////////////////////
	//analysis of the results
	double avSol, avTimes, avPreProcTimes, avSteps, avLB;	
	double maxSol = 0;
	int nFails = 0; 
	
	//main outer loop over algorithms
	//vector<double> avnCounters;			
	for(auto j = 0; j < nAlg_; ++j){

		////////////
		//initializes context for current algorithm j
		avSol = 0.0;
		avTimes = 0.0; 
		avPreProcTimes = 0.0;
		avSteps = 0.0;
		avLB = 0.0;
		maxSol = 0.0;
		nFails = 0;
			
		//determines the maximum number of counters of any algorithm and repetition
		//usint nMaxCounters = 0;						
		//for(int rep = 0; rep < nRep_; ++rep)
		//{
		//	if (nMaxCounters < arrayOfTests_[rep].at(j).number_of_counters()) {
		//		nMaxCounters = arrayOfTests_[rep].at(j).number_of_counters();
		//	}
		//}

		////allocates nMaxCounters with initial value 0.0
		//avnCounters.assign(nMaxCounters,0.0);	

		////////////////////////////////
		//inner loop repetitions for algorithm j

		for(auto rep = 0; rep < nRep_; ++rep){
			auto res = arrayOfTests_[rep].at(j);				 

			//extracts counter info - always reported independent of TIMEOUT
			/*for (auto i = 0; i < res.number_of_counters(); i++) {
				avnCounters[i] += res.get_counters()[i];
			}*/
						
			if(!res.is_time_out())
			{	
				//no time_out
				double sol = res.ub();
				avSol += sol;
				if (sol > maxSol) {
					maxSol = sol;
				}
				avTimes += res.search_time();
				avPreProcTimes += res.preprocessing_time();
				avSteps += res.number_of_steps();
				avLB += res.lb();

			}
			else {			
				//time_out
				if(nRep_== 1){									//if only one test, time out results are reported anyway
				
					avSol += res.ub();
					maxSol = avSol;			
					avSteps += res.number_of_steps();
					avLB += res.lb();
				}

				nFails++;										//if more than one test, time out results are not reported
			}
		}//endFor repetitions for algorithm j

		///////////////////////////
		// Report results for algorithm j
		 
		//counter info (indep. of FAILS)
	/*	for(auto i = 0; i < avnCounters.size(); ++i){
			avnCounters[i] /= nRep_;
		}
		arrayOfCounters.push_back(avnCounters);*/

	
		//average info - taking into account FAILS
		arrayOfFails.push_back(nFails);

		if(nFails != nRep_){
			
			auto nonFailedReps = nRep_ - nFails;

			//At least one case in which algorithm j did not time out exists
			arrayOfAvSol.push_back			(avSol   / nonFailedReps);
			arrayOfAvTimes.push_back		(avTimes / nonFailedReps);
			
			arrayOfAvPreProcTimes.push_back	(avPreProcTimes / nonFailedReps);

			arrayOfAvSteps.push_back		(avSteps / nonFailedReps);
			arrayOfAvLB.push_back			(avLB / nonFailedReps);
			arrayOfMaxSol.push_back			(maxSol);
		}
		else
		{ 
			//Algorithm j timed-out in all cases
			arrayOfAvTimes.push_back		(-1.0);	
			arrayOfAvPreProcTimes.push_back	(-1.0);

			if(nRep_ == 1)
			{ 
				//single execution: report values anyway
				arrayOfAvSol.push_back		(avSol);
				arrayOfAvSteps.push_back	(avSteps);
				arrayOfAvLB.push_back		(avLB);
				arrayOfMaxSol.push_back		(maxSol);
			}
			else
			{
				//multiple repetitions: do not show values
				arrayOfAvSol.push_back		(0.0);
				arrayOfAvSteps.push_back	(0.0);
				arrayOfAvLB.push_back		(0.0);
				arrayOfMaxSol.push_back		(0);
			}
		}

	}//endFor algorithms

	//compares algorithms (use for comparison of two algorithms)
	if(info != nullptr){

		if (com::stl::all_equal(arrayOfAvSol))		{ info -> same_sol = true; }
		if (com::stl::all_equal(arrayOfAvSteps))	{ info -> same_steps = true; }
		if (com::stl::all_equal(arrayOfAvLB))		{ info -> same_lb = true; }
		if (arrayOfAvSteps[0] > arrayOfAvSteps[1])	{ info -> steps_first_greater = true; }
		
		info -> steps_lhs = arrayOfAvSteps[0]; 
		info -> steps_rhs = arrayOfAvSteps[1]; 
	}

	//TODO - STANDARD DEVIATION ANALYSIS
	
return 0;
}

template<class AlgInfo_t>
bool InfoAnalyser<AlgInfo_t>::consistent_sol_val(int& num_error){

	num_error = -1;
	bool same_sol = true;

	//check no reported solutions
	if (arrayOfAvSol.empty()){
		LOG_ERROR("No reported solutions, possibly all timed -  InfoAnalyser<AlgInfo_t>::is_consistent_sol");
		num_error = 0;	
		return false;
	}

	//iterates over the array of avergaed solutions
	same_sol = true;
	double firstItem = arrayOfAvSol.front();
	for (auto it = arrayOfAvSol.begin() + 1; it != arrayOfAvSol.end() ; it++) {
		
		//finds a different solution
		if(*it != firstItem) {
			same_sol = false;
			num_error = std::distance (arrayOfAvSol.begin(), it);		//CHECK (20/01/2015)
			break;
		}

	}

	return same_sol;
}

template<class AlgInfo_t>
bool InfoAnalyser<AlgInfo_t>::is_consistent_array_of_tests()
{
	return (	(nRep_ == arrayOfTests_.size())		&& 
				(nAlg_ == arrayOfTests_[0].size())		);

}

template<class AlgInfo_t>
ostream& InfoAnalyser<AlgInfo_t>::print_single(ostream & o, int idAlg){

	//update nRep_ and nAlg_ appropiately
	make_consistent();
	if(nRep_ == 0 || nAlg_ == 0 ){
		LOG_ERROR("Empty tests - InfoAnalyser<AlgInfo_t>::print_single");
		return o;
	}

	//default cases
	if (idAlg == -1) {
		idAlg = nAlg_;
	}
	else {
		(idAlg > nAlg_) ? idAlg = nAlg_ : 1;
	}
	
	o << "------------------------------------------" << endl;
	
	for(auto r = 0; r < nRep_; ++r){
		for(auto a = 0; a < idAlg; ++a){

			/////////////////////////////////
			o<<arrayOfTests_[r][a]<<" ";
			/////////////////////////////////
		}
		o << endl;
	}

	o << "------------------------------------------" << endl;

	return o;
}

template<class AlgInfo_t>
std::ostream& InfoAnalyser<AlgInfo_t>::print_single_rep	(ostream & o, int nRep, int idAlg){

	//assert
	if (nRep < 0) {
		LOG_ERROR("incorrect number of repetitions", nRep, "-InfoAnalyser<AlgInfo_t>::print_single_rep");
		return o;
	}
	
	//update nRep_ and nAlg_ appropiately
	make_consistent();
	if(nRep_==0 || nAlg_==0 ){
		LOG_ERROR("Empty tests - InfoAnalyser<AlgInfo_t>::print_single_rep");
		return o;
	}	

	//default cases
	if (idAlg == -1){
		idAlg = nAlg_; 
	}
	else if (idAlg > nAlg_) {
		idAlg = nAlg_;
	}
	if (nRep > nRep_) {
		nRep = nRep_;
	}
		
	//Streams data: Exception possible because nRep is not Synchro
	o << "------------------------------------------" << endl;
	
	for(int a = 0; a < idAlg; ++a){

		try{

			////////////////////////////////////////
			o << arrayOfTests_[nRep - 1][a]<<" ";				//nRep is 1-based
			////////////////////////////////////////

		}catch(exception e){
			LOGG_ERROR("Bad output", " Test:", a, " Rep:", nRep,  "-InfoAnalyser<AlgInfo_t>::print_single_rep");
			break;
		}
	}

	o << "------------------------------------------" << endl;
	return o;
}

template<class AlgInfo_t>
int InfoAnalyser<AlgInfo_t>::make_consistent(){
		
	int retVal = 0;

	///////////////////////////////////////////////////////////////
	nRep_ = arrayOfTests_.size();
	(nRep_ > 0)?  nAlg_ = arrayOfTests_[0].size() :  nAlg_=0;
	///////////////////////////////////////////////////////////////	
	
	(nRep_ > 0 && nAlg_ > 0)? retVal = 0 : retVal = -1;
	return retVal;
}


/////////////////////////////////
// declaration of valid types 

template class InfoAnalyser< com::infoCLQ<int> >;
template class InfoAnalyser< com::infoCLQ<double> >;

template std::ostream& operator << (std::ostream& o, const InfoAnalyser<com::infoCLQ<int>>& t);
template std::ostream& operator << (std::ostream& o, const InfoAnalyser<com::infoCLQ<double>>& t);

/////////////////////////////////
