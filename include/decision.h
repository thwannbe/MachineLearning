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
/// 2015/02/01 Suwon Oh classified tree node @n
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

class DTreeRoot;
class DTreeLeaf;

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Decision Tree Node Class
///

class DTreeNode {
private:
  DTreeRoot *parent;      ///< parent node

public:
  /// @name constructor
  /// @{
  
  /// @brief default constructor
  DTreeNode(void);
  
  /// @brief constructor taking parent
  ///
  /// @param parent parent DTreeRoot
  DTreeNode(DTreeRoot *parent);
  /// @}
  
  /// @name destructor
  /// @{

  /// @brief default destructor
  ~DTreeNode(void);
  /// @}
  
  virtual void for_polymorphism(void) = 0;

  /// @name Informative Attributes
  /// @{

  /// @brief getting the parent
  ///
  /// @retval parent entry node
  DTreeRoot* getParent(void) const;
  /// @}

  /// @name Functional Attributes
  /// @{

  /// @brief setting parent node
  ///
  /// @param parent parent DTreeRoot
  void setParent(DTreeRoot *parent);
  /// @}
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Decision Tree Root (Linking Node) Class
///

class DTreeRoot : public DTreeNode {
private:
  ATTINDEX attIndex;      ///< next test attr
  DTreeNode **child;      ///< The array of child node (array index means attr possible value)
  ATTVAL nr_child;        ///< The number of child branch

public:
  /// @name constructor
  /// @{
  
  /// @brief default constructor
  DTreeRoot(void);
  
  /// @brief constructor taking all informations
  ///
  /// @param attIndex next test attr
  /// @param child the array of child node
  /// @param nr_child the number of child branch
  DTreeRoot(ATTINDEX attIndex, DTreeNode **child, ATTVAL nr_child);
  /// @}
  
  /// @name destructor
  /// @{

  /// @brief default destructor
  ~DTreeRoot(void);
  /// @}
  
  virtual void for_polymorphism(void) {};

  /// @name Informative Attributes
  /// @{
  
  /// @brief getting attribute index
  ///
  /// @retval attribute index
  ATTINDEX getAttIndex(void) const;

  /// @brief getting the child entry node
  ///
  /// @retval entire child array pointer
  DTreeNode** getChildArray(void) const;

  /// @brief getting one child from array
  ///
  /// @param index child index
  /// @retval one child DTreeNode
  DTreeNode* getOneChild(ATTVAL index) const;
  
  /// @brief getting the number of child
  ///
  /// @retval the number of possible value
  ATTVAL getNrChild(void) const;
  /// @}
  
  /// @name Functional Attributes
  /// @{
  
  /// @brief setting attribute index
  ///
  /// @param attIndex new attribute index
  void setAttIndex(ATTINDEX attIndex);
  
  /// @brief setting entire child array
  ///
  /// @param child the new array of child node
  void setChildArray(DTreeNode **child);

  /// @brief setting one child in array
  ///
  /// @param node new child node
  /// @param index new child node index
  void setOneChild(DTreeNode *node, ATTVAL index);
  
  /// @brief setting the number of child
  ///
  /// @param nr_child the number of child branch
  void setNrChild(ATTVAL nr_child);
  /// @}
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Decision Tree Leaf (Edge Node) Class
///

class DTreeLeaf : public DTreeNode {
private:
  ATTVAL val;        ///< target attr prediction value

public:
  /// @name constructor
  /// @{
  
  /// @brief default constructor
  DTreeLeaf(void);
  
  /// @brief constructor taking prediction value
  ///
  /// @param val new prediction value
  DTreeLeaf(ATTVAL val);
  /// @}
  
  /// @name destructor
  /// @{

  /// @brief default destructor
  ~DTreeLeaf(void);
  /// @}

  virtual void for_polymorphism(void) {};
  
  /// @name Informative Attributes
  /// @{
  
  /// @brief getting prediction value
  ///
  /// @retval target attr prediction value
  ATTVAL getVal(void) const;
  /// @}
  
