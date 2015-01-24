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
///
/// @section reference_section Reference
/// MACHINE LEARNING - TOM M. MITCHELL
/// 
/// @section purpose_section Purpose
/// Application trial for personal studying
///

#include "decision.h"
#include <iostream>
#include <cmath>

#define DEBUG_MODE  1

using namespace std;

////////////////////////////////////////////////////////////////////////////////
///
/// DTreeNode class implementation
///

DTreeNode::DTreeNode() {  // making empty node
  child = NULL;
  attIndex = -1;
  predict = -1;
  nr_val = 0;
}

DTreeNode::DTreeNode(DTreeNode **array, ATTINDEX index, ATTVAL n) { // making linking node
  child = array;
  attIndex = index;
  predict = -1;
  nr_val = n;
}

DTreeNode::DTreeNode(ATTVAL pred) { // making leaf
  child = NULL;
  attIndex = -1;
  predict = pred;
  nr_val = 0;
}

DTreeNode::~DTreeNode() {
  if (child)
    for (ATTVAL i = 0; i < nr_val; i++)
      if (child[i])
        delete(child[i]);
  delete(child);
}

DTreeNode* DTreeNode::getChild(ATTVAL val) const {
  if (val >= nr_val)
    return NULL;
  
  return child[val];
}

ATTINDEX DTreeNode::getIndex() const {
  return attIndex;
}

ATTVAL DTreeNode::getPred() const {
  return predict;
}

ATTVAL DTreeNode::getNrVal() const {
  return nr_val;
}

bool DTreeNode::isLeaf() const {
  return (child) ? false : true;
}

bool DTreeNode::setChild(DTreeNode* node, ATTVAL val) {
  if (val >= nr_val)
    return false;
  
  if (!child) { // making child
    child = new DTreeNode*[nr_val];
    for (int i = 0; i < nr_val; i++)
      child[i] = NULL;
  }
  
  if (child[val]) // overwrite
    delete(child[val]);
  child[val] = node;
  
  return true;
}

void DTreeNode::setValue(ATTINDEX index, ATTVAL pred, ATTVAL nr) {
  attIndex = index;
  predict = pred;
  nr_val = nr;
}

////////////////////////////////////////////////////////////////////////////////
///
/// DTree class implementation
///

DTree::DTree(int *att_array, int n) {
  root = NULL;
  att_size_array = att_array;
  nr_Att = n;
}

DTree::~DTree() {
  if (root)
    delete(root);
}

DTreeNode* DTree::getRoot() const {
  return root;
}

DTreeNode* DTree::getLeaf(ATTVAL* attArray) const {
  if (!root)
    return NULL;
  
  DTreeNode *cur = root;
  while(true) {
    if (cur->isLeaf())
      return cur;

    if (cur->getIndex() != -1) {
      if (attArray[cur->getIndex()] < cur->getNrVal())
        cur = cur->getChild(attArray[cur->getIndex()]);
      else
        return NULL;  // att value should be in range of nr_val
    }
    else {
      return NULL; // non-leaf should have attIndex
    }    
  }
}

void DTree::setRoot(DTreeNode* root) {
  this->root = root;
}

////////////////////////////////////////////////////////////////////////////////
///
/// ID3 algorithm class implementation
///

