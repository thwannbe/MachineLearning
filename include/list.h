////////////////////////////////////////////////////////////////////////////////
/// 
/// @file list.h
/// @brief List Class Library
/// @details Due to hold templates, it cannot be created with separate @n
///          cpp codes, but merged with cpp source code. @n
///          This file is both header file and source file. @n
///          The way to use List is just including this file only.
/// @author Suwon Oh <suwon@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2014/12/15 Suwon Oh created @n
/// 2014/12/15 Suwon Oh implemented prototype @n
/// 2014/12/16 Suwon Oh merged with List.cpp due to template @n
/// 2014/12/16 Suwon Oh adapted to Doxygen @n
/// 
/// @section purpose_section Purpose
/// Personal studying practice for implementing template class
///

#ifndef __LIST_CLASS__
#define __LIST_CLASS__

#ifndef NULL
#define NULL 0
#endif  /* NULL */

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief Cyclic Double-Linked List Node Class with Template
/// @details Using template, it supports every datatype as its node entry. @n
///          Starting from head, all node is connecting with each two pointers; @n
///          prev and next. the last node points a next node as the head.
///

template <typename T>
class ListNode
{
private:
  ListNode <T> *prev;     ///< previous list node
  ListNode <T> *next;     ///< next list node
  T content;              ///< content entry

public:
  /// @name constructors
  /// @{

  /// @brief default constructor
  /// @details Defined for creating List head node which has empty content
  ListNode(void)
  {
    prev = NULL;
    next = NULL;
  }

  /// @brief constructor having prev, next pointer and content
  /// 
  /// @details Defined for creating normal node which would be added
  /// @param prev node pointer which linked front of
  /// @param next node pointer which linked behind
  /// @param content ListNode content
  ListNode(ListNode<T> *prev, ListNode<T> *next, T content) : content(content)
  {
    this->prev = prev;
    this->next = next;
  }
  /// @}
  
  /// @name destructor
  /// @{
  
  /// @brief default destructor
  /// @details There is no way to figure out type T is pointer type. @n
  ///          Therefore, ignore it.
  ~ListNode(void)
  {
    //if (dynamic_cast<void*>(content))
    //  delete(content);
  }
  /// @}

  /// @name informative attributes
  /// @{

  /// @brief getting previous node
  /// 
  /// @retval previous node
  ListNode <T>* getPrev(void) const
  {
    return prev;
  }

  /// @brief getting next node
  ///
  /// @retval next node
  ListNode <T>* getNext(void) const
  {
    return next;
  }

  /// @brief getting content
  ///
  /// @retval content with given template type
  T getContent(void) const
  {
    return content;
  }
  /// @}
  
  /// @name setting attributes
  /// @{

  /// @brief setting previous node
  ///
  /// @param prev node pointer which will be linked front of
  /// @retval true if success, false if prev is NULL
  bool setPrev(ListNode<T> *prev)
  {
    if (prev) {
      this->prev = prev;
      return true;
    }
    return false;
  }

  /// @brief setting next node
  ///
  /// @param next node pointer which will be linked at next
  /// @retval true if success, false if next is NULL
  bool setNext(ListNode<T> *next)
  {
    if (next) {
      this->next = next;
      return true;
    }
    return false;
  }

  /// @brief setting content
  ///
  /// @param content content which will be contained in this node
  /// @retval always true
  bool setContent(T content)
  {
    //if (dynamic_cast<void*>(content))
    //  if (!content)
    //    return false;
    this->content = content;
    return true;
  }
  /// @}
};

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief List Class which node is ListNode Class type
/// @details Container of ListNode Class, is constructing with multiple nodes. @n
///          Automatically, enabling to find appropriate index and to add @n
///          appropriate position (back), also supporting delete operation
///

template <typename T>
class List
{
private:
  ListNode <T> *head;         ///< List head list node
  unsigned int size;          ///< the number of total nodes

public:
  /// @name constructors
  /// @{

  /// @brief default constructor
  List(void)
  {
    if (head = new ListNode<T>()) {
      // make List cyclic
      head->setPrev(head);
      head->setNext(head);
    }
    size = 0;
  }
  /// @}

  /// @name destructors
  /// @{

  /// @brief default destructor
  ///
  /// @details find all nodes, and deletes all
  ~List(void)
  {
    if (head) {
      ListNode <T>* cur = head->getNext();
      while (cur != head) {
        ListNode <T>* next = cur->getNext();
        delete(cur);  // call ListNode destructor
        cur = next;
      }
      // only left head
      delete(head);
    }
  }
  /// @}
  
  /// @name informative attributes
  /// @{

  /// @brief return list head node
  ///
  /// @retval ListNode pointer
  ListNode <T>* getHead(void) const
  {
    return head;
  }

  /// @brief return node which have a given index
  ///
  /// @retval ListNode pointer
  ListNode <T>* getNode(unsigned int index) const
  {
    // index check
    if (index >= size || size == 0)
      return NULL;
    
    // first non-head node
    ListNode <T>* cur = head->getNext();
    for (unsigned int i = 0; i < index; i++)
      cur = cur->getNext();

    return cur;
  }
  
  /// @brief return content which is contained given index node
  ///
  /// @retval template T type
  T getContent(unsigned int index) const
  {
    // index check
    if (index >= size || size == 0)
      return NULL;

    return getNode(index)->getContent();
  }
  
  /// @brief return the number of nodes which the list has
  ///
  /// @retval unsigned integer
  unsigned int getSize(void) const
  {
    return size;
  }
  /// @}
  
  /// @name functional attributes
  /// @{

  /// @brief add new list node which has a given content
  ///
  /// @param content content which will be contained
  /// @retval true if success, false if fail
  bool addNode(T content)
  {
    ListNode <T>* last = NULL;
    // choose last node
    if (size == 0)
      last = getHead();
    else
      last = getNode(size - 1);
    
    if (ListNode <T>* newNode = new ListNode <T>(last, head, content)) {
      // link new node
      last->setNext(newNode);
      head->setPrev(newNode);
      size++; // increase size
      return true;
    }
    return false;
  }

  /// @brief delete target list node which has a given index
  ///
  /// @param index list node index which will be deleted
  /// @retval true if success, false if fail
  bool delNode(unsigned int index)
  {
    // index check
    if (index >= size || size == 0)
      return false;

    if (ListNode <T>* target = getNode(index)) {
      // unlink
      target->getPrev()->setNext(target->getNext());
      target->getNext()->setPrev(target->getPrev());
      delete(target);
      size--;
      return true;
    }
    return false;
  }
  
  /// @brief set content to target list node
  ///
  /// @param index list node index whose content will be set
  /// @param content content which will be replaced
  /// @retval true if success, false if fail
  bool setContent(unsigned int index, T content)
  {
    // index check
    if (index >= size || size == 0)
      return false;

    if (getNode(index)->setContent(content))
      return true;
    return false;
  }
  /// @}
};

#endif  /* __LIST_CLASS__ */
