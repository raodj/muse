/* 
 * File:   AgentPQ.h
 * Author: meseret gebre - meseret.gebre@gmail.com
 *
 * This implementation is based from  Dietmar Kuehl's f_heap implementation.
 // --------------------------------------------------------------------------
 // This implementation is based on the description found in "Network Flow",
 // R.K.Ahuja, T.L.Magnanti, J.B.Orlin, Prentice Hall. The algorithmic stuff
 // should be basically identical to this description, however, the actual
 // representation is not.
 // --------------------------------------------------------------------------
 * Created on April 19, 2009, 3:08 AM
 */

#ifndef AGENTPQ_H
#define	AGENTPQ_H

#include <functional>
#include <vector>
#include "Agent.h"
#include "DataTypes.h"
#include <iostream>

BEGIN_NAMESPACE(muse);

class AgentPQ {


protected:
  //This is the node define. what the fibonacci heap will maintain.
  class node
  {

  public:
    node(Agent * data): m_parent(0), m_lost_child(0), m_data(data) { m_children.reserve(8); }
    ~node() {}

    void      destroy();
    node*     join(node* tree); // add tree as a child
    void      cut(node* child); // remove the child
    
    int       lost_child() const { return m_lost_child; }
    void      clear()            { m_parent = 0; m_lost_child = 0; }
    int       rank() const       { return m_children.size(); }
    bool      is_root() const    { return m_parent == 0; }
    node*     parent() const     { return m_parent; }
    void      remove_all()       { m_children.erase(m_children.begin(), m_children.end()); }
    Agent*    data() const       { return m_data; }
    void      data(Agent* data)  { m_data = data; }

    std::vector<node*>::const_iterator begin() const   { return m_children.begin(); }
    std::vector<node*>::const_iterator end()   const   { return m_children.end();   }

    void ppHelper(std::ostream& os, const std::string &indent) const{
      os << indent <<((is_root()) ? "*" : ">") << *data() << std::endl;
      for (size_t i=0; (i <  m_children.size()); i++) {
          if ( m_children[i] != 0 ){
              m_children[i]->ppHelper(os, indent + "-");
          }
      }//end for
    }//end ppHelper

   private:
    int                m_index;  // index of the object in the parent's vector
    node*              m_parent; // pointer to the parent node
    std::vector<node*> m_children;
    int                m_lost_child;
    Agent*             m_data;

    node(node const&);
    void operator= (node const&);
  };

protected:
  std::vector<node*> m_roots;
  int                m_size;

public:  
    typedef node*                                           pointer;

    AgentPQ();
    ~AgentPQ();
    pointer  push(Agent* data);
    //void     pop();
    Agent*   top();

    void     update(pointer n, double old_top_time);
    //void     remove(pointer);

    bool     empty() const { return m_size == 0; }
    int      size()  const  { return m_size; }

    void prettyPrint(std::ostream& os) const {
      for (size_t i=0; (i < m_roots.size()); i++){
          if (m_roots[i] != 0 ){
              m_roots[i]->ppHelper(os,"-");
          }
      }
    }
private:
  void     decrease(pointer, Agent*);
  void     increase(pointer, Agent*);
  void     add_root(node* n);
  void     cut(node* n);
  void     find_min() const;
  mutable node* m_min;
  
  inline bool compare(const Agent *lhs, const Agent * rhs){
       return (lhs->getTopTime() >= rhs->getTopTime());
  }
 
  AgentPQ(AgentPQ const&);       // deliberately not implemented
  void operator=(AgentPQ const&);      // deliberately not implemented
};


END_NAMESPACE(muse); //end namespace

#endif	/* AGENTPQ_H */

