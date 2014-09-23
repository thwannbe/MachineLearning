#include "concept.h"
#include <cstdlib>

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

void Hypothesis::generalize(bool* input) {
}

void Hypothesis::specialize(bool* input) {
}

void S_Bound::Initialize(int size) {
    AttrVal* hArray = new AttrVal[size];
    
    for(int i = 0; i < size; i++) {
	hArray[i] = vAllDeny;	// most specific hypothesis
    }

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
	    sb_Array.erase(i);	    // should remove this hypothesis from S_Bound
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
	    i->generalize(p_input);
	}
    }
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
    
    for(int i = 0; i < size; i++) {
	hArray[i] = vAllAccept;	// most general hypothesis
    }

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
	    gb_Array.erase(i);	    // should remove this hypothesis from G_Bound
	}
    }
}

void G_Bound::UpdateBound(bool* n_input) {
    // at first, check whether this input is positive
    if(n_input[AttrNum] == true) {  // this is positive input
	exit(-1);		    // trap
    }
    
    for(list<Hypothesis>::iterator i = gb_Array.begin(); i != gb_Array.end(); i++) {
	if(i->isCover(n_input)) {  // for not consistent hypothesis
	    i->generalize(n_input);
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
    s_bound.Initialize(size);
    g_bound.Initialize(size);
    AttrNum = size;
}

void ConceptLearning::UpdateVersionSpace(bool* input) {
    if(input[AttrNum]) {    // means positive example
	g_bound.CoverCheckUpdate(input);
	s_bound.UpdateBound(input);
    } else {	// means negative example
	s_bound.CoverCheckUpdate(input);
	g_bound.UpdateBound(input);
    }
}

Result ConceptLearning::CoverCheck(bool* u_input) {
    Result s_ret = s_bound.CoverCheck(u_input);
    Result g_ret = g_bound.CoverCheck(u_input);
    
    return (s_ret == g_ret) ? s_ret : r_dontknow;
}
