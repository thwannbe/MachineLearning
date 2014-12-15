#include "concept.h"
#include <cstdlib>
#include <iostream>

#define DEBUG_MODE  1

char SYMBOL[] = {'F', 'T', '?', '-'};

Hypothesis::Hypothesis(int size, AttrVal* hArray) : AttrNum(size) {
    this->hArray = hArray;
}

AttrVal* Hypothesis::GethArray(void) {
    return hArray;
}

bool Hypothesis::isCover(bool* input) {
    bool cover = true;

    for(int i = 0; i < AttrNum; i++) {
	if(cover) {
	    AttrVal curVal = hArray[i];
	    bool curInput = input[i];
	    cover = !((curVal == vFalse && curInput == true) || (curVal == vTrue && curInput == false) || \
		    (curVal == vAllDeny));
	}
    }

    return cover;
}

bool Hypothesis::isMoreGeneralThanEqualTo(Hypothesis h) {
    // h should have same size with this hypothesis
    AttrVal* compare_with = h.GethArray();
    
    bool more_general = true;
    for(int i = 0; i < AttrNum; i++) {
	if((hArray[i] == vAllDeny && compare_with[i] != vAllDeny) || (hArray[i] != vAllAccept && compare_with[i] != vAllDeny \
	    && hArray[i] != compare_with[i])) {
	    more_general = false;
	    break;
	}
    }

    return more_general;
}

void S_Bound::Initialize(int size) {
    AttrVal* hArray = new AttrVal[size];
   
#if DEBUG_MODE
    cout << " -: Most Specific Bound is initialized with ... " << endl;

    cout << " -: < ";
#endif

    for(int i = 0; i < size; i++) {
	hArray[i] = vAllDeny;	// most specific hypothesis
#if DEBUG_MODE
	cout << SYMBOL[hArray[i]] << " ";
#endif
    }

#if DEBUG_MODE
    cout << "> " << endl;
#endif

    Hypothesis* hypo = new Hypothesis(size, hArray);
    
    sb_Array.push_back(*hypo);

    AttrNum = size;
}

void S_Bound::CoverCheckUpdate(bool* n_input) {
    // at first, check whether this input is negative
    if(n_input[AttrNum] == true) {  // this is positive input
	exit(-1);   		    // trap
    }

    for(list<Hypothesis>::iterator i = sb_Array.begin(); i != sb_Array.end(); i++) {
	if(i->isCover(n_input)) {
#if DEBUG_MODE
	    cout << " -: S_Bound Entry < ";
	    for(int j = 0; j < AttrNum; j++) {
		cout << SYMBOL[i->GethArray()[j]] << " ";
	    }
	    cout << "> is deleted." << endl;
#endif
	    sb_Array.erase(i);	    // should remove this hypothesis from S_Bound
	    i--;
	}
    }
}

void S_Bound::UpdateBound(bool* p_input) {
    // at first, check whether this input is positive
    if(p_input[AttrNum] == false) { // this is negative input
	exit(-1);		    // trap
    }
    
    for(list<Hypothesis>::iterator i = sb_Array.begin(); i != sb_Array.end(); i++) {
	if(!i->isCover(p_input)) {  // for not consistent hypothesis
#if DEBUG_MODE
	    cout << " -: S_Bound Entry < ";
#endif
	    AttrVal* hArray = new AttrVal[AttrNum];
	    for(int j = 0; j < AttrNum; j++) {
		hArray[j] = i->GethArray()[j];
#if DEBUG_MODE
		cout << SYMBOL[hArray[j]] << " ";
#endif		
	    }
#if DEBUG_MODE
	    cout << "> is updated with ..." << endl;
#endif
	    sb_Array.erase(i);
	    Generalize(hArray, p_input);
	}
    }
}

void S_Bound::Generalize(AttrVal* hArray, bool* p_input) {
#if DEBUG_MODE
    cout << " -: < ";
#endif
    for(int i = 0; i < AttrNum; i++) {
	if(hArray[i] == vAllDeny) {
	    hArray[i] = (p_input[i]) ? vTrue : vFalse;
	} else if(hArray[i] != vAllAccept) {
	    if((hArray[i] == vTrue && !p_input[i]) || (hArray[i] == vFalse && p_input[i]))
		hArray[i] = vAllAccept;
	}
#if DEBUG_MODE
	cout << SYMBOL[hArray[i]] << " ";
#endif
    }
#if DEBUG_MODE
    cout << "> " << endl;
#endif
    
    Hypothesis* h = new Hypothesis(AttrNum, hArray);
    sb_Array.push_back(*h);
}

Result S_Bound::CoverCheck(bool* u_input) {
    Result ret = r_dontknow;
    bool first_val;

    for(list<Hypothesis>::iterator i = sb_Array.begin(); i != sb_Array.end(); i++) {
	if(i == sb_Array.begin())
	    first_val = i->isCover(u_input);
	else if(i->isCover(u_input) != first_val)
	    return r_dontknow;
    }

    return (first_val) ? r_true : r_false;
}

void G_Bound::Initialize(int size) {
    AttrVal* hArray = new AttrVal[size];
 
 #if DEBUG_MODE
    cout << " -: Most General Bound is initialized with ... " << endl;

    cout << " -: < ";
#endif
   
    for(int i = 0; i < size; i++) {
	hArray[i] = vAllAccept;	// most general hypothesis

#if DEBUG_MODE
	cout << SYMBOL[hArray[i]] << " ";
#endif
    }
#if DEBUG_MODE
    cout << "> " << endl;
#endif

    Hypothesis* hypo = new Hypothesis(size, hArray);
    
    gb_Array.push_back(*hypo);

    AttrNum = size;
}

