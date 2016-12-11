// $Id: listmap.h,v 1.12 2016-05-04 13:49:56-07 - - $

#ifndef __LISTMAP_H__
#define __LISTMAP_H__

#include "xless.h"
#include "xpair.h"
#include "trace.h"

template <typename Key, typename Value, class Less=xless<Key>>
class listmap {
public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = xpair<const key_type, mapped_type>;
private:
    Less less;
    struct node;
    struct link {
        node* next{};
        node* prev{};
        link (node* next, node* prev): next(next), prev(prev){}
    };
    struct node: link {
        value_type value{};
        node (node* next, node* prev, const value_type&);
    };
    node* anchor() { return static_cast<node*> (&anchor_); }
    link anchor_ {anchor(), anchor()};
public:
    class iterator;
    listmap (){};
    listmap (const listmap&);
    listmap& operator= (const listmap&);
    ~listmap ();
    iterator insert (const value_type&);
    string find (const key_type&);
    iterator erase (iterator position);
    iterator begin() { return anchor()->next; }
    iterator end() { return anchor(); }
    bool empty() { return begin() == end(); }
};

template <typename Key, typename Value, class Less>
class listmap<Key,Value,Less>::iterator {
private:
    friend class listmap<Key,Value>;
    listmap<Key,Value,Less>::node* where {nullptr};
    iterator (node* where): where(where){};
public:
    iterator(){}
    value_type& operator*();
    value_type* operator->();
    iterator& operator++(); //++itor
    iterator& operator--(); //--itor
    void erase() { delete where; };
    bool operator== (const iterator&) const;
    bool operator!= (const iterator&) const;
};



//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (
                                     node* next, node* prev,
                                     const value_type& value):
link (next, prev), value (value) {
    // Constructor
}
//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap() {
    TRACE ('l', (void*) this);
    auto it = begin();
    ++it;
    auto temp = begin();
    while (it != end()) {
        erase(temp);
        temp = it;
        ++it;
    }
    if(temp != end()){
        erase(temp);
    }
}


//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::insert (const value_type& pair) {
    
    node *to_insert = new node(nullptr, nullptr, pair);
    if (empty()) {
        begin().where->next = to_insert;
        begin().where->prev = nullptr;
        to_insert->next = end().where;
        return nullptr;
    }
    iterator it;
    it = begin();
    while (it.where != end().where) {
        if (pair.first == it.where->value.first) {
            it.where->value.second = pair.second;
            return nullptr;
        }
        if (less(pair.first, it.where->value.first)) {
            if (it == begin().where) {
                to_insert->next = it.where;
                it.where->prev = to_insert;
                anchor_.next = to_insert;
                return nullptr;
            } else {
                to_insert->next = it.where;
                to_insert->prev = it.where->prev;
                it.where->prev->next = to_insert;
                it.where->prev = to_insert;
                return nullptr;
            }
        }
        auto trace = it;
        ++it;
        if (it.where == end().where) {
            to_insert->next = end().where;
            to_insert->prev = trace.where;
            trace.where->next = to_insert;
            return nullptr;
        }
    }
    return end();
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
string listmap<Key,Value,Less>::find (const key_type& that) {
    for (auto it = begin(); it != end(); ++it) {
        if (it->first == that){
            return it->second;
        }
    }
    return "";
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {
    auto *bef_node = position.where->prev;
    auto *aft_node = position.where->next;
    if (bef_node != nullptr) {
        position.where->prev->next = position.where->next;
    }
    if (aft_node != nullptr) {
        position.where->next->prev = position.where->prev;
    }
    if (position == begin()) {
        anchor_.next = anchor_.next->next;
    }
    if (position == end()) {
        anchor_.prev = anchor_.prev->prev;
    }
    position.erase();
    
    
    return nullptr;
}



//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type&
listmap<Key,Value,Less>::iterator::operator*() {
    TRACE ('l', where);
    return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type*
listmap<Key,Value,Less>::iterator::operator->() {
    TRACE ('l', where);
    return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++() {
    TRACE ('l', where);
    where = where->next;
    return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator--() {
    TRACE ('l', where);
    where = where->prev;
    return *this;
}


//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator==
(const iterator& that) const {
    return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator!=
(const iterator& that) const {
    return this->where != that.where;
}

#endif

