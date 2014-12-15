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

#ifdef  __LIST_CLASS__
#define __LIST_CLASS__

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief List Class
/// 
/// Double-Linked List which can hold various type content.
///
////////////////////////////////////////////////////////////////////////////////

template <typename T>
class List
{
private:
  /// @name prev/next
  /// @{
  /// @
  List *prev, *next;
  T content;
  unsigned int index;
  /// @}
}



#endif  /* __LIST_CLASS__ */
