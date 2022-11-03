#include <cstdlib>
#include <cstdint>
#include <iterator>
#include <type_traits>


    /*              StackStorage                */


template <size_t N>
class StackStorage {
private:
    char first_p[N];
    char* last_p{first_p};

public:
    StackStorage() = default;
    StackStorage(const StackStorage&) = delete;
    StackStorage& operator=(const StackStorage&) = delete;

    char* allocate(size_t n, const size_t alignof_t) {
        n += alignof_t - ((reinterpret_cast<uintptr_t>(last_p) + n) % alignof_t);
        char* last_p_copy = last_p;
        last_p += n;
        return last_p_copy;
    }

    void deallocate(char* pointer, size_t n, const size_t alignof_t) {
        n += alignof_t - ((reinterpret_cast<uintptr_t>(last_p) + n) % alignof_t);
        if (pointer + n == last_p) last_p = pointer;
    }
};


    /*              StackAllocator                */


template <class T, size_t N>
class StackAllocator {
private:
    template <class U, size_t M> friend class StackAllocator;
    StackStorage<N>* storage {};

public:
    using value_type = T;

    template <class U> struct rebind { using other = StackAllocator<U, N>; };

    StackAllocator() = default;
    StackAllocator(StackStorage<N>& other_storage): storage(&other_storage) {}

    template <class U>
    StackAllocator(StackAllocator<U, N> const& other): storage(other.storage) { storage->allocate(1, alignof(T)); }

    StackAllocator& operator=(StackAllocator const& other_allocator) {
        storage = other_allocator.storage;
        return *this;
    }

    T* allocate(size_t const n) { return static_cast<T*>(static_cast<void*>(storage->allocate(n * sizeof(T), alignof(T)))); }
    void deallocate(T* const pointer, size_t const n) {
        storage->deallocate(static_cast<char*>(static_cast<void*>(pointer)), n * sizeof(T), alignof(T));
    }

    template <class U, class ...A>
    void construct(U* const pointer, A&& ...args) { new (pointer) U(std::forward<A>(args)...); }

    template <class U>
    void destroy(U* const pointer) { pointer->~U(); }

    template <class U, size_t M>
    bool operator==(StackAllocator<U, M> const& other_alloc) const { return storage == other_alloc.storage; }

    template <class U, size_t M>
    bool operator!=(StackAllocator<U, M> const& other_alloc) const { return !(*this == other_alloc); }
};


    /*              List                */


template<typename T, typename Allocator = std::allocator<T>>
class List {
private:
    struct Node {
        T value;
        Node* prev;
        Node* next;

        Node() = default;
        Node(Node* prev = nullptr, Node* next = nullptr): value(T()), prev(prev), next(next) {}
        explicit Node(const T& value, Node* prev, Node* next): value(value), prev(prev), next(next) {};
    };

    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

    Node* base_node;
    NodeAllocator node_alloc;
    size_t sz = 0;

    void connect_nodes(Node* left, Node* right) {
        if (left != nullptr) left->next = right;
        if (right != nullptr) right->prev = left;
    }

    Node* build_base_node() {
        Node* new_head = static_cast<Node*>(std::allocator_traits<NodeAllocator>::allocate(node_alloc, 1));
        connect_nodes(new_head, new_head);
        return new_head;
    }


    /*              Iterators                */


    template<bool Const>
    class common_iterator {
    private:
        Node* node;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = int8_t;
        using pointer = typename std::conditional_t<Const, const T*, T*>;
        using reference = typename std::conditional_t<Const, const T&, T&>;

        explicit common_iterator(const Node* node): node(const_cast<Node*>(node)) {};
        common_iterator(): node(nullptr) {};
        common_iterator(const common_iterator<false>& iter): node(iter.node) {};

        typename common_iterator::reference operator*() const { return node->value; }
        typename common_iterator::pointer operator->() const { return &node->value; }

        bool operator==(const common_iterator& other_iter) const {
            if (other_iter.node == nullptr || node == nullptr) return false;
            return node == other_iter.node;
        }

        bool operator!=(const common_iterator& other) const { return !(*this == other); }

        common_iterator& operator++() {
            node = node->next;
            return *this;
        }
        common_iterator operator++(int) {
            common_iterator result = *this;
            ++(*this);
            return result;
        }

        common_iterator& operator--() {
            node = node->prev;
            return *this;
        }

        common_iterator operator--(int) {
            common_iterator result = *this;
            --(*this);
            return result;
        }

