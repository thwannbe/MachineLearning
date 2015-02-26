////////////////////////////////////////////////////////////////////////////////
///
/// @file decision.cpp
/// @brief Decision Tree Learning Framework Source File
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

#include "decision.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

#define DEBUG_MODE  1

using namespace std;

////////////////////////////////////////////////////////////////////////////////
///
/// self-implemented assert function
///

static inline void ASSERT(bool exp, char* message = NULL) {
  if (!exp) {
    cout << "ASSERT" << endl;
    if (message)
      cout << "ERROR: " << message << endl;
    exit(-1);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// DTreeNode class implementation
///

DTreeNode::DTreeNode() {
  parent = NULL;
}

DTreeNode::DTreeNode(DTreeRoot *parent) {
  ASSERT (parent != NULL);
  this->parent = parent;
}

DTreeNode::~DTreeNode() {
}

DTreeRoot* DTreeNode::getParent() const {
  return parent;
}

void DTreeNode::setParent(DTreeRoot *parent) {
  // if parent = NULL, this node is root
  this->parent = parent;
}

////////////////////////////////////////////////////////////////////////////////
///
/// DTreeRoot class implementation
///

DTreeRoot::DTreeRoot() {
  attIndex = -1; // no index
  child = NULL;
  nr_child = 0; // no child
}

DTreeRoot::DTreeRoot(ATTINDEX attIndex, DTreeNode **child, ATTVAL nr_child) {
  ASSERT (child != NULL && attIndex >= 0 && nr_child > 0);
  this->attIndex = attIndex;
  this->child = child;
  this->nr_child = nr_child;
}

DTreeRoot::~DTreeRoot() {
  if (child && nr_child != 0) {
    for (int i = 0; i < nr_child; i++)
      delete (child[i]);
    delete (child);
  }
}

ATTINDEX DTreeRoot::getAttIndex() const {
  return attIndex;
}

DTreeNode** DTreeRoot::getChildArray() const {
  return child;
}

DTreeNode* DTreeRoot::getOneChild(ATTVAL index) const {
  ASSERT (child != NULL && index < nr_child);
  return child[index]; 
}

ATTVAL DTreeRoot::getNrChild() const {
  return nr_child;
}

void DTreeRoot::setAttIndex(ATTINDEX attIndex) {
  ASSERT (attIndex >= 0);
  this->attIndex = attIndex;
}

void DTreeRoot::setChildArray(DTreeNode **child) {
  ASSERT (child != NULL);
  this->child = child;
}

void DTreeRoot::setOneChild(DTreeNode *node, ATTVAL index) {
  ASSERT (child != NULL && node != NULL && index < nr_child);
  child[index] = node;
}

void DTreeRoot::setNrChild(ATTVAL nr_child) {
  ASSERT (nr_child > 0);
  this->nr_child = nr_child;
}

////////////////////////////////////////////////////////////////////////////////
///
/// DTreeLeaf class implementation
///

DTreeLeaf::DTreeLeaf() {
  val = -1; // no value
}

DTreeLeaf::DTreeLeaf(ATTVAL val) {
  ASSERT (val >= 0);
  this->val = val;
}

DTreeLeaf::~DTreeLeaf() {
}

ATTVAL DTreeLeaf::getVal() const {
  return val;
}

void DTreeLeaf::setVal(ATTVAL val) {
  ASSERT (val >= 0);
  this->val = val;
}

////////////////////////////////////////////////////////////////////////////////
///
/// DTree class implementation
///

DTree::DTree() {
  root = NULL;
  att_size_array = NULL;
  nr_att = 0;
}

DTree::DTree(DTreeNode *root, ATTVAL *att_size_array, ATTINDEX nr_att) {
  ASSERT (root != NULL && att_size_array != NULL && nr_att > 1);
  this->root = root;
  this->att_size_array = att_size_array;
  this->nr_att = nr_att;
}

DTree::~DTree() {
  if (root)
    delete (root);
  if (att_size_array)
    delete (att_size_array);
}

DTreeNode* DTree::getRoot() const {
  return root;
}

ATTVAL* DTree::getAttSizeArray() const {
  return att_size_array;
}

ATTINDEX DTree::getNrAtt() const {
  return nr_att;
}

DTreeLeaf* DTree::getLeaf(ATTVAL* attArray) const {
  ASSERT (attArray != NULL && root != NULL, (char*) "attArray or root is NULL");
  
  DTreeNode *cur = root;

  while(true) {
    // get leaf
    if (DTreeLeaf *edge = dynamic_cast <DTreeLeaf*> (cur))
      return edge;
    
    // if not leaf, get child and search again
    DTreeRoot *link = dynamic_cast <DTreeRoot*> (cur);
    ASSERT (link != NULL, (char*) "link is not valid");
    ASSERT (link->getChildArray() != NULL, (char*) "child array doesn't exist");
    ASSERT (link->getAttIndex() >= 0 && link->getAttIndex() < nr_att,
      (char*) "attIndex is not valid");
    ASSERT (attArray[link->getAttIndex()] >= 0 &&
      attArray[link->getAttIndex()] < link->getNrChild(),
      (char*) "attArray value is not valid");
    ASSERT (link->getOneChild(attArray[link->getAttIndex()]) != NULL,
      (char*) "child doesn't exist");
    cur = link->getOneChild(attArray[link->getAttIndex()]);
  }
}

void DTree::setRoot(DTreeNode* root) {
  ASSERT (root != NULL);
  this->root = root;
}

void DTree::setAttSizeArray(ATTVAL *att_size_array) {
  ASSERT (att_size_array != NULL);
  this->att_size_array = att_size_array;
}

void DTree::setNrAtt(ATTINDEX nr_att) {
  this->nr_att = nr_att;
}

////////////////////////////////////////////////////////////////////////////////
///
/// ID3 algorithm class implementation
///

ID3::ID3(ATTVAL *attSizes, ATTINDEX nr_att, ATTVAL **train, ATTVAL **valid, int nr_train, int nr_valid) {
  ASSERT (attSizes != NULL && nr_att > 1 && train != NULL &&
          valid != NULL && nr_train > 0 && nr_valid > 0);
  dtree = new DTree();
  this->attSizes = attSizes;
  dtree->setAttSizeArray(attSizes);
  this->nr_att = nr_att;
  dtree->setNrAtt(nr_att);
  this->train = train;
  this->valid = valid;
  this->nr_train = nr_train;
  this->nr_valid = nr_valid;
}

ID3::~ID3() {
  if (dtree)
    delete (dtree);

  if (attSizes)
    delete (attSizes);

  if (train)
    for (int i = 0; i < nr_train; i++)
      if (train[i])
        delete (train[i]);
  delete (train);
  
  if (valid)
    for (int i = 0; i < nr_valid; i++)
      if (valid[i])
        delete (valid[i]);
  delete (valid);
}

void ID3::makeTree() {
  bool *remainAtt = new bool[nr_att - 1]; // except for target attr
  for (int i = 0; i < nr_att; i++)
    remainAtt[i] = true; // means reamin
  recursive_make_tree(NULL, 0, dtree, remainAtt, train, nr_train, 0, false, false); 
}

void ID3::prune() {
  if (dynamic_cast <DTreeLeaf*> (dtree->getRoot()))
    return;
  
  ATTVAL *empty_path = new ATTVAL[nr_att];
  for (int i = 0; i < nr_att; i++)
    empty_path[i] = -1; // means empty (no index)
  recursive_prune_tree(dtree->getRoot(), empty_path, valid, nr_valid);
}

ATTVAL ID3::predict(ATTVAL *input) {
  // it supposed that makeTree is already executed!
  if (!dtree)
    return -1; // error
  
  return dtree->getLeaf(input)->getVal();
}


// check if data satisfy target array
// ATTVAL -1 means unconditionally satisfaction
static bool satisfaction(ATTVAL *dat, ATTVAL *target, ATTINDEX nr_att) {
  ASSERT(dat != NULL && target != NULL && nr_att > 0,
    (char*) "satisfaction condition error");

  bool ret = true;
  for (int i = 0; i < nr_att; i++)
    if (target[i] != -1 && dat[i] != target[i])
      ret = false;

  return ret;
}

int ID3::countInstance(ATTVAL *att_array, ATTVAL **examples, int nr_ex) {
  ASSERT(att_array != NULL && examples != NULL, 
    (char*) "att_array or example is NULL");

  int count = 0;
  for (int i = 0; i < nr_ex; i++)
    if (satisfaction(examples[i], att_array, nr_att))
      count++;
  
  return count;
}

double ID3::calEntropy(ATTVAL **examples, int nr_ex) {
  if (examples == NULL)
    return 0;

  double entropy = 0;
  
  ATTVAL *tmp_array = new ATTVAL[nr_att];
  for (ATTINDEX i = 0; i < nr_att; i++)
    tmp_array[i] = -1;
  for (ATTVAL i = 0; i < attSizes[nr_att-1]; i++) {
    tmp_array[nr_att-1] = i;
    
    double p = ((double)countInstance(tmp_array, examples, nr_ex)) / ((double)nr_ex);
    entropy += -p * log(p) / log((double)2);
  }
  //delete (tmp_array);

  return entropy;
}

double ID3::calGain(ATTINDEX att, ATTVAL **examples, int nr_ex) {
  ASSERT(att >= 0 && att < nr_att - 1 && examples != NULL && nr_ex > 0,
    (char*) "calGain condition error");
  
  double gain = 0;
  
  gain += calEntropy(examples, nr_ex);

  for (ATTVAL i = 0; i < attSizes[att]; i++) {
    int nr_tmp_exam;
    ATTVAL **tmp_exam = makeNewExam(examples, nr_ex, att, i, nr_tmp_exam);
    gain -= ((double)nr_tmp_exam / (double)nr_ex) * calEntropy(tmp_exam, nr_tmp_exam);
  }
  
  return gain;  
}

ATTINDEX ID3::takeBestAtt(bool *remainAtt, ATTVAL **examples, int nr_ex) {
  ASSERT(remainAtt != NULL && examples != NULL && nr_ex > 0,
    (char*) "takeBestAtt condition error");

  ATTINDEX best = -1;
  double max_gain;

  for (ATTINDEX i = 0; i < nr_att - 1; i++) {
    if (remainAtt[i] == true) {
      if (best == -1) {
        best = i;
        max_gain = calGain(i, examples, nr_ex);
        continue;
      }
      
      double curGain = calGain(i, examples, nr_ex);
      if (curGain > max_gain) {
        max_gain = curGain;
        best = i;
      }
    }
  }
  
  return best;
}

ATTVAL** ID3::makeNewExam(ATTVAL **src, int nr_src, ATTINDEX att, ATTVAL val, 
    int &nr_dst) {
  ASSERT(src != NULL && nr_src > 0 && att >= 0 && att < nr_att &&
    val >= 0 && val < attSizes[att], (char*) "makeNewExam condition error");
  
  ATTVAL **ret;
  // first, count wanted examples to allocate destination array mem
  int cnt_target = 0;
  for (int i = 0; i < nr_src; i++)
    if (src[i][att] == val)
      cnt_target++;
  
  nr_dst = cnt_target;
  if (cnt_target == 0)
    return NULL;

  // second, allocate destination array and fill it
  ret = new ATTVAL*[cnt_target];
  int index_dst = 0;
  for (int i = 0; i < nr_src; i++)
    if (src[i][att] == val)
      ret[index_dst++] = src[i];
 
  return ret;
}

static void print_tree_entry(int indent, bool first, bool last, int val, bool leaf)
{
  char lBrac = (leaf) ? '{' : '(';
  char rBrac = (leaf) ? '}' : ')';
  if (indent != 0) {
    if (first) {
      cout << " - ";
      cout << lBrac << val << rBrac;
    } else {
      cout << endl << "///\t   ";
      for (int i = 0; i < indent - 1; i++)
        cout << "      ";
      if (last)
        cout << " \\ ";
      else
        cout << " + ";
      cout << lBrac << val << rBrac;
    }
  } else {
    cout << "///\t" << lBrac << val << rBrac;
  }
}

void ID3::recursive_make_tree(DTreeRoot *parent, int ch_index, DTree *tree, 
    bool *remainAtt, ATTVAL **cur_ex, int nr_cur_ex, int indent, bool first,
    bool last) {
  // step 0. condition check
  ASSERT (remainAtt != NULL, (char*) "remainAtt is NULL");
  if (cur_ex != NULL && nr_cur_ex > 0)
    for (int i = 0; i < nr_cur_ex; i++)
      ASSERT (cur_ex[i] != NULL, (char*) "cur_ex entry is NULL");
  
  if (cur_ex)
  {
    // step 1. check if all train data have same value => make leaf
    bool same = true;
    // first train value (last attribute is target attr)
    ATTVAL same_val = cur_ex[0][nr_att-1];     
    for (int i = 0; i < nr_cur_ex; i++) {
      if (cur_ex[i][nr_att-1] != same_val) {
        same = false;
        break;
      }
    }

    if (same) {
      if (!parent) { // Root node
        if (tree) {
          dtree->setRoot(new DTreeLeaf(same_val));
          print_tree_entry(indent, first, last, same_val, true);
          return;
        } else { // error
          cout << "ERROR: Root node must have tree info" << endl;
          exit(1);
        }
      } else { // leaf node
        if (ch_index >= 0 && ch_index <= attSizes[parent->getAttIndex()]) {
          parent->setOneChild(new DTreeLeaf(same_val), ch_index);
          print_tree_entry(indent, first, last, same_val, true);
          return;
        } else {
          cout << "ERROR: ch_index is invalid" << endl;
          exit(1);
        }
      }
    }

    // step 2. check if att num is 0 except for target attr => make leaf
    int nr_remain_att = 0;
    for (int i = 0; i < nr_att - 1; i++)
      if (remainAtt[i])
        nr_remain_att++;

    if (nr_remain_att == 0) { // only attr is target
      ATTVAL *att_array = new ATTVAL[nr_att];
      for (int i = 0; i < nr_att - 1; i++)
        att_array[i] = -1;
      int maxCnt = 0;
      ATTVAL maxVal = -1;
      for (int i = 0; i < attSizes[nr_att - 1]; i++) {
        att_array[nr_att - 1] = i;
        int curCnt = countInstance(att_array, cur_ex, nr_cur_ex);
        if (maxCnt < curCnt) {
          maxCnt = curCnt;
          maxVal = i;
        }
      }

      if (!parent) { // Root node
        if (tree) {
          dtree->setRoot(new DTreeLeaf(maxVal));
          print_tree_entry(indent, first, last, maxVal, true);
        } else { // error
          cout << "ERROR: Root node must have tree info" << endl;
          exit(1);
        }
      } else { // leaf node
        if (ch_index >= 0 && ch_index <= attSizes[parent->getAttIndex()]) {
          parent->setOneChild(new DTreeLeaf(maxVal), ch_index);
          print_tree_entry(indent, first, last, maxVal, true);
        } else {
          cout << "ERROR: ch_index is invalid" << endl;
          exit(1);
        }
      }
      return;
    }
  } 
  else // cur_ex = NULL
  {
    // step 3. check if there is no example => make leaf as common val
    if (nr_cur_ex <= 0) {
      ATTVAL *att_array = new ATTVAL[nr_att];
      for (int i = 0; i < nr_att - 1; i++)
        att_array[i] = -1;
      int maxCnt = 0;
      ATTVAL maxVal = -1;
      for (int i = 0; i < attSizes[nr_att - 1]; i++) {
        att_array[nr_att - 1] = i;
        int curCnt = countInstance(att_array, train, nr_train);
        if (maxCnt < curCnt) {
          maxCnt = curCnt;
          maxVal = i;
        }
      }

      if (!parent) { // Root node
        if (tree) {
          dtree->setRoot(new DTreeLeaf(maxVal));
          print_tree_entry(indent, first, last, maxVal, true);
        } else { // error
          cout << "ERROR: Root node must have tree info" << endl;
          exit(1);
        }
      } else { // leaf node
        if (ch_index >= 0 && ch_index <= attSizes[parent->getAttIndex()]) {
          parent->setOneChild(new DTreeLeaf(maxVal), ch_index);
          print_tree_entry(indent, first, last, maxVal, true);
        } else {
          cout << "ERROR: ch_index is invalid" << endl;
          exit(1);
        }
      }
      return;
    } else {
      cout << "ERROR: nr_cur_ex should be 0" << endl;
      exit(1);
    }
  }

  // step 4. making normal child node
  ATTINDEX best = takeBestAtt(remainAtt, cur_ex, nr_cur_ex);
  DTreeRoot *cur = 
    new DTreeRoot(best, new DTreeNode*[attSizes[best]], attSizes[best]);
  
  if (!parent) { // Root node
    if (tree) {
      dtree->setRoot(cur);
      print_tree_entry(indent, first, last, best, false);
    } else { // error
      cout << "ERROR: Root node must have tree info" << endl;
      exit(1);
    }
  } else { // leaf node
    if (ch_index >= 0 && ch_index <= attSizes[parent->getAttIndex()]) {
      parent->setOneChild(cur, ch_index);
      print_tree_entry(indent, first, last, best, false);
    } else {
      cout << "ERROR: ch_index is invalid" << endl;
      exit(1);
    }
  }

  bool *new_remainAtt = new bool[nr_att - 1]; // make remainAtt copy
  for (int i = 0; i < nr_att - 1; i++)
    new_remainAtt[i] = remainAtt[i];
  new_remainAtt[best] = false; // clear current 'best' attr
  
  for (int i = 0; i < attSizes[best]; i++) {
    int nr_new_ex;
    ATTVAL **new_ex = makeNewExam(cur_ex, nr_cur_ex, best, i, nr_new_ex);
    recursive_make_tree(cur, i, NULL, new_remainAtt, new_ex, nr_new_ex, indent + 1, 
    (i == 0) ? true : false, (i == attSizes[best] - 1) ? true : false);
  }
}

#define PRUNE_RATE 80 // the rate for accepting pruning

void ID3::recursive_prune_tree(DTreeNode *cur, ATTVAL *path, ATTVAL **valid, int nr_valid) {
  ASSERT (cur != NULL && path != NULL);
  for (int i = 0; i < nr_valid; i++)
    ASSERT (valid[i] != NULL);
  
  if (valid) {
    ATTVAL *new_path = new ATTVAL[nr_att]; // make path copy
    for (int i = 0; i < nr_att - 1; i++)
      new_path[i] = path[i];

    // count from validation data itself
    int maxCnt = 0;
    ATTVAL maxVal = -1;
    for (int i = 0; i < attSizes[nr_att - 1]; i++) {
      new_path[nr_att - 1] = i;
      int curCnt = countInstance(new_path, valid, nr_valid);
      if (maxCnt < curCnt) {
        maxCnt = curCnt;
        maxVal = i;
      }
    }

    // prune condition
    if (100 * maxCnt / nr_valid > PRUNE_RATE) {
      cur = new DTreeLeaf(maxVal);
      return;
    }

    DTreeRoot *link = dynamic_cast <DTreeRoot*> (cur);
    // if not, recursively search again
    for (int i = 0; i < link->getNrChild(); i++) {
      if (dynamic_cast <DTreeLeaf*> (link->getOneChild(i)))
        continue;

      new_path[link->getAttIndex()] = i;
      int nr_new_val;
      ATTVAL **new_val = makeNewExam(valid, nr_valid, link->getAttIndex(), 
          i, nr_new_val);
      recursive_prune_tree(link->getOneChild(i), new_path, new_val, nr_new_val);
      delete (new_val);
    }

    delete (new_path);
  }
}

