#include <iostream>
#include <vector>

template <typename T>
class Deque {
private:
    static const size_t chunk_capacity = 32;
    size_t deque_size = 0;
    std::vector<T*> array;
    std::pair<size_t, size_t> first_element_location;
    std::pair<size_t, size_t> last_element_location;

public:

    Deque();
    explicit Deque(const int& given_size, const T& value = T());
    Deque(const Deque& other_deque);

    size_t size() const;
    void swap(Deque& other_deque);

    Deque& operator=(const Deque& other_deque);
    T& operator[](size_t index);
    const T& operator[](size_t index) const;
    T& at(size_t index);
    const T& at(size_t index) const;
    void push_back(const T& value);
    void pop_back();
    void push_front(const T& value);
    void pop_front();

    template <bool is_const>
    struct deque_iterator {
        size_t element_index;
        typename std::vector<T*>::const_iterator pointer;

        deque_iterator(typename std::vector<T*>::const_iterator pointer, size_t element_index): element_index(element_index), pointer(pointer) {}

        deque_iterator& operator++() {
            if (element_index == chunk_capacity - 1)
                ++pointer;

            ++element_index;
            element_index %= chunk_capacity;

            return *this;
        }

        deque_iterator& operator--() {
            if (element_index == 0)
                --pointer;
            element_index = (chunk_capacity + element_index - 1) % chunk_capacity;

            return *this;
        }

        int operator-(const deque_iterator<is_const>& other) const{
            return (pointer - other.pointer) * chunk_capacity + static_cast<int>(element_index) - static_cast<int>(other.element_index);
        }

        deque_iterator<is_const> operator+(const int& shift) const {
            return deque_iterator<is_const>(pointer + (element_index + shift) / chunk_capacity, (element_index + shift) % chunk_capacity);
        }

        deque_iterator<is_const> operator-(const int& shift) const {
            int blocks_difference = (element_index - shift) / chunk_capacity;
            blocks_difference = blocks_difference - ((element_index - shift) % chunk_capacity < 0 ? 1 : 0);

            return deque_iterator<is_const>(pointer + blocks_difference, ((element_index - shift) % chunk_capacity + chunk_capacity) % chunk_capacity);
        }

        bool operator<(const deque_iterator<is_const>& other) const {
            if (*this - other < 0)
                return true;
            return false;
        }

        bool operator>(const deque_iterator<is_const>& other) const {
            if (*this - other > 0)
                return true;
            return false;
        }

        bool operator==(const deque_iterator<is_const>& other) const {
            if (!(*this < other) && !(*this > other))
                return true;
            return false;
        }

        bool operator!=(const deque_iterator<is_const>& other) const {
            if (!(*this == other))
                return true;
            return false;
        }

        bool operator<=(const deque_iterator<is_const>& other) const {
            if (*this < other || *this == other)
                return true;
            return false;
        }

        bool operator>=(const deque_iterator<is_const>& other) const {
            if (*this > other || *this == other)
                return true;
            return false;
        }

        std::conditional_t<is_const, const T&, T&> operator*() const {
            auto& needed = *(*pointer + element_index);
            return needed;
        }

        std::conditional_t<is_const, const T*, T*> operator->() const {
            auto needed = ((*pointer) + element_index);
            return needed;
        }

    };

    using iterator = deque_iterator<false>;
    using const_iterator = const deque_iterator<true>;

    iterator begin() {
        auto needed = iterator(array.begin() + first_element_location.first, first_element_location.second);
        return needed;
    }

    iterator end() {
        Deque::deque_iterator<false> iterator_copy = iterator(array.begin() + last_element_location.first, last_element_location.second) + 1;
        return iterator_copy;
    }

    const_iterator begin() const {
        auto needed = const_iterator(array.begin() + first_element_location.first, first_element_location.second);
        return needed;
    }

    const_iterator end() const {
        Deque::deque_iterator<true> iterator_copy = const_iterator(array.begin() + last_element_location.first, last_element_location.second) + 1;
        return iterator_copy;
    }

    const_iterator cbegin() const {
        auto needed = const_iterator(array.begin() + first_element_location.first, first_element_location.second);
        return needed;
    }

    const_iterator cend() const {
        Deque::deque_iterator<true> iterator_copy = const_iterator(array.begin() + last_element_location.first, last_element_location.second) + 1;
        return iterator_copy;
    }


//    using reverse_iterator = std::reverse_iterator<iterator>;
//    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

//    reverse_iterator rbegin() { return std::reverse_iterator(end());}
//    reverse_iterator rend() { return std::reverse_iterator(begin());}
//    const_reverse_iterator rbegin() const { return std::reverse_iterator(cend());}
//    const_reverse_iterator rend() const { return std::reverse_iterator(cbegin());}
//    const_reverse_iterator crbegin() const { return std::reverse_iterator(cend());}
//    const_reverse_iterator crend() const { return std::reverse_iterator(cbegin());}

    void insert(iterator needed_iterator, const T& value);
    void erase(iterator needed_iterator);
};

template<typename T>
Deque<T>::Deque() {
    array.resize(4);
    first_element_location = std::make_pair(array.size()/2, chunk_capacity/2 + 1);
    last_element_location = std::make_pair(array.size()/2, chunk_capacity/2);

    for (size_t i = 0; i < array.size(); ++i) {
        array[i] = reinterpret_cast<T*>(new int8_t[chunk_capacity * sizeof(T)]);
    }
}

