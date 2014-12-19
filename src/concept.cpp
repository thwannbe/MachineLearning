////////////////////////////////////////////////////////////////////////////////
///
/// @file concept.cpp
/// @brief Concept Learning Framework Source File
/// @details Based on Candidate Elimination Algorithm, it predict target @n
///          concept. It assumes that input is a conjunctive set of attributes @n
///          including target attribute. All attribute has binary output ( T/F )
/// @author Suwon Oh <suwon@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2014/09/22 Suwon Oh created @n
/// 2014/09/24 Suwon Oh implemented prototype @n
/// 2014/12/14 Suwon Oh adapted to Doxygen @n
/// 2014/12/16 Suwon Oh adapted self-implemented list library @n
///
/// @section reference_section Reference
/// MACHINE LEARNING - TOM M. MITCHELL
/// 
/// @section purpose_section Purpose
/// Application trial for personal studying
///

#include "concept.h"
#include <iostream>

#define DEBUG_MODE  0

using namespace std;

////////////////////////////////////////////////////////////////////////////////
///
/// @brief symbol table

char SYMBOL[] = {'F', 'T', '?', '-'};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief test input positivity
/// @details only available to training example which @n
///          has target attribute value.
/// @param input training example
/// @param size the number of attributes of the example @n
///        except for target attribute
/// @retval true if example is positive
/// @retval false if example is negative

bool isPositive(bool* input, int size)
{
  return input[size];
}

Hypothesis::Hypothesis(int size, AttrVal* hypo) : size(size) {
  this->hypo = hypo;
}

Hypothesis::~Hypothesis() {
  if(hypo)
    delete(hypo);
}

AttrVal* Hypothesis::getHypo(void) const
{
  return hypo;
}

bool Hypothesis::isCover(bool* input) const
{
  bool cover = true;

  for(int i = 0; i < size; i++) {
    if(cover) {
      AttrVal curVal = hypo[i];
      bool curInput = input[i];
      // if cover is true, continue to search
      cover = !((curVal == vFalse && curInput == true) || \
                (curVal == vTrue && curInput == false) || \
                (curVal == vAllDeny));
    }
  }

  return cover;
}

bool Hypothesis::isMoreGeneralThanEqualTo(Hypothesis* h) const
{
  // h should have same size with this hypothesis
  AttrVal* compare = h->getHypo();
  
  bool more_general = true;
  for(int i = 0; i < size; i++) {
    if((hypo[i] == vAllDeny && compare[i] != vAllDeny) || \
      (hypo[i] != vAllAccept && compare[i] != vAllDeny && \
      hypo[i] != compare[i])) {
      more_general = false;
      break;
    }
  }

  return more_general;
}

S_Bound::S_Bound(int size) : size(size)
{
  // create most specific hypothesis
  AttrVal* array = new AttrVal[size];
#if DEBUG_MODE
  cout << " -: Most Specific Bound is initialized with ... " << endl;
  cout << " -: < ";
#endif
  for(int i = 0; i < size; i++) {
    array[i] = vAllDeny;	// most specific hypothesis
#if DEBUG_MODE
    cout << SYMBOL[array[i]] << " ";
#endif
  }
#if DEBUG_MODE
  cout << "> " << endl;
#endif
  Hypothesis* hypo = new Hypothesis(size, array);
  sb_Array = new List <Hypothesis*>();
  sb_Array->addNode(hypo);
}

S_Bound::~S_Bound()
{
  if(sb_Array)
    delete(sb_Array);
}

bool S_Bound::negUpdate(bool* n_input)
{
  // at first, check whether this input is negative
  if(isPositive(n_input, size))
	  return false;

  for(unsigned int i = 0; i < sb_Array->getSize(); i++) {
	  if(sb_Array->getContent(i)->isCover(n_input)) {
#if DEBUG_MODE
	    cout << " -: S_Bound Entry < ";
	    for(int j = 0; j < size; j++) {
		    cout << SYMBOL[sb_Array->getContent(i)->getHypo()[j]] << " ";
	    }
	    cout << "> is deleted." << endl;
#endif
	    if(!sb_Array->delNode(i))
        return false;
      i--; // rearrange array
	  }
  }
  return true;
}