ID3::ID3(ATTVAL *sizes, ATTINDEX n, ATTVAL **train, int nr_train, ATTVAL **valid, int nr_valid) {
  attSizes = sizes;
  nr_Att = n;
  dtree = new DTree(sizes, n);
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

bool ID3::makeTree(DTreeNode *current, bool *remainAtt, ATTVAL **examples, int nr_ex) {
  bool ret;

  // step 0. condition check
  if (!dtree || !attSizes || !train || nr_train <= 0)
    return false;
  
  for (int i = 0; i < nr_ex; i++)
    if (!examples[i])
      return false;

  // step 1. check if all train data have same value
  bool same = true;
  ATTVAL val = examples[0][nr_Att-1]; // first train value (last attribute is target attr)
  for (int i = 0; i < nr_ex; i++) {
    if (examples[i][nr_Att-1] != val) {
      same = false;
      break;
    }
  }
  
  if (same) {
    if (!current) {  // only for making root
      DTreeNode *root = new DTreeNode(val);
      dtree->setRoot(root);
      return true;
    } else {
      current->setValue(-1, val, 0);
      return true;
    }
  }

  // step 2. check if att num is 0 except for target attr
  if (!remainAtt) { // only for making root
    if (current)
      return false;

    if (nr_Att == 1) { // only attr is target
      ATTVAL *att_array = new ATTVAL[nr_Att]; // nr_Att must be 1
      int maxCnt = 0;
      ATTVAL maxVal = -1;
      for (ATTVAL i = 0; i < attSizes[nr_Att-1]; i++) {
        att_array[nr_Att-1] = i;
        int curCnt = countInstance(att_array, train, nr_train);
        if (maxCnt < curCnt) {
          maxCnt = curCnt;
          maxVal = i;
        }
      }

      if (maxVal != -1) {
        DTreeNode *root = new DTreeNode(maxVal);
        dtree->setRoot(root);
        return true;
      }
      else {
        return false;
      }
    }
  }
  else {
    if (!current)
      return false;

    ATTINDEX first_remain_att;
    for (ATTINDEX i = 0; i < nr_Att; i++) {
      if (remainAtt[i]) {
        first_remain_att = i;
        break;
      }
    }
    
    if (first_remain_att == nr_Att - 1) { // nr_Att - 1 is target attr
      ATTVAL *att_array = new ATTVAL[nr_Att];
      for (ATTINDEX i = 0; i < nr_Att - 1; i++)
        att_array[i] = -1;
      int maxCnt = 0;
      ATTVAL maxVal = -1;
      for (ATTVAL i = 0; i < attSizes[nr_Att-1]; i++) {
        att_array[nr_Att-1] = i;
        int curCnt = countInstance(att_array, examples, nr_ex);
        if (maxCnt < curCnt) {
          maxCnt = curCnt;
          maxVal = i;
        }
      }

      if (maxVal != -1) {
        current->setValue(-1, maxVal, 0);
        return true;
      }
      else {
        return false;
      }
    }
  }
  
  // step 3. check if there is any examples
  if (!examples && current) { // no example else when not root
    if (nr_ex > 0)
      return false;
    
    ATTVAL *att_array = new ATTVAL[nr_Att];
    for (ATTINDEX i = 0; i < nr_Att - 1; i++)
      att_array[i] = -1;
    int maxCnt = 0;
    ATTVAL maxVal = -1;
    for (ATTVAL i = 0; i < attSizes[nr_Att-1]; i++) {
      att_array[nr_Att-1] = i;
      int curCnt = countInstance(att_array, train, nr_train);
      if (maxCnt < curCnt) {
        maxCnt = curCnt;
        maxVal = i;
      }
    }

    if (maxVal != -1) {
      current->setValue(-1, maxVal, 0);
      return true;
    }
    else {
      return false;
    }
  }

  // step 4. making normal child node
  ATTINDEX best;
  if (examples)
    best = takeBestAtt(remainAtt, examples, nr_ex);
  else
    best = takeBestAtt(remainAtt, train, nr_train);

  
  if (!current) { // only for root
    DTreeNode *root = new DTreeNode();
    dtree->setRoot(root);
    current = root;
  }

  // prepare remainAtt array
  bool *new_remainAtt = new bool[nr_Att];
  if (!remainAtt) { // only for root
    for (ATTINDEX j = 0; j < nr_Att; j++)
      new_remainAtt[j] = true;
  }
  else {
    for (ATTINDEX j = 0; j < nr_Att; j++)
      new_remainAtt[j] = remainAtt[j];
  }

  new_remainAtt[best] = false;

  // write current feature
  current->setValue(best, -1, attSizes[best]);
  for (ATTVAL i = 0; i < attSizes[best]; i++) {
    DTreeNode *tmp = new DTreeNode();
    if(!current->setChild(tmp, i))
      return false;
    
    ATTVAL **new_examples = NULL;
    int new_nr_ex;
    if (examples)
      makeNewExam(examples, nr_ex, best, i, new_examples);
    else
      makeNewExam(train, nr_train, best, i, new_examples);

    makeTree(current->getChild(i), new_remainAtt, new_examples, new_nr_ex);
  }

  return true;
}

bool ID3::prune() {
  return true;
}

ATTVAL ID3::predict(ATTVAL *input) {
  // it supposed that makeTree is already executed!
  if (!dtree)
    return -1; // error
  
  return dtree->getLeaf(input)->getPred();
}


// check if data satisfy target array
// ATTVAL -1 means unconditionally satisfaction
static bool satisfaction(ATTVAL *dat, ATTVAL *target, ATTINDEX nr_att) {
  bool ret = true;
  
  for (int i = 0; i < nr_att; i++)
    if (dat[i] != target[i] && target[i] != -1)
      ret = false;

  return ret;
}

int ID3::countInstance(ATTVAL *att_array, ATTVAL **examples, int nr_ex) {
  int count = 0;
  
  for (int i = 0; i < nr_ex; i++)
    if (satisfaction(examples[i], att_array, nr_Att))
      count++;

  return count;
}

double ID3::calEntropy(ATTVAL **examples, int nr_ex) {
  double entropy = 0;
  
  ATTVAL *tmp_array = new ATTVAL[nr_Att];
  for (ATTINDEX i = 0; i < nr_Att; i++)
    tmp_array[i] = -1;
  for (ATTVAL i = 0; i < attSizes[nr_Att-1]; i++) {
    tmp_array[nr_Att-1] = i;
    
    double p = ((double)countInstance(tmp_array, examples, nr_ex)) / ((double)nr_ex);
    entropy += -p * log(p) / log((double)2);
  }

  return entropy;
}

double ID3::calGain(ATTINDEX att, ATTVAL **examples, int nr_ex) {
  double gain = 0;
  
  gain += calEntropy(examples, nr_ex);

  for (ATTVAL i = 0; i < attSizes[att]; i++) {
    ATTVAL **tmp_exam = NULL;
    int nr_tmp_exam = makeNewExam(examples, nr_ex, att, i, tmp_exam);
    
    gain -= ((double)nr_tmp_exam / (double)nr_ex) * calEntropy(tmp_exam, nr_tmp_exam);

    delete (tmp_exam);
  }
  
  return gain;  
}

ATTINDEX ID3::takeBestAtt(bool *remainAtt, ATTVAL **examples, int nr_ex) {
  ATTINDEX best = -1;
  double max_gain;

  for (ATTINDEX i = 0; i < nr_Att; i++) {
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

int ID3::makeNewExam(ATTVAL **src, int nr_src, ATTINDEX att, ATTVAL val, ATTVAL **dst) {
  // first, count wanted examples to allocate destination array mem
  int cnt_target = 0;
  for (int i = 0; i < nr_src; i++)
    if (src[i][att] == val)
      cnt_target++;
  
  if (cnt_target == 0)
    return 0;
  
  // second, allocate destination array and fill it
  dst = new ATTVAL*[cnt_target];
  int index_dst = 0;
  for (int i = 0; i < nr_src; i++)
    if (src[i][att] == val)
      dst[index_dst++] = src[i];
  
  return cnt_target;
}