template<typename T>
Deque<T>::Deque(const int& given_size, const T& value) {
    size_t new_array_size = static_cast<size_t>(static_cast<double>(given_size) / static_cast<double>(chunk_capacity)) + 1;
    array.resize(new_array_size);
    deque_size = given_size;
    first_element_location.first = 0;
    first_element_location.second = 0;
    last_element_location.first = new_array_size - 1;
    last_element_location.second = (given_size - 1) % chunk_capacity;

    for (size_t i = 0; i < new_array_size; ++i) {
        array[i] = reinterpret_cast<T*>(new int8_t[chunk_capacity * sizeof(T)]);
        for (size_t j = 0; j < chunk_capacity; ++j) {
            if (i == new_array_size - 1 && j > last_element_location.second)
                break;
            new (array[i] + j) T(value);
        }
    }
}

template<typename T>
Deque<T>::Deque(const Deque& other_deque): Deque() {
    first_element_location = other_deque.first_element_location;
    last_element_location = other_deque.last_element_location;
    deque_size = other_deque.deque_size;
    array.resize(other_deque.array.size());

    for (size_t i = 0; i < array.size(); ++i) {
        array[i] = reinterpret_cast<T*>(new int8_t[chunk_capacity*sizeof(T)]);

        if ((i <= last_element_location.first) && (i >= first_element_location.first)) {
            for (size_t j = 0; j < chunk_capacity; ++j) {
                new (array[i] + j) T(other_deque.array[i][j]);
            }
        }
    }
}

template<typename T>
Deque<T>& Deque<T>::operator=(const Deque<T>& other_deque) {
    Deque iterator_copy = Deque(other_deque);
    swap(iterator_copy);
    return *this;
}

template<typename T>
size_t Deque<T>::size() const {
    return deque_size;
}

template<typename T>
void Deque<T>::swap(Deque& other_deque) {
    std::swap(array, other_deque.array);
    std::swap(deque_size, other_deque.deque_size);
    std::swap(first_element_location, other_deque.first_element_location);
    std::swap(last_element_location, other_deque.last_element_location);
}

template<typename T>
T& Deque<T>::operator[](size_t index) {
    return array[first_element_location.first + (first_element_location.second + index) / chunk_capacity][(first_element_location.second + index) % chunk_capacity];
}

template<typename T>
const T& Deque<T>::operator[](size_t index) const {
    return array[first_element_location.first + (first_element_location.second + index) / chunk_capacity][(first_element_location.second + index) % chunk_capacity];
}

template<typename T>
T& Deque<T>::at(size_t index) {
    if (index >= deque_size)
        throw std::out_of_range("Error: out of range");
    return (*this)[index];
}

template<typename T>
const T& Deque<T>::at(size_t index) const {
    if (index >= deque_size)
        throw std::out_of_range("Error: out of range");
    return (*this)[index];
}

template<typename T>
void Deque<T>::push_back(const T& value) {
    if (last_element_location == std::make_pair(array.size() - 1, chunk_capacity - 1)) {
        array.resize(2 * array.size());

        for (size_t i = array.size()/2; i < array.size(); ++i) {
            array[i] = reinterpret_cast<T*>(new int8_t[chunk_capacity * sizeof(T)]);
        }
    }

    if (last_element_location.second == chunk_capacity - 1)
        ++last_element_location.first;

    ++last_element_location.second;
    last_element_location.second %= chunk_capacity;

    new (array[last_element_location.first] + last_element_location.second) T(value);
    ++deque_size;
}

template<typename T>
void Deque<T>::pop_back() {
    if (size() == 0)
        return;

    (array[last_element_location.first] + last_element_location.second)->~T();

    if (last_element_location.second == 0)
        --last_element_location.first;

    last_element_location.second = (chunk_capacity + last_element_location.second - 1) % chunk_capacity;

    --deque_size;
}

template<typename T>
void Deque<T>::push_front(const T& value) {
    if (first_element_location == std::make_pair(size_t(0), size_t(0))) {
        array.resize(2 * array.size());

        first_element_location.first = array.size()/2;
        last_element_location.first += array.size()/2;

        for(size_t i = 0; i < array.size()/2 ; ++i) {
            array[array.size()/2 + i] = array[i];
            array[i] = reinterpret_cast<T*>(new int8_t[chunk_capacity * sizeof(T)]);
        }
    }

    if (first_element_location.second == 0) {
        --first_element_location.first;
        first_element_location.second = chunk_capacity - 1;
    } else {
        --first_element_location.second;
    }

    new(array[first_element_location.first] + first_element_location.second) T(value);
    ++deque_size;
}

template<typename T>
void Deque<T>::pop_front() {
    if (size() == 0)
        return;

    --deque_size;

    (array[first_element_location.first] + first_element_location.second)->~T();
    if (first_element_location.second == chunk_capacity - 1) {
        first_element_location.second = 0;
        ++first_element_location.first;
    } else {
        ++first_element_location.second;
    }
}

template<typename T>
void Deque<T>::insert(iterator needed_iterator, const T& value) {
    push_back(value);
    auto iter = --end() + 1;
    while (--iter != needed_iterator) {
        std::iter_swap(iter, iter - 1);
    }
}

template<typename T>
void Deque<T>::erase(iterator needed_iterator) {
    auto iter = needed_iterator - 1;
    while (++iter != --end()) {
        std::iter_swap(iter, iter + 1);
    }
    pop_back();
}
