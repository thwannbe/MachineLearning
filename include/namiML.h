////////////////////////////////////////////////////////////////////////////////
///
/// @file namiML.h
/// @brief NAtive Mechanic Intelligent Machine Learning Framework Header File
/// @details namiML framework supports multiple machine learning algorithms. @n
///          Some algorithms require different forms of inputs or training @n
///          data. Be careful not to adapt wrong type of data before executing @n
///          the program. The following is the list of machine learning algorithms @n
///          which are supported by this program and the input specifications.
/// 
/// @section list_section Lists
/// @b CONCEPT @b LEARNING - BASED ON CANDIDATE ELIMINATION ; @n
///                    This assume that all input is a conjunction of binary @n
///                    attribute with a target attribute as its training data. @n
///                    Also training data must have no error. @n
///                    New instance which will be predicted should have same number @n
///                    of attributes like training data except for target attribute. @n
///                    All attributes must be clear(i.e. 1-true or 0-false). @n @n
/// @b DECISION @b TREE - COMING SOON ; 
/// @author Suwon Oh <suwon@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2014/12/18 Suwon Oh created @n
/// 2014/12/18 Suwon Oh implemented prototype @n
/// 2014/12/18 Suwon Oh adapted to Doxygen @n
/// 2014/12/18 Suwon Oh added to concept learning framework @n
/// 2014/12/19 Suwon Oh finished first version @n
/// 2015/01/17 Suwon Oh option update @n
/// 2015/01/17 Suwon Oh added generating input & train functions for CE@n
/// 2015/01/23 Suwon Oh added decision tree learning @n
///
/// @section reference_section Reference
/// MACHINE LEARNING - TOM M. MITCHELL
/// 
/// @section purpose_section Purpose
/// Application trial for personal studying
///

#ifndef __NAMIML__
#define __NAMIML__

#include <iostream>     ///< for standard output
#include <fstream>      ///< managing console output
#include "concept.h"
#include "decision.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Machine Learning Abstarct Machine Framework
///

class ML_Machine {
protected:
  ios     *input;       ///< new instance io stream
  ios     *training;    ///< training data io stream
  ios     *answer;      ///< answer data io stream
  ostream *output;      ///< output stream
public:
  /// @name constructor & destructor
  /// @{
  ML_Machine(ios *input, ios *training, ios *answer, ostream *output);
  virtual ~ML_Machine(void);
  /// @}

  /// @name functional attributes
  /// @{
  virtual bool train(void) = 0;
  virtual bool predict(void) = 0;
  virtual bool generate(void) = 0;
  /// @}
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Candidate Elmination Abstarct Machine
/// @details CE_MACHINE provides abstract layer to outer framework of CE 
///

class CE_Machine : public ML_Machine {
private:
  int size;       ///< hypothesis attribute number
  CE* ce;         ///< candidate elimination engine
  
  /// @brief create target concept
  ///
  /// @retval Result enum array which represents target concept
  Result* createTarget(void);
  
  /// @brief make one entry
  ///
  /// @param target target concept attribute value
  /// @param t true or false
  bool makeOneEntry(Result target, bool t);

  /// @brief create instance from target concept
  ///
  /// @param target target concept
  /// @param t flag if true instance or false instance
  /// @retval Boolean array which represents instance
  bool* createInstance(Result* target, bool t);

public:
  /// @name constructor & destructor
  /// @{
  
  /// @brief CE constructor
  ///
  /// @param input new instance io stream
  /// @param training training data io stream
  /// @param answer answer data io stream
  /// @param output output stream
  CE_Machine(ios *input, ios *training, ios *answer, ostream *output);

  /// @brief CE destructor
  virtual ~CE_Machine(void);
  /// @}
  
  /// @name functional attributes
  /// @{
  
  /// @brief train CE
  ///
  /// @retval true if training succeeds
  /// @retval false if training fails
  virtual bool train(void);

  /// @brief predict CE
  ///
  /// @retval true if prediction succeeds
  /// @retval false if prediction fails 
  virtual bool predict(void);

  /// @brief generate CE input and train data
  ///
  /// @retval true if generation succeeds
  /// @retval false if generation fails
  virtual bool generate(void);
  /// @}
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief ID3 Learning Abstarct Machine
/// @details ID3_MACHINE provides abstract layer to outer framework of ID3
///

class ID3_Machine : public ML_Machine {
private:
  ATTVAL* attSizes; ///< attribute size information array including target
  ATTINDEX nr_att;    ///< the number of attr including target attr
  ID3* id3;         ///< ID3 engine
  
  /// @brief recursively making answer tree randomly
  ///
  /// @param parent parent DTreeRoot
  /// @param remainAtt remain attribute flags array
  /// @param ans answer output stream
  /// @param indent indentation for answer file & standard output
  /// @param first flag for whether current node is first child of parent
  /// @param last flag for whether current node is last child of parent
  /// @param tree master tree
  /// @param ch_index index for child number of parent
  void recursive_make_answer_tree(DTreeRoot *parent, bool *remainAtt, 
  ostream *ans, int indent, bool first, bool last, DTree *tree, int ch_index); 
  /// @brief making answer tree
  ///
  /// @retval answer tree
  DTree* make_answer_tree(ostream *ans);

  /// @brief recursively generating answer tree
  ///
  /// @param parent parent DTreeRoot
  /// @param ans answer text file istream
  /// @param tree master tree
  /// @param ch_index child index
  void recursive_answer_tree_gen(DTreeRoot *parent, istream *ans, DTree *tree,
  int ch_index);
   
  /// @brief generating answer tree
  ///
  /// @retval answer tree
  DTree* answer_tree_gen(void);
    
  /// @brief creating a ID3 instance
  ///
  /// @retval ID3 instance array
  ATTVAL* createInstance(void);

public:
  /// @name constructor & destructor
  /// @{
  
  /// @brief ID3 constructor
  ///
  /// @param input new instance io stream
  /// @param training training data io stream
  /// @param answer answer data io stream
  /// @param output output stream
  ID3_Machine(ios *input, ios *training, ios *answer, ostream *output);

  /// @brief CE destructor
  virtual ~ID3_Machine(void);
  /// @}
  
  /// @name functional attributes
  /// @{
  
  /// @brief train ID3
  ///
  /// @retval true if training succeeds
  /// @retval false if training fails
  virtual bool train(void);

  /// @brief predict ID3
  ///
  /// @retval true if prediction succeeds
  /// @retval false if prediction fails 
  virtual bool predict(void);

  /// @brief generate ID3 input and train data
  ///
  /// @retval true if generation succeeds
  /// @retval false if generation fails
  virtual bool generate(void);
  /// @}
};

#endif /* __NAMIML__ */
