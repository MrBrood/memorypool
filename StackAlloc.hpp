//StackAlloc.hpp
#ifndef STACK_ALLOC 
#define STACK_ALLOC

#include <memory>

template<typename T>
struct StackNode_
{
  T data;
  StackNode_* prev;
};


template<class T, class Alloc = std::allocator<T>>
class StackAlloc
{
public:
  //使用typedef 简化类型名
  typedef StackNode_<T> Node;
  typedef typename Alloc::template rebind<Node>::other allocator;

  StackAlloc()
  {
    head_ = NULL;
  }

  ~StackAlloc()
  {
    clear();
  }

  bool empty(){return head_ == NULL;}

  void clear()
  {
    Node* curr = head_;
    while(NULL != curr)
    {
      Node* tmp = curr->prev;
      allocator_.destroy(curr);
      allocator_.deallocate(curr, 1);
      curr = tmp;
    }
    head_ = NULL;
  }

  void push(T element)
  {
    Node* newNode = allocator_.allocate(1);
    allocator_.construct(newNode, Node());

    //
    newNode->data = element;
    newNode->prev = head_;
    head_ = newNode;
  }
  //出栈
  T pop()
  {
    T result = head_->data;
    Node* tmp = head_->prev;
    allocator_.destroy(head_);
    allocator_.deallocate(head_, 1);
    head_ = tmp;
  }

  //返回栈顶元素
  T top()
  {
    return head_->data;
  }


private:
  allocator allocator_;
  Node* head_;
};
#endif