bool S_Bound::posUpdate(bool* p_input)
{
  // at first, check whether this input is positive
  if(!isPositive(p_input, size))
    return false;

  for(unsigned int i = 0; i < sb_Array->getSize(); i++) {
    if(!sb_Array->getContent(i)->isCover(p_input)) {
    // should be more generalized
#if DEBUG_MODE
      cout << " -: S_Bound Entry < ";
#endif
      AttrVal* copy = new AttrVal[size];
      for(int j = 0; j < size; j++) {
        copy[j] = sb_Array->getContent(i)->getHypo()[j];
#if DEBUG_MODE
        cout << SYMBOL[copy[j]] << " ";
#endif		
      }
#if DEBUG_MODE
      cout << "> is updated with ..." << endl;
#endif
      // replace generalized one
      if(!sb_Array->delNode(i))
        return false;
      if(!sb_Array->addNode(generalize(copy, p_input)))
        return false;
    }
  }
  return true;
}

Hypothesis* S_Bound::generalize(AttrVal* hypo, bool* p_input) {
#if DEBUG_MODE
  cout << " -: < ";
#endif
  for(int i = 0; i < size; i++) {
    if(hypo[i] == vAllDeny) {
      hypo[i] = (p_input[i]) ? vTrue : vFalse;
    } else if(hypo[i] != vAllAccept) {
      if((hypo[i] == vTrue && !p_input[i]) || (hypo[i] == vFalse && p_input[i]))
        hypo[i] = vAllAccept;
    }
#if DEBUG_MODE
    cout << SYMBOL[hypo[i]] << " ";
#endif
  }
#if DEBUG_MODE
  cout << "> " << endl;
#endif

  Hypothesis* h = new Hypothesis(size, hypo);
  return h;
}

Result S_Bound::predict(bool* u_input) {
  Result ret = r_dontknow;
  bool first_val;

  for(unsigned int i = 0; i < sb_Array->getSize(); i++) {
    if(i == 0)
      first_val = sb_Array->getContent(i)->isCover(u_input);
    else if(sb_Array->getContent(i)->isCover(u_input) != first_val)
      return r_dontknow;
  }

  return (first_val) ? r_true : r_false;
}

G_Bound::G_Bound(int size) : size(size)
{
  // create most general hypothesis
  AttrVal* array = new AttrVal[size];
#if DEBUG_MODE
  cout << " -: Most General Bound is initialized with ... " << endl;
  cout << " -: < ";
#endif
  for(int i = 0; i < size; i++) {
    array[i] = vAllAccept;	// most general hypothesis
#if DEBUG_MODE
    cout << SYMBOL[array[i]] << " ";
#endif
  }
#if DEBUG_MODE
  cout << "> " << endl;
#endif
  Hypothesis* hypo = new Hypothesis(size, array);
  gb_Array = new List <Hypothesis*>();
  gb_Array->addNode(hypo);
}

G_Bound::~G_Bound()
{
  if(gb_Array)
    delete(gb_Array);
}

bool G_Bound::posUpdate(bool* p_input)
{
  // at first, check whether this input is positive
  if(!isPositive(p_input, size))
    return false;

  for(unsigned int i = 0; i < gb_Array->getSize(); i++) {
    if(!gb_Array->getContent(i)->isCover(p_input)) {
#if DEBUG_MODE
      cout << " -: G_Bound Entry < ";
      for(int j = 0; j < size; j++) {
        cout << SYMBOL[gb_Array->getContent(i)->getHypo()[j]] << " ";
      }
      cout << "> is deleted." << endl;
#endif
      if(!gb_Array->delNode(i))
        return false;
      i--; // rearrange array
    }
  }
  return true;
}

bool G_Bound::negUpdate(bool* n_input, S_Bound* ptr_sb) {
  // at first, check whether this input is negative
  if(isPositive(n_input, size))
    return false;

  for(unsigned int i = 0; i < gb_Array->getSize(); i++) {
    if(gb_Array->getContent(i)->isCover(n_input)) {
#if DEBUG_MODE
      cout << " -: G_Bound Entry < ";
#endif
      AttrVal* copy = new AttrVal[size];
      for(int j = 0; j < size; j++) {
        copy[j] = gb_Array->getContent(i)->getHypo()[j];
#if DEBUG_MODE
        cout << SYMBOL[copy[j]] << " ";
#endif		
      }
#if DEBUG_MODE
      cout << "> is updated with ..." << endl;
#endif
      if(!gb_Array->delNode(i))
        return false;
      if(!specialize(copy, n_input, ptr_sb))
        return false;
    }
  }
  return true;
}

