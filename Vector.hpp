#include <memory>
#include <new>


template <typename T, typename Alloc = std::allocator<T>>
class vector {
  private:
    [[no_unique_address]] Alloc alloc;
    using AllocTraits = std::allocator_traits<Alloc>;
    T* arr = nullptr;
    size_t cap = 0;
    size_t sz = 0;

  public:
    vector() noexcept {}

    constexpr vector(size_t count): alloc(), arr(AllocTraits::allocate(alloc, count)) {
        size_t i = 0;
        try {
            for (; i < count; ++i) {
                AllocTraits::construct(alloc, arr + i, T());
            }
        } catch(...) {
            for (size_t j = 0; j < i; ++j) {
                AllocTraits::destroy(alloc, arr + j);
            }
            AllocTraits::deallocate(alloc, arr, count);
            throw;
        }
        cap = count;
        sz = count;
    }

    constexpr vector(size_t count, const T& value)
                                    : alloc(), arr(AllocTraits::allocate(alloc, count)) {
        size_t i = 0;
        try {
            for (; i < count; ++i) {
                AllocTraits::construct(alloc, arr + i, value);
            }
        } catch(...) {
            for (size_t j = 0; j < i; ++j) {
                AllocTraits::destroy(alloc, arr + j);
            }
            AllocTraits::deallocate(alloc, arr, count);
            throw;
        }
        cap = count;
        sz = count;
    }

    constexpr vector(size_t count, const T& value, const Alloc& alloc)
                                    : alloc(alloc), arr(AllocTraits::allocate(alloc, count)) {
        size_t i = 0;
        try {
            for (; i < count; ++i) {
                AllocTraits::construct(alloc, arr + i, value);
            }
        } catch(...) {
            for (size_t j = 0; j < i; ++j) {
                AllocTraits::destroy(alloc, arr + j);
            }
            AllocTraits::deallocate(alloc, arr, count);
            throw;
        }
        cap = count;
        sz = count;
    }

    constexpr vector(const vector& other): alloc(AllocTraits::select_on_container_copy_construction(other.alloc)), arr(AllocTraits::allocate(alloc, other.capacity())) {
        size_t i = 0;
        try {
            for (; i < other.size(); ++i) {
                AllocTraits::construct(alloc, arr + i, other[i]);
            }
        } catch(...) {
            for (size_t j = 0; j < i; ++j) {
                AllocTraits::destroy(alloc, arr + j);
            }
            AllocTraits::deallocate(alloc, arr, other.capacity());
            throw;
        }
        cap = other.capacity();
        sz = other.size();
    }

    constexpr vector(const vector& other, const Alloc& alloc)
                    : alloc(alloc), arr(AllocTraits::allocate(alloc, other.capacity())) {
        size_t i = 0;
        try {
            for (; i < other.size(); ++i) {
                AllocTraits::construct(alloc, arr + i, other[i]);
            }
        } catch(...) {
            for (size_t j = 0; j < i; ++j) {
                AllocTraits::destroy(alloc, arr + j);
            }
            AllocTraits::deallocate(alloc, arr, other.capacity());
            throw;
        }
        cap = other.capacity();
        sz = other.size();
    }

    constexpr vector(vector&& other) noexcept
        : sz(other.size()), cap(other.capacity()), arr(other.arr) {
        other.cap = 0;
        other.sz = 0;
        other.arr = nullptr;
        alloc = std::move(other.alloc);
    }

    constexpr vector(vector&& other, const Alloc& alloc)
        : sz(other.size()), cap(other.capacity()), arr(other.arr), alloc(alloc) {
        other.cap = 0;
        other.sz = 0;
        other.arr = nullptr;
    }

    constexpr vector(std::initializer_list<T> init, const Alloc& alloc = Alloc()): alloc(alloc) { // is it corr move sem
        reserve(init.size());
        for (auto&& t : init) {
            emplace_back(std::move(t));
        }
    }

    T& operator[](const size_t& ind) {
        return arr[ind];
    }

    const T& operator[](const size_t& ind) const {
        return arr[ind];
    }

    T& at(const size_t& ind) {
        if (ind >= sz) {
            throw std::out_of_range("");
        }
        return arr[ind];
    }

    const T& at(const size_t& ind) const {
        if (ind >= sz) {
            throw std::out_of_range("");
        }
        return arr[ind];
    }

    size_t size() const noexcept {
        return sz;
    }

