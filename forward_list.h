#pragma once
#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>

template <typename T, typename Alloc = std::allocator<T>>
class ForwardList {
    struct Node {
        T value;
        Node* next;
        Node(const T& v) : value(v), next(nullptr) {}
    };
    using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using NodeTraits = std::allocator_traits<NodeAlloc>;

    template <bool Const>
    class Iter {
        using NodePtr = typename std::conditional<Const, const Node*, Node*>::type;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = typename std::conditional<Const, const T*, T*>::type;
        using reference = typename std::conditional<Const, const T&, T&>::type;

        Iter() = default;
        explicit Iter(NodePtr n) : n_(n) {}
        reference operator*() const { return n_->value; }
        pointer operator->() const { return &n_->value; }
        Iter& operator++() { n_ = n_->next; return *this; }
        Iter operator++(int) { Iter t = *this; n_ = n_->next; return t; }
        bool operator==(const Iter& o) const { return n_ == o.n_; }
        bool operator!=(const Iter& o) const { return n_ != o.n_; }
    private:
        NodePtr n_ = nullptr;
    };

public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = std::size_t;
    using iterator = Iter<false>;
    using const_iterator = Iter<true>;

    ForwardList() = default;
    explicit ForwardList(const Alloc& a) : alloc_(a) {}
    ForwardList(const ForwardList&) = delete;
    ForwardList& operator=(const ForwardList&) = delete;
    ~ForwardList() { clear(); }

    void push_back(const T& v) {
        Node* n = NodeTraits::allocate(alloc_, 1);
        try {
            NodeTraits::construct(alloc_, n, v);
        } catch (...) {
            NodeTraits::deallocate(alloc_, n, 1);
            throw;
        }
        if (tail_) tail_->next = n; else head_ = n;
        tail_ = n;
        ++size_;
    }

    void clear() {
        while (head_) {
            Node* next = head_->next;
            NodeTraits::destroy(alloc_, head_);
            NodeTraits::deallocate(alloc_, head_, 1);
            head_ = next;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    size_type size() const { return size_; }
    bool empty() const { return size_ == 0; }

    iterator begin() { return iterator(head_); }
    iterator end() { return iterator(nullptr); }
    const_iterator begin() const { return const_iterator(head_); }
    const_iterator end() const { return const_iterator(nullptr); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

private:
    NodeAlloc alloc_;
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    size_type size_ = 0;
};
