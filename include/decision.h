////////////////////////////////////////////////////////////////////////////////
///
/// @file decision.h
/// @brief Decision Tree Learning Framework Header File
/// @details Based on ID3 algorithm. Also, I modified it allows a target @n
///          attribute to have multiple values rather than binary value. @n
///
/// @author Suwon Oh <suwon@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2015/01/22 Suwon Oh created @n
///
/// @section reference_section Reference
/// MACHINE LEARNING - TOM M. MITCHELL
/// 
/// @section purpose_section Purpose
/// Application trial for personal studying
///

#ifndef __DECISIONTREE__
#define __DECISIONTREE__

#ifndef NULL
#define NULL 0
#endif

typedef int ATTINDEX;  ///< means an attribute index
typedef int ATTVAL;    ///< means an attribute value

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Decision Tree Node Class
/// @details Decision Tree Learning has no exact form of hypothesis, but @n
///          the path which follows any tree node is one of hypothesis. @n
///          Tree node has a list for possible values which this attribute @n
///          can have, and a value which is target value means when this node @n
///          is a leaf. When this node is not leaf, value means next test attribute @n
///          index.
///

class DTreeNode {
private:
  DTreeNode **child;      ///< The array of child node (array index means attr possible value)
  ATTINDEX attIndex;      ///< current node attribute index
  ATTVAL predict;         ///< target prediction
  ATTVAL nr_val;    ///< the number of possible value

public:
  /// @name constructor
  /// @{
  
  /// @brief default constructor
  DTreeNode(void);
  
  /// @brief constructor taking children
  ///
  /// @param array child array
  /// @param index attribute index
  /// @param n the number of possible value
  DTreeNode(DTreeNode **array, ATTINDEX index, ATTVAL n);

  /// @brief constructor making leaf
  ///
  /// @param pred target prediction value
  DTreeNode(ATTVAL pred);
  /// @}
  
  /// @name destructor
  /// @{

  /// @brief default destructor
  ~DTreeNode(void);
  /// @}
  
  /// @name Informative Attributes
  /// @{

  /// @brief getting the child entry node
  ///
  /// @param val child possible value
  /// @retval child entry node
  /// @retval NULL if fail to get
  DTreeNode* getChild(ATTVAL val) const;
  
  /// @brief getting attribute index
  ///
  /// @retval attribute index
  ATTINDEX getIndex(void) const;
  
  /// @brief getting target prediction
  ///
  /// @retval target prediction
  ATTVAL getPred(void) const;
  
  /// @brief getting the number of possible value
  ///
  /// @retval the number of possible value
  ATTVAL getNrVal(void) const;
  
  /// @brief check if this node is leaf
  ///
  /// @retval true if it is leaf
  /// @retval false if it is not leaf
  bool isLeaf(void) const;
  /// @}
  
  /// @name Functional Attributes
  /// @{

  /// @brief setting new child to this node
  ///
  /// @param node new child node
  /// @param val this attribute possible value which links to child 
  /// @retval true if success to add, otherwise false
  bool setChild(DTreeNode* node, ATTVAL val);
  
  /// @brief setting new values
  ///
  /// @param index attribute index
  /// @param pred target prediction
  /// @param nr the number of possible value
  void setValue(ATTINDEX index, ATTVAL pred, ATTVAL nr);
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Decision Tree Class
/// @details DTree is consisting with DTreeNodes. It supports minimal tree @n
///          feature which need in decision tree algorithm.
///

class DTree {
private:
  DTreeNode       *root;            ///< Decision Tree Root
  ATTVAL    *att_size_array;        ///< each index entry means that attribute can have
                                    ///     much possible attributes
  ATTINDEX  nr_Att;                 ///< the number of attributes including target attr 

public:
  /// @name constructor
  /// @{

  /// @brief constructor taking the number of attr
  ///
  /// @param att_array attribute possible value number array
  /// @param n the number of attr
  DTree(ATTVAL *att_array, ATTINDEX n);
  /// @}

  /// @name destructor
  /// @{

  /// @brief default destructor
  ~DTree(void);
  /// @}
  
