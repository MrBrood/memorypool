//MemoryPool.hpp

#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP
#include <climits>
#include <cstddef>

template<typename T, size_t BlockSize = 4096>
class MemoryPool
{
public:
  typedef T* pointer;
  template<typename U> struct rebind{
    typedef MemoryPool<U> other;
  };

  MemoryPool() noexcept
  {
    currentBlock_ = nullptr;
    currentSlot_ = nullptr;
    lastSlot_ = nullptr;
    freeSlots_ = nullptr;
  }

  //销毁一个现有的内存池
  ~MemoryPool() noexcept
  {
    slot_pointer_ curr = currentBlock_;
    while(curr != nullptr)
    {
      slot_pointer_ prev = curr->next;
      operator delete(reinterpret_cast<void*>(curr));
      curr = prev;
    }
  }

  pointer allocate(size_t n=1, const T* hint = 0)
  {
    if(freeSlots_ != nullptr)
    {
      pointer result = reinterpret_cast<pointer>(freeSlots_);
      freeSlots_ = freeSlots_->next;
      return result;
    }
    else
    {
      if(currentSlot_ >= lastSlot_)
      {
        data_pointer_ newBlock = reinterpret_cast<data_pointer_>(operator new(BlockSize));
	reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
	currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
	data_pointer_ body = newBlock + sizeof(slot_pointer_);
	uintptr_t result = reinterpret_cast<uintptr_t>(body);
	size_t bodyPadding = (alignof(slot_type_) - result) % alignof(slot_type_);
	currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
	lastSlot_ = reinterpret_cast<slot_pointer_>(newBlock + BlockSize - sizeof(slot_type_));
      }
      return reinterpret_cast<pointer>(currentSlot_++);
    }
  }

  //销毁指针p指向的内存块
  void deallocate(pointer p, size_t n=1)
  {
    if(nullptr != p)
    {
      //reinterpret_cate是强制类型转换符
      //要访问next，必须强制将p转换为slot_pointer
      reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
      freeSlots_ = reinterpret_cast<slot_pointer_>(p);
    }
  }
  
  //调用构造函数
  template<typename U, typename... Args>
  void construct(U* p, Args&&... args)
  {
    new(p) U(std::forward<Args>(args)...);
  }

  
  //销毁内存池中的对象。即调用对象的析构函数
  template<typename U>
  void destroy(U* p)
  {
    p->~U();
  }


protected:
private:
  //用于存储内存池中的对象槽
  //要么被实例化为一个存放对象的槽；
  //要么被实例化为一个指向存放对象槽的槽指针
  union Slot_{
    T element;
    Slot_* next;
  };

  typedef char* data_pointer_;
  typedef Slot_ slot_type_;
  typedef Slot_* slot_pointer_;

  slot_pointer_ currentBlock_;
  slot_pointer_ currentSlot_;
  slot_pointer_ lastSlot_;
  slot_pointer_ freeSlots_;

  //检查定义的内存池的大小是否过小
  static_assert(BlockSize >= 2*sizeof(slot_type_), "BlockSize too small.");
};
#endif