bool G_Bound::specialize(AttrVal* hypo, bool* n_input, S_Bound* ptr_sb) {
  // search modifiable attribute
  for(int i = 0; i < size; i++) {
    if(hypo[i] == vAllAccept) {
      // make next more specific hypothesis
      AttrVal* next_sp = new AttrVal[size];
      for(int j = 0; j < size; j++) {
        if(i == j)
          next_sp[j] = (n_input[j]) ? vFalse : vTrue;
        else
          next_sp[j] = hypo[j];
      }
      
      // check it is general than S_Bound h
      Hypothesis* tmp_h = new Hypothesis(size, next_sp);
      bool isCoverAllSBound = true;
      for(unsigned int j = 0; j < ptr_sb->sb_Array->getSize(); j++) {
        if(!tmp_h->isMoreGeneralThanEqualTo(ptr_sb->sb_Array->getContent(j))) {
          isCoverAllSBound = false;
          break;
        }
      }
      
      // check it is specific than other G_Bound h
      if(isCoverAllSBound) {
        bool isSpecific = false;
        for(unsigned int j = 0; j < gb_Array->getSize(); j++) {
          if(gb_Array->getContent(j)->isMoreGeneralThanEqualTo(tmp_h)) {
            isSpecific = true;
            break;
          }
        }
        
        if(isSpecific) {
          delete(tmp_h);
        }
        else {	// all conditions satisfied
#if DEBUG_MODE
          cout << " -: < ";
          for(int j = 0; j < size; j++) {
            cout << SYMBOL[next_sp[j]] << " ";
          }
          cout << "> " << endl;
#endif
          if(!gb_Array->addNode(tmp_h))
            return false;
        }
      } else {
        delete(tmp_h);
      }
    }
  }
  return true;
}

Result G_Bound::predict(bool* u_input) {
  Result ret = r_dontknow;
  bool first_val;

  for(unsigned int i = 0; i < gb_Array->getSize(); i++) {
    if(i == 0)
      first_val = gb_Array->getContent(i)->isCover(u_input);
    else if(gb_Array->getContent(i)->isCover(u_input) != first_val)
      return r_dontknow;
  }

  return (first_val) ? r_true : r_false;
}

CE::CE(int size) : size(size)
{
#if DEBUG_MODE
  cout << "          <<< Candidate Elimination Framework (v1.0) >>>" << endl << endl;
  cout << "-------------------------- INITIALIZATION ----------------------------" << endl;
#endif
  s_bound = new S_Bound(size);
  g_bound = new G_Bound(size);
#if DEBUG_MODE
  cout << "----------------------------------------------------------------------" << endl << endl;
#endif
}

CE::~CE()
{
  if(s_bound)
    delete(s_bound);
  if(g_bound)
    delete(g_bound);
}

bool CE::updateVS(bool* input) {
  bool success;
#if DEBUG_MODE
  cout << "------------------------------ UPDATE --------------------------------" << endl;
  cout << " -: Input < ";
  for(int i = 0; i < size; i++) {
    int tmp = (input[i]) ? 1 : 0;
    cout << SYMBOL[tmp] << " ";
  }
  int ret = (input[size]) ? 1 : 0;
  cout << "> = (" << SYMBOL[ret] << ")" << endl << endl;
#endif
  if(input[size]) {    // means positive example
    success = g_bound->posUpdate(input);
    success &= s_bound->posUpdate(input);
  } else {	// means negative example
    success = s_bound->negUpdate(input);
    success &= g_bound->negUpdate(input, s_bound);
  }
#if DEBUG_MODE
  cout << "----------------------------------------------------------------------" << endl << endl;
#endif
  return success;
}

Result CE::predict(bool* u_input) {
  Result pred;
#if DEBUG_MODE
  cout << "------------------------------ PREDICT -------------------------------" << endl;
  cout << " -: New instance < ";
  for(int i = 0; i < size; i++) {
    int tmp = (u_input[i]) ? 1 : 0;
    cout << SYMBOL[tmp] << " ";
  }
#endif

  Result s_ret = s_bound->predict(u_input);
  Result g_ret = g_bound->predict(u_input);
  
  pred = (s_ret == g_ret) ? s_ret : r_dontknow;
#if DEBUG_MODE
  cout << "=> (" << SYMBOL[pred] << ")" << endl;
  cout << "----------------------------------------------------------------------" << endl << endl;
#endif
  return pred;
}
