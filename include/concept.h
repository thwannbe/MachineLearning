////////////////////////////////////////////////////////////////////////////////
///
/// @file concept.h
/// @brief Concept Learning Framework Header File
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

#ifndef __CONCEPTLEARNING__
#define __CONCEPTLEARNING__

#include "list.h"

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Possible Attribute Values
/// @details Each attributes not only can have true or false, but also can have @n
///          AllAccept ( which has symbol as '?' ) and AllDeny ( which has symbol @n
///          as 'null' )
///

enum AttrVal {
    vFalse=0,		    ///< means false of this attribute -- 'T'
    vTrue,		      ///< means true of this attribute -- 'F'
    vAllAccept,		  ///< can accept any value of attribute ( true or false ) -- '?'
    vAllDeny,		    ///< deny all value of attribute -- 'null'
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Possible Hypothesis Result
/// @details Candidate Elimination Algorithm predict new instance searching all @n
///          hypothesis space and voting their result for new instance. If they @n
///          output same result as true or false, it will result T / F. Otherwise @n
///          their result is not unanimous, then it will result dontknow.

enum Result {
    r_false=0,		  ///< means all hypothesis vote false		
    r_true,		      ///< means all hypothesis vote true
    r_dontknow		  ///< means hypothesis vote is not unanimous
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Concept Learning Base Hypothesis Class
/// @details Hypothesis consists of array of attributes, which has AttrVal. @n
///          Array means actually conjunctively added for each other attributes. @n
///          Hypothesis assumes that its holding attributes represent positive. @n          
///

class Hypothesis {
private:
    AttrVal* hypo;	  ///< hypothesis format array ; ex) < ? , T , F, null, ? > ...
    int size;      ///< The number of total attributes for hypothesis except for target attribute

public:
    /// @name constructor
    /// @{

    /// @brief constructor taking size and given array of attributes
    /// 
    /// @param size The number of attributes which hypothesis will hold
    /// @param hypo Given attributes conjunction
    Hypothesis(int size, AttrVal* hypo);
    /// @}
    
    /// @name destructor
    /// @{

    /// @brief default destructor
    ~Hypothesis(void);
    /// @}

    /// @name Informative Attributes
    /// @{

    /// @brief getting this hypothesis attributes array
    ///
    /// @retval The array of attributes
    AttrVal* getHypo(void) const;
    /// @}

    /// @name Functional Attributes
    /// @{

    /// @brief checking if new input satisfy this hypothesis
    ///
    /// @param input new input whose form is array of boolean values
    /// @retval true if new input satisfy this hypothesis
    /// @retval false if new input don't satisfy this hypothesis
    bool isCover(bool* input) const;

    /// @brief checking if this hypothesis is more general than param h
    ///
    /// @details 'More general than' concept is based on logics @n
    ///          Explaining with upper atrributes value, AllAccept is @n
    ///          most general than others. True and False has same priority. @n
    ///          And last AllDeny is most specific one.
    /// @param h hypothesis which will be compared with this hypothesis
    /// @retval true if this hypothesis is more general than h
    /// @retval false if this hypothesis is not more general than h
    bool isMoreGeneralThanEqualTo(Hypothesis* h) const;
    /// @}
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Most Specific Bound of Version Space
/// @details Version Space is hypothesis space whose hypothesis are satisfying @n
///          target attributes as positive. This most specific bound is represented @n
///          as upper bound of all hypothesis in version space
///

class S_Bound {
private:
    List <Hypothesis*> *sb_Array;   ///< set of hypothesis in most specific bound
    int size;                       ///< the number of attributes except for target attribute
    friend class G_Bound;

public:
    /// @name constructor
    /// @{

    /// @brief constructor taking the number of attributes
    ///
    /// @param size the number of attributes
    S_Bound(int size);
    /// @}

    /// @name destructor
    /// @{
    
    /// @brief default destructor
    ~S_Bound(void);
    /// @}
  
    /// @name functional attributes
    /// @{

    /// @brief update s_bound with negative training instance
    ///
    /// @param n_input negative training instance
    /// @retval true if update success
    /// @retval false if update fail
    bool negUpdate(bool* n_input);
    
