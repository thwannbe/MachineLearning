////////////////////////////////////////////////////////////////////////////////
///
/// @brief Concept Learning Framework
/// @author Suwon Oh <suwon@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2014/09/22 Suwon Oh created
/// 2014/09/24 Suwon Oh implemented prototype
/// 2014/12/14 Suwon Oh adapted to Doxygen
///
/// @section reference_section Reference
/// MACHINE LEARNING - TOM M. MITCHELL
/// 
/// @section purpose_section Purpose
/// Application trial for personal studying
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __CONCEPTLEARNING__
#define __CONCEPTLEARNING__

#include <list>

using namespace std;

enum AttrVal {
    vFalse=0,		// means false of this attribute
    vTrue,		// means true of this attribute 
    vAllAccept,		// can accept any value of attribute ( true or false ) -- '?'
    vAllDeny,		// deny all value of attribute -- 'null'
};

enum Result {
    r_false=0,		// means all hypothesis vote false		
    r_true,		// means all hypothesis vote true
    r_dontknow		// means hypothesis vote is not unanimous
};

class Hypothesis {
private:
    AttrVal* hArray;	// hypothesis format array ; ex) < ? , T , F, null, ? > ...
    int AttrNum;
public:
    Hypothesis(int size, AttrVal* hArray);
    AttrVal* GethArray(void);
    bool isCover(bool* input);
    bool isMoreGeneralThanEqualTo(Hypothesis h);
};

class S_Bound {
private:
    list <Hypothesis> sb_Array;   // set of hypothesis in most specific bound
    int AttrNum;
    friend class G_Bound;
public:
    void Initialize(int size);
    void CoverCheckUpdate(bool* n_input);	    // n_input should be negative
    void UpdateBound(bool* p_input);    // p_input should be positive
    void Generalize(AttrVal* hArray, bool* p_input);
    Result CoverCheck(bool* u_input); // u_input don't have target attr (unknown)
};

class G_Bound {
private:
    list <Hypothesis> gb_Array;    // set of hypothesis in most general bound
    int AttrNum;
public:
    void Initialize(int size);
    void CoverCheckUpdate(bool* p_input);		// p_input should be positive
    void UpdateBound(bool* n_input, void* ptr_S_Bound);	// n_input should be negative
    void Specialize(AttrVal* hArray, bool* n_input, void* ptr_S_Bound);
    Result CoverCheck(bool* u_input);
};

class ConceptLearning {
private:
    S_Bound s_bound;
    G_Bound g_bound;
    int AttrNum;
public:
    void Initialize(int size);
    void UpdateVersionSpace(bool* input);   // input should have target attr
    Result CoverCheck(bool* u_input);
};

#endif /* __CONCEPTLEARNING__ */