  /// @name informative attributes
  /// @{

  /// @brief getting tree root
  ///
  /// @retval root tree root
  DTreeNode* getRoot(void) const;
  
  /// @brief getting leaf node from path array
  ///
  /// @param attArray array of attr path
  /// @retval child decision tree leaf node
  /// @retval NULL if fail to find
  DTreeNode* getLeaf(ATTVAL* attArray) const;
  /// @}
  
  /// @name functional attributes
  /// @{

  /// @brief setting tree root
  ///
  /// @param root tree root
  void setRoot(DTreeNode* root);
  /// @}
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief ID3 Algorithm Engine
/// @details ID3 algorithm is most basic form of decision tree learning. @n
///          It uses DTree
///

class ID3 {
private:
  DTree *dtree;         ///< a decision tree used for ID3
  ATTVAL *attSizes;     ///< attribute size information array including target attr
  ATTINDEX nr_Att;      ///< the number of attributes including target attributes
  ATTVAL **train;       ///< two dimension array of training data set
  ATTVAL **valid;       ///< two dimension array of validation data set
  int nr_train;         ///< the number of training data
  int nr_valid;         ///< the number of validation data
  
  /// @name help private attributes
  /// @{

  /// @brief counting given attribute combination in training set
  ///
  /// @param att_array attribute combination ( ATTVAL -1 is recognized as dontcare )
  /// @param examples given examples
  /// @param nr_ex the number of examples
  /// @retval counting number
  int countInstance(ATTVAL *att_array, ATTVAL **examples, int nr_ex);
  
  /// @brief calculating entropy
  ///
  /// @param examples given examples
  /// @param nr_ex the number of examples
  /// @retval entropy for given examples with target attr
  double calEntropy(ATTVAL **examples, int nr_ex);

  /// @brief calculating Gain value
  ///
  /// @param att attribute which is wanted to test
  /// @param examples given examples
  /// @param nr_ex the number of examples
  /// @retval gain value
  double calGain(ATTINDEX att, ATTVAL **examples, int nr_ex);
  /// @brief taking best attribute with Gain function
  ///
  /// @param remainAtt remaining attr displaying array ( 1 means remain, 0 means used )
  /// @param example given examples
  /// @param nr_ex the number of examples
  /// @retval best attr index
  ATTINDEX takeBestAtt(bool *remainAtt, ATTVAL **examples, int nr_ex);

  /// @brief making new example data set
  ///
  /// @param src source example data set
  /// @param nr_src the number of source data
  /// @param att the index of attribute
  /// @param val the value of this attribute
  /// @param dst destination example data array pointer
  /// @retval the number of destination data
  int makeNewExam(ATTVAL **src, int nr_src, ATTINDEX att, ATTVAL val, ATTVAL **dst);
  /// @}
public:
  /// @name constructor
  /// @{

  /// @brief constructor taking initial values
  ///
  /// @param sizes attributes sizes array
  /// @param n the number of attributes
  /// @param train training data array ( n x nr_train )
  /// @param nr_train the number of training data
  /// @param valid validation data set
  /// @param nr_valid the number of validataion data
  ID3(ATTVAL *sizes, ATTINDEX n, ATTVAL **train, 
      int nr_train, ATTVAL **valid, int nr_valid);
  /// @}

  /// @name destructor
  /// @{

  /// @brief default destructor
  ~ID3(void);
  /// @}

  /// @name functional attributes
  /// @{
  
  /// @brief making decision tree recursively
  ///
  /// @retval true if success to make
  /// @retval false if fail
  bool makeTree(DTreeNode *current = NULL, bool *remainAtt = NULL,
                ATTVAL **examples = NULL, int nr_ex = 0); 
  
  /// @brief reduced error tree pruning
  ///
  /// @retval true if success to prune
  /// @retval false if fail
  bool prune(void); 

  /// @brief predict for new unknown instance
  ///
  /// @param input new unknown input
  /// @retval target attribute value predicted
  ATTVAL predict(ATTVAL *input);
  /// @}
};

#endif /* __DECISIONTREE__ */