        friend class List<T, Allocator>;
    };

public:
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() { return iterator(base_node->next); }
    const_iterator begin() const { return const_iterator(base_node->next); }
    const_iterator cbegin() const { return const_iterator(base_node->next); }
    reverse_iterator rbegin() { return std::reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return std::reverse_iterator(cend()); }
    const_reverse_iterator crbegin() const { return std::reverse_iterator(cend());}

    iterator end() { return iterator(base_node); }
    const_iterator end() const { return const_iterator(base_node); }
    const_iterator cend() const { return const_iterator(base_node); }
    reverse_iterator rend() { return std::reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return std::reverse_iterator(cbegin()); }
    const_reverse_iterator crend() const { return std::reverse_iterator(cbegin()); }


    /*              List Methods                */


    List(): node_alloc(Allocator()) {base_node = build_base_node();}

    explicit List(const Allocator& allocator): node_alloc(allocator) { base_node = build_base_node(); }

    List(size_t given_size, const T& value, const Allocator& allocator = Allocator()): List(allocator) {
        try {
            for (size_t i = 0; i < given_size; ++i) {
                insert_before_with_value(end(), T());
            }
        } catch(...) {
            while (!empty()) {
                pop_front();
            }
            throw;
        }
    }

    explicit List(size_t given_size, const Allocator& allocator = Allocator()): List(allocator) {
        try {
            for (size_t i = 0; i < given_size; ++i) {
                insert_before(end());
            }
        } catch(...) {
            while (!empty()) {
                pop_front();
            }
            throw;
        }
    }

    List(const List& other_list): List(std::allocator_traits<Allocator>::select_on_container_copy_construction(other_list.node_alloc)) {
        try {
            for (const auto& others_node: other_list) {
                push_back(others_node);
            }
        } catch(...) {
            while (!empty()) {
                pop_front();
            }
            throw;
        }
    }

    List& operator=(const List& other_list) {
        if (this == &other_list) return *this;

        size_t num_of_added = 0;
        size_t copy_size = sz;
        try {
            const_iterator iter = other_list.begin();
            while (iter++ != other_list.end()) {
                push_back(*iter);
                ++num_of_added;
            }
        } catch(...) {
            for (size_t i = 0; i < num_of_added; ++i) {
                pop_back();
            }
            throw;
        }

        for (size_t i = 0; i < copy_size; ++i) {
            pop_front();
        }

        if (std::allocator_traits<NodeAllocator>::propagate_on_container_copy_assignment::value)
            node_alloc = other_list.node_alloc;

        return *this;
    }

    NodeAllocator get_allocator() { return node_alloc; }

    void insert_before(const_iterator pos) {
        Node* prev = (--pos).node;
        Node* new_node = std::allocator_traits<NodeAllocator>::allocate(node_alloc, 1);
        try {
            std::allocator_traits<NodeAllocator>::construct(node_alloc, new_node, prev, (++pos).node);
            connect_nodes(new_node, pos.node);
            connect_nodes(prev, new_node);
        } catch(...) {
            std::allocator_traits<NodeAllocator>::deallocate(node_alloc, new_node, 1);
            throw;
        }
        ++sz;
    }

    void insert_before_with_value(const_iterator pos, const T& given_value) {
        Node* prev = (--pos).node;
        Node* new_node = std::allocator_traits<NodeAllocator>::allocate(node_alloc, 1);
        try {
            std::allocator_traits<NodeAllocator>::construct(node_alloc, new_node, given_value, prev, (++pos).node);
            connect_nodes(new_node, pos.node);
            connect_nodes(prev, new_node);
        } catch(...) {
            std::allocator_traits<NodeAllocator>::deallocate(node_alloc, new_node, 1);
            throw;
        }
        ++sz;
    }

    void erase(const_iterator pos) {
        if (pos != end()) {
            Node* next = (++pos).node;
            connect_nodes(std::prev(--pos).node, next);
            std::allocator_traits<NodeAllocator>::destroy(node_alloc, pos.node);
            std::allocator_traits<NodeAllocator>::deallocate(node_alloc, pos.node, 1);
            --sz;
        }
    }

    void push_back(const T& value) { insert_before_with_value(end(), value); }
    void push_front(const T& value) { insert_before_with_value(begin(), value); }
    void pop_back() { erase(std::prev(end())); }
    void pop_front() { erase(begin()); }
    void insert(const_iterator pos, const T& value) { insert_before_with_value(pos, value); }

    size_t size() const { return sz; }
    bool empty() const { return sz == 0; }

    ~List() {
        while (!empty()) {
            pop_front();
        }
        std::allocator_traits<NodeAllocator>::deallocate(node_alloc, base_node, 1);
    }
};
