#pragma once
#include <cassert>

#include "../noncopyable.h"

namespace t9::intrusive {

template <class Tag = void>
class ListNode : private NonMovable {
 public:
  ListNode* prev;
  ListNode* next;

 protected:
  ListNode() {
    prev = this;
    next = this;
  }
  ~ListNode() {
    assert(prev == this);
    assert(next == this);
  }
};

template <class Node, class T>
class ListIterator {
 private:
  Node* node_ = nullptr;

 public:
  explicit ListIterator(Node* n) : node_(n) {}

  T* get() const { return static_cast<T*>(node_); }
  T* operator->() const { return static_cast<T*>(node_); }
  T& operator*() const { return *static_cast<T*>(node_); }

  ListIterator& operator++() {
    node_ = node_->next;
    return *this;
  }
  ListIterator operator++(int) {
    ListIterator it(*this);
    node_ = node_->next;
    return it;
  }
  ListIterator& operator--() {
    node_ = node_->prev;
    return *this;
  }
  ListIterator operator--(int) {
    ListIterator it(*this);
    node_ = node_->prev;
    return it;
  }

  bool operator==(const ListIterator& rhs) const { return node_ == rhs.node_; }
  bool operator!=(const ListIterator& rhs) const { return node_ != rhs.node_; }
};

template <class T, class Tag = void>
class List : private NonMovable {
 private:
  using Node = ListNode<Tag>;
  using Iterator = ListIterator<Node, T>;
  using ConstIterator = ListIterator<const Node, const T>;

 private:
  class RootNode : public Node {};

 private:
  RootNode root_;

 public:
  List() = default;

  Iterator begin() { return Iterator(root_.next); }
  ConstIterator begin() const { return ConstIterator(root_.next); }
  Iterator end() { return Iterator(&root_); }
  ConstIterator end() const { return ConstIterator(&root_); }

  const T* front() const { return static_cast<T*>(root_.next); }
  T* front_mut() { return static_cast<T*>(root_.next); }
  const T* back() const { return static_cast<T*>(root_.prev); }
  T* back_mut() { return static_cast<T*>(root_.prev); }

  void push_front(Node* x) { link_prev(root_.next, x); }
  void push_back(Node* x) { link_prev(&root_, x); }
  void pop_front() { unlink(root_.next); }
  void pop_back() { unlink(root_.prev); }

  void unlink(Node* node) {
    auto p = node->prev;
    auto n = node->next;
    p->next = n;
    n->prev = p;
    node->prev = node->next = node;
  }
  Iterator erase(Iterator it) {
    auto n = (it++).get();
    unlink(n);
    return it;
  }
  void remove(Node* n) { unlink(n); }

  bool is_empty() const { return !is_linked_(&root_); }
  bool contains(const Node* n) const { return is_linked_(n); }

  void clear() {
    Node* it = root_.next;
    Node* end = &root_;
    while (it != end) {
      auto n = it;
      it = it->next;
      unlink(n);
    }
  }

 private:
  bool is_linked_(const Node* n) const { return n->prev != n || n->next != n; }
  void link_prev(Node* next, Node* x) {
    auto p = next->prev;
    p->next = x;
    next->prev = x;
    x->prev = p;
    x->next = next;
  }
};

}  // namespace t9::intrusive