void G_Bound::CoverCheckUpdate(bool* p_input) {
    // at first, check whether this input is negative
    if(p_input[AttrNum] == false) { // this is negative input
	exit(-1);   		    // trap
    }

    for(list<Hypothesis>::iterator i = gb_Array.begin(); i != gb_Array.end(); i++) {
	if(!i->isCover(p_input)) {
#if DEBUG_MODE
	    cout << " -: G_Bound Entry < ";
	    for(int j = 0; j < AttrNum; j++) {
		cout << SYMBOL[i->GethArray()[j]] << " ";
	    }
	    cout << "> is deleted." << endl;
#endif
	    gb_Array.erase(i);	    // should remove this hypothesis from G_Bound
	    i--;
	}
    }
}

void G_Bound::UpdateBound(bool* n_input, void* ptr_S_Bound) {
    // at first, check whether this input is positive
    if(n_input[AttrNum] == true) {  // this is positive input
	exit(-1);		    // trap
    }
    
    for(list<Hypothesis>::iterator i = gb_Array.begin(); i != gb_Array.end(); i++) {
	if(i->isCover(n_input)) {  // for not consistent hypothesis
#if DEBUG_MODE
	    cout << " -: G_Bound Entry < ";
#endif
	    AttrVal* hArray = new AttrVal[AttrNum];
	    for(int j = 0; j < AttrNum; j++) {
		hArray[j] = i->GethArray()[j];
#if DEBUG_MODE
		cout << SYMBOL[hArray[j]] << " ";
#endif		
	    }
#if DEBUG_MODE
	    cout << "> is updated with ..." << endl;
#endif
	    gb_Array.erase(i);
	    Specialize(hArray, n_input, ptr_S_Bound);
	}
    }
}

void G_Bound::Specialize(AttrVal* hArray, bool* n_input, void* ptr_S_Bound) {
    S_Bound* ptr_sb = (S_Bound*) ptr_S_Bound;
    for(int i = 0; i < AttrNum; i++) {
	if(hArray[i] == vAllAccept) {
	    AttrVal* new_hArray = new AttrVal[AttrNum];
	    for(int j = 0; j < AttrNum; j++) {
		if(i == j)
		    new_hArray[j] = (n_input[j]) ? vFalse : vTrue;
		else
		    new_hArray[j] = hArray[j];
	    }
	    
	    Hypothesis* tmp_h = new Hypothesis(AttrNum, new_hArray);

	    bool isCoverAllSBound = true;
	    for(list<Hypothesis>::iterator iter = ptr_sb->sb_Array.begin(); iter != ptr_sb->sb_Array.end(); iter++) {
		if(!tmp_h->isMoreGeneralThanEqualTo(*iter)) {
		    isCoverAllSBound = false;
		    break;
		}
	    }
	    
	    if(isCoverAllSBound) {
		bool isSpecific = false;
		for(list<Hypothesis>::iterator iter = gb_Array.begin(); iter != gb_Array.end(); iter++) {
		    if(iter->isMoreGeneralThanEqualTo(*tmp_h)) {
			isSpecific = true;
			break;
		    }
		}

		if(isSpecific) {
		    delete(new_hArray);
		    delete(tmp_h);
		}
		else {	// all conditions satisfied
#if DEBUG_MODE
		    cout << " -: < ";
		    for(int j = 0; j < AttrNum; j++) {
			cout << SYMBOL[new_hArray[j]] << " ";
		    }
		    cout << "> " << endl;
#endif
		    gb_Array.push_back(*tmp_h);
		}
	    } else {
		delete(new_hArray);
		delete(tmp_h);
	    }
	}
    }
}

Result G_Bound::CoverCheck(bool* u_input) {
    Result ret = r_dontknow;
    bool first_val;

    for(list<Hypothesis>::iterator i = gb_Array.begin(); i != gb_Array.end(); i++) {
	if(i == gb_Array.begin())
	    first_val = i->isCover(u_input);
	else if(i->isCover(u_input) != first_val)
	    return r_dontknow;
    }

    return (first_val) ? r_true : r_false;
}

void ConceptLearning::Initialize(int size) {
#if DEBUG_MODE
    cout << " <<< Concept Learning Framework (v1.0) >>>" << endl << endl;
    cout << "-------------------------- INITIALIZATION ----------------------------" << endl;
#endif
    s_bound.Initialize(size);
    g_bound.Initialize(size);
#if DEBUG_MODE
    cout << "----------------------------------------------------------------------" << endl << endl;
#endif
    AttrNum = size;
}

void ConceptLearning::UpdateVersionSpace(bool* input) {
#if DEBUG_MODE
    cout << "------------------------------ UPDATE --------------------------------" << endl;
    cout << " -: Input < ";
    for(int i = 0; i < AttrNum; i++) {
	int tmp = (input[i]) ? 1 : 0;
	cout << SYMBOL[tmp] << " ";
    }
    int ret = (input[AttrNum]) ? 1 : 0;
    cout << "> = (" << SYMBOL[ret] << ")" << endl << endl;
#endif
    if(input[AttrNum]) {    // means positive example
	g_bound.CoverCheckUpdate(input);
	s_bound.UpdateBound(input);
    } else {	// means negative example
	s_bound.CoverCheckUpdate(input);
	g_bound.UpdateBound(input, &s_bound);
    }
#if DEBUG_MODE
    cout << "----------------------------------------------------------------------" << endl << endl;
#endif
}

Result ConceptLearning::CoverCheck(bool* u_input) {
    Result s_ret = s_bound.CoverCheck(u_input);
    Result g_ret = g_bound.CoverCheck(u_input);
    
    return (s_ret == g_ret) ? s_ret : r_dontknow;
}
