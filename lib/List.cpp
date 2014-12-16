////////////////////////////////////////////////////////////////////////////////
///
/// @brief List Class Library
/// @author Suwon Oh <suwon@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2014/12/15 Suwon Oh created
/// 2014/12/15 Suwon Oh implemented prototype
/// 2014/12/15 Suwon Oh adapted to Doxygen
/// 
/// @section purpose_section Purpose
/// Personal studying practice for implementing template class
///
////////////////////////////////////////////////////////////////////////////////

#include "List.h"

#ifndef NULL
#define NULL 0
#endif  /* NULL */

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief ListNode Class
/// 
/// Double-Linked ListNode which can hold various type content.
///
////////////////////////////////////////////////////////////////////////////////

template <typename T>
ListNode<T>::ListNode()
{
  prev = NULL;
  next = NULL;
}

template <typename T>
ListNode<T>::ListNode(T content) : content(content)
{
  prev = NULL;
  next = NULL;
}

template <typename T>
ListNode<T>::ListNode(ListNode<T> *prev, ListNode<T> *next, T content) : content(content)
{
  this->prev = prev;
  this->next = next;
}

template <typename T>
ListNode<T>::~ListNode()
{
  //if (dynamic_cast<void*>(content))
  //  delete(content);
}

template <typename T>
ListNode <T>* ListNode<T>::getPrev() const
{
  return prev;
}

template <typename T>
ListNode <T>* ListNode<T>::getNext() const
{
  return next;
}

template <typename T>
T ListNode<T>::getContent() const
{
  return content;
}

template <typename T>
bool ListNode<T>::setPrev(ListNode<T> *prev)
{
  if (prev) {
    this->prev = prev;
    return true;
  }
  return false;
}

template <typename T>
bool ListNode<T>::setNext(ListNode<T> *next)
{
  if (next) {
    this->next = next;
    return true;
  }
  return false;
}

template <typename T>
bool ListNode<T>::setContent(T content)
{
  //if (dynamic_cast<void*>(content))
  //  if (!content)
  //    return false;

  this->content = content;
  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief List Class
/// 
/// Double-Linked List which can hold various type content.
///
////////////////////////////////////////////////////////////////////////////////

template <typename T>
List<T>::List()
{
  if (head = new ListNode<T>())
    head->setPrev(head);
    head->setNext(head);
  size = 0;
}

template <typename T>
List<T>::~List()
{
  if (head) {
    ListNode <T>* cur = head->getNext();
    while (cur != head) {
      ListNode <T>* next = cur->getNext();
      delete(cur);
      cur = next;
    }
    delete(head);
  }
}

template <typename T>
ListNode <T>* List<T>::getHead() const
{
  return head;
}

template <typename T>
ListNode <T>* List<T>::getNode(unsigned int index) const
{
  if (index >= size || size == 0)
    return NULL;
  
  ListNode <T>* cur = head->getNext();
  for (unsigned int i = 0; i < index; i++)
    cur = cur->getNext();
  
  return cur;
}

template <typename T>
T List<T>::getContent(unsigned int index) const
{
  if (index >= size || size == 0)
    return NULL;

  return getNode(index)->getContent();
}

template <typename T>
unsigned int List<T>::getSize(void) const
{
  return size;
}

template <typename T>
bool List<T>::addNode(T content)
{
  ListNode <T>* last = NULL;
  if (size == 0)
    last = getHead();
  else
    last = getNode(size - 1);

  if (ListNode <T>* newNode = new ListNode <T>(last, head, content)) {
    last->setNext(newNode);
    head->setPrev(newNode);
    size++;
    return true;
  }
  return false;
}

template <typename T>
bool List<T>::delNode(unsigned int index)
{
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

template <typename T>
bool List<T>::setContent(unsigned int index, T content)
{
  if (index >= size || size == 0)
    return false;
  
  if (getNode(index)->setContent(content))
    return true;
  return false;
}