  /// @name Functional Attributes
  /// @{
  
  /// @brief setting prediction value
  ///
  /// @param val new prediction value
  void setVal(ATTVAL val);
  /// @}
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Decision Tree Class
/// @details DTree is consisting with DTreeNodes. It supports minimal tree @n
///          feature which need in decision tree algorithm.
///

class DTree {
private:
  DTreeNode *root;                  ///< Decision Tree Root
  ATTVAL    *att_size_array;        ///< informs how many each attr can have values
  ATTINDEX  nr_att;                 ///< the number of attributes including target attr 

public:
  /// @name constructor
  /// @{

  /// @brief default constructor
  DTree(void);

  /// @brief constructor taking all values
  ///
  /// @param root decision tree root node
  /// @param att_size_array attribute size array
  /// @param nr_Att the number of attr
  DTree(DTreeNode *root, ATTVAL *att_size_array, ATTINDEX nr_att);
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

  /// @brief getting att size array
  ///
  /// @retval att_size_array
  ATTVAL* getAttSizeArray(void) const;

  /// @brief getting the number of attr
  ///
  /// @retval nr_att
  ATTINDEX getNrAtt(void) const;
  
  /// @brief getting leaf node from path array
  ///
  /// @param attArray array of attr path
  /// @retval child decision tree leaf node
  /// @retval NULL if fail to find
  DTreeLeaf* getLeaf(ATTVAL* attArray) const;
  /// @}
  
  /// @name functional attributes
  /// @{

  /// @brief setting tree root
  ///
  /// @param root tree root
  void setRoot(DTreeNode* root);

  /// @brief setting att_size_array
  ///
  /// @param att_size_array new array
  void setAttSizeArray(ATTVAL *att_size_array);
  
  /// @brief setting nr_att
  ///
  /// @param nr_att the number of attr
  void setNrAtt(ATTINDEX nr_att);
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
  ATTINDEX nr_att;      ///< the number of attributes including target attribute
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
  ATTVAL** makeNewExam(ATTVAL **src, int nr_src, ATTINDEX att, ATTVAL val, int &nr_dst);
  
  /// @brief recursively making tree function
  ///
  /// @param parent current parent DTreeRoot
  /// @param cur current DTreeNode
  /// @param remainAtt current remain attribute flags array (except for target attr)
  /// @param cur_ex current test examples
  /// @param nr_cur_ex the number of current test examples
  void recursive_make_tree(DTreeRoot *parent, int ch_index, DTree *tree,
    bool *remainAtt, ATTVAL **cur_ex, int nr_cur_ex, int indent, bool first, bool last); 
  
  /// @brief recursively pruning tree function
  ///
  /// @param cur current DTreeNode
  /// @param path attribution combination (used for countInstance())
  /// @param valid validation examples set
  /// @param nr_valid the number of validation set
  void recursive_prune_tree(DTreeNode *cur, ATTVAL *path, ATTVAL **valid, int nr_valid);
  /// @}

public:
  /// @name constructor
  /// @{

  /// @brief constructor taking initial values
  ///
  /// @param attSizes attributes sizes array
  /// @param nr_att the number of attributes
  /// @param train training data array ( n x nr_train )
  /// @param valid validation data set
  /// @param nr_train the number of training data
  /// @param nr_valid the number of validataion data
  ID3(ATTVAL *attSizes, ATTINDEX nr_att, ATTVAL **train, 
      ATTVAL **valid, int nr_train, int nr_valid);
  /// @}

  /// @name destructor
  /// @{

  /// @brief default destructor
  ~ID3(void);
  /// @}

  /// @name functional attributes
  /// @{
  
  /// @brief making decision tree
  void makeTree(void);

  /// @brief reduced error tree pruning
  void prune(void); 

  /// @brief predict for new unknown instance
  ///
  /// @param input new unknown input
  /// @retval target attribute value predicted
  ATTVAL predict(ATTVAL *input);
  /// @}
};

#endif /* __DECISIONTREE__ */
