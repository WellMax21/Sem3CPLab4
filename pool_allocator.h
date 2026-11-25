#pragma once
#include <cstddef>
#include <memory>
#include <new>
#include <vector>

template <typename T, std::size_t BlockSize>
class Pool {
    static_assert(BlockSize > 0, "BlockSize must be > 0");

public:
    Pool() = default;
    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;
    ~Pool() {
        for (auto& b : blocks_)
            ::operator delete(b.ptr);
    }

    T* allocate(std::size_t n) {
        if (n == 0) return nullptr;
        if (n == 1 && !free_.empty()) {       
            T* p = free_.back();
            free_.pop_back();
            return p;
        }
        if (blocks_.empty() || cap_ - used_ < n) 
            addBlock(n > BlockSize ? n : BlockSize);
        T* p = cur_ + used_;
        used_ += n;
        return p;
    }

    void deallocate(T* p, std::size_t n) noexcept {
        for (std::size_t i = 0; i < n; ++i) {
            try { free_.push_back(p + i); } catch (...) {  }
        }
    }

    std::size_t blocksCount() const { return blocks_.size(); }

private:
    struct Block { T* ptr; };

    void addBlock(std::size_t count) {
        T* p = static_cast<T*>(::operator new(count * sizeof(T)));
        try {
            blocks_.push_back({p});
        } catch (...) {
            ::operator delete(p);
            throw;
        }
        cur_ = p;
        cap_ = count;
        used_ = 0;
    }

    std::vector<Block> blocks_;
    std::vector<T*> free_;
    T* cur_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t used_ = 0;
};

template <typename T, std::size_t BlockSize = 10>
class PoolAllocator {
public:
    using value_type = T;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    template <typename U>
    struct rebind { using other = PoolAllocator<U, BlockSize>; };

    PoolAllocator() : pool_(std::make_shared<Pool<T, BlockSize>>()) {}
    PoolAllocator(const PoolAllocator&) = default;
    PoolAllocator& operator=(const PoolAllocator&) = default;

    
    template <typename U>
    PoolAllocator(const PoolAllocator<U, BlockSize>&)
        : pool_(std::make_shared<Pool<T, BlockSize>>()) {}

    T* allocate(std::size_t n) { return pool_->allocate(n); }
    void deallocate(T* p, std::size_t n) noexcept { pool_->deallocate(p, n); }

    template <typename U, std::size_t M>
    bool operator==(const PoolAllocator<U, M>&) const = delete;

    bool operator==(const PoolAllocator& o) const { return pool_ == o.pool_; }
    bool operator!=(const PoolAllocator& o) const { return !(*this == o); }

private:
    std::shared_ptr<Pool<T, BlockSize>> pool_;
};