    /// @brief update s_bound with positive training instance 
    ///
    /// @param p_input positive training instance
    /// @retval true if update success
    /// @retval false if update fail
    bool posUpdate(bool* p_input);
    
    /// @brief generalize hypothesis
    /// @details used for posUpdate
    /// @param hypo copied hypothesis attribute array
    /// @param p_input compared new training example
    /// @retval generalized hypothesis
    Hypothesis* generalize(AttrVal* hypo, bool* p_input);

    /// @brief predict for new instance
    /// 
    /// @param u_input new instance which has no target attribute information
    /// @retval r_false all hypothesis votes false
    /// @retval r_true all hypothesis votes true
    /// @retval r_dontknow hypothesis votes non-unanimous
    Result predict(bool* u_input);
    /// @}
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Most General Bound of Version Space
/// @details Version Space is hypothesis space whose hypothesis are satisfying @n
///          target attributes as positive. This most general bound is represented @n
///          as lower bound of all hypothesis in version space
///

class G_Bound {
private:
    List <Hypothesis*> *gb_Array;    ///< set of hypothesis in most general bound
    int size;                        ///< the number of attributes except for target attribute

public:
    /// @name constructor
    /// @{

    /// @brief constructor taking the number of attributes
    /// @param size the number of attributes except for target attribute
    G_Bound(int size);
    /// @}

    /// @name destructor
    /// @{

    /// @brief default destructor
    ~G_Bound(void);
    /// @}

    /// @name functional attributes
    /// @{

    /// @brief update g_bound with positive training instance
    ///
    /// @param p_input positive training instance
    /// @retval true if update success
    /// @retval false if update fail
    bool posUpdate(bool* p_input);

    /// @brief update g_bound with negative training instance 
    ///
    /// @param n_input negative training instance
    /// @param ptr_sb used for specialization 
    /// @retval true if update success
    /// @retval false if update fail
    bool negUpdate(bool* n_input, S_Bound* ptr_sb);

    /// @brief specialize hypothesis and enroll to gb_Array
    /// @details used for negUpdate, and automatically enroll to gb_Array
    /// @param hypo copied hypothesis attribute array
    /// @param n_input compared new training example
    /// @param ptr_sb used for comparing with S_Bound
    /// @retval true if enroll success
    /// @retval false if enroll fail
    bool specialize(AttrVal* hypo, bool* n_input, S_Bound* ptr_sb);
    
    /// @brief predict for new instance
    /// 
    /// @param u_input new instance which has no target attribute information
    /// @retval r_false all hypothesis votes false
    /// @retval r_true all hypothesis votes true
    /// @retval r_dontknow hypothesis votes non-unanimous
    Result predict(bool* u_input);
    /// @}
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Candidate Elmination Algorithm Engine
/// @details CE algorithm as a kind of concept learning algorithm. @n
///          Defines two bound to represent version space, and predict @n
///          comparing only bound hypotheses. Training examples are used for @n
///          building two bounds.
///

class CE {
private:
    S_Bound* s_bound;    ///< Most Specific Bound
    G_Bound* g_bound;    ///< Most General Bound
    int size;        ///< the number of attribute except for target attribute

public:
    /// @name constructor
    /// @{

    /// @brief constructor taking the number of attributes
    /// @param size the number of attributes
    CE(int size);
    /// @}

    /// @name destructor
    /// @{

    /// @brief default destructor
    ~CE(void);
    /// @}
    
    /// @name functional attributes
    /// @{
    
    /// @brief update version space
    ///
    /// @param input training example with target attribute value
    /// @retval true if update success
    /// @retval false if update fail
    bool updateVS(bool* input);
    
    /// @brief predict for new unknown instance
    ///
    /// @param u_input new unknown instance with no target attribute
    /// @retval r_true if all version space hypotheses vote true
    /// @retval r_false if all version space hypotheses vote false
    /// @retval r_dontknow if hypotheses vote unanimously
    Result predict(bool* u_input);
    /// @}
};

#endif /* __CONCEPTLEARNING__ */