    size_t capacity() const noexcept {
        return cap;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        if (cap == sz) {
            size_t newcap = std::max(1ul, cap * 2);
            T* newarr = AllocTraits::allocate(alloc, sizeof(T) * newcap);
            size_t i = 0;
            try {
                AllocTraits::construct(alloc, newarr + sz, std::forward<Args>(args)...);
                for (; i < sz; ++i) {
                    AllocTraits::construct(alloc, newarr + i, std::move_if_noexcept(arr[i]));
                }
            } catch(...) {
                for (size_t j = 0; j < i; ++j) {
                    AllocTraits::destroy(alloc, newarr + j);
                }
                AllocTraits::deallocate(alloc, newarr, newcap);
                throw;
            }

            for (i = 0; i < sz; ++i) {
                AllocTraits::destroy(alloc, arr + i);
            }
            AllocTraits::deallocate(alloc, arr, cap);
            arr = newarr;
            cap = newcap;
        } else {
            AllocTraits::construct(alloc, arr + sz, std::forward<Args>(args)...);
        }
        ++sz;
    }

    void push_back(const T& value) {
        emplace_back<const T&>(value);
    }

    void pop_back() {
        AllocTraits::destoy(alloc, arr + sz);
        --sz;
    }

    constexpr void resize(const size_t count, const T& value) {
        if (count == sz || count < 0) {
            return;
        } else if (count < sz) {
            for (size_t i = sz; i > count; --i) {
                pop_back(); 
            }
        } else {
            if (count > cap) { 
                T* newarr = AllocTraits::allocate(alloc, std::max(1ul, cap * 2));
                size_t i = 0;
                try {
                    for (; i < sz; ++i) {
                        AllocTraits::construct(alloc, newarr + i, arr[i]);
                    }
                    for (i = sz; i < count; ++i) {
                        AllocTraits::construct(alloc, newarr + i, value);
                    }
                } catch(...) {
                    for (size_t j = 0; j < i; ++j) {
                        AllocTraits::destroy(alloc, newarr + j);
                    }
                    AllocTraits::deallocate(alloc, newarr, std::max(1ul, cap * 2));
                    throw;
                }
                arr = newarr;
                cap = std::max(1ul, cap * 2);
            } else {
                size_t i = sz;
                try {
                    for (; i < count; ++i) {
                        AllocTraits::constuct(alloc, arr + i, std::forward(value));
                    }
                } catch(...) {
                    for (size_t j = sz; j < i; ++j) {
                        AllocTraits::destroy(alloc, arr + j);
                    }
                    throw;
                }
            }
            sz = count;
        }
    }

    constexpr void reserve(const size_t newcap) {
        if (newcap <= cap) {
            return;
        }
        T* newarr = AllocTraits::allocate(alloc, sizeof(T) * newcap);
        size_t i = 0;
        try {
            for (; i < sz; ++i) {
                AllocTraits::construct(alloc, newarr + i, std::move_if_noexcept(arr[i]));
            }
        } catch(...) {
            for (size_t j = 0; j < i; ++j) {
                AllocTraits::destroy(alloc, newarr + j);
            }
            AllocTraits::deallocate(alloc, newarr, newcap);
            throw;
        }

        for (i = 0; i < sz; ++i) {
            AllocTraits::destroy(alloc, arr + i);
        }
        AllocTraits::deallocate(alloc, arr, cap);
        arr = newarr;
        cap = newcap;
    }

    constexpr void clear() noexcept {
        for (size_t i = sz; i > 0; ++i) {
            pop_back();
        }
    }

    vector<T>& operator=(const vector<T>& other) {
        if (std::is_same_v<AllocTraits::propagate_on_container_copy_assignment(), std::true_type>) {       // ??
            alloc = other.alloc;
        } else {
            alloc = Alloc();
        }

        arr = AllocTraits::allocate(alloc, sizeof(T) * other.capacity());
        size_t i = 0;
        try {
            for (; i < other.size(); ++i) {
                AllocTraits::construct(alloc, arr + i, std::move_if_noexcept(other[i]));
            }
        } catch(...) {
            for (size_t j = 0; j < i; ++j) {
                AllocTraits::destroy(alloc, arr + j);
            }
            AllocTraits::deallocate(alloc, arr, other.capacity());
            throw;
        }
    }

    ~vector() {
        for (size_t i = 0; i < sz; ++i) {
            AllocTraits::destroy(alloc, arr +i);
        }
        AllocTraits::deallocate(alloc, arr, sz);
        sz = 0;
        cap = 0;
    }
};

template <>
class vector<bool> {
  private:
    char *arr;
    size_t sz;
    size_t cap;

    struct BoolReference {
        char* chunk;
        uint8_t bit;

        BoolReference(char* chunk, uint8_t bit): chunk(chunk), bit(bit) {}
        BoolReference operator=(bool b) {
            if (b) {
                *chunk |= 1 << bit;
            } else {
                *chunk &= ~(1 << bit);
            }
            return *this;
        }
        operator bool() const {
            return *chunk & (1 << bit);
        }
    };

  public:
    BoolReference operator[](size_t index) {
        return BoolReference(arr + (index >> 3), index & 7);
    }

    // TODO bool specialization
};
