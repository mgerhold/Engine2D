class TypeErasedVectorIteratorBase {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;

public:
    TypeErasedVectorIteratorBase() = default;

    TypeErasedVectorIteratorBase(const TypeErasedVectorIteratorBase& other) = default;

    TypeErasedVectorIteratorBase(TypeErasedVectorIteratorBase&& other) {
        mAddress = other.mAddress;
        mStepSize = other.mStepSize;
        other.mAddress = nullptr;
        other.mStepSize = 0;
    }

    TypeErasedVectorIteratorBase& operator=(const TypeErasedVectorIteratorBase& other) noexcept {
        if (this == &other) {
            return *this;
        }
        mAddress = other.mAddress;
        mStepSize = other.mStepSize;
        return *this;
    }

    TypeErasedVectorIteratorBase& operator=(TypeErasedVectorIteratorBase&& other) noexcept {
        swap(*this, other);
        return *this;
    }

    [[nodiscard]] TypeErasedVectorIteratorBase operator+(std::size_t increment) noexcept {
        return TypeErasedVectorIteratorBase{ static_cast<std::uint8_t*>(mAddress) + increment * mStepSize, mStepSize };
    }

    TypeErasedVectorIteratorBase operator++(int) noexcept {// postfix
        const auto copy{ *this };
        ++(*this);
        return copy;
    }

    TypeErasedVectorIteratorBase& operator++() noexcept {// prefix
        mAddress = (static_cast<std::uint8_t*>(mAddress) + mStepSize);
        return *this;
    }

    difference_type operator-(const TypeErasedVectorIteratorBase& other) noexcept {
        return static_cast<difference_type>(static_cast<std::uint8_t*>(mAddress) -
                                            static_cast<std::uint8_t*>(other.mAddress)) /
               mStepSize;
    }

    [[nodiscard]] bool operator==(const TypeErasedVectorIteratorBase& other) const noexcept {
        assert(mStepSize == other.mStepSize);
        return mAddress == other.mAddress;
    }

    [[nodiscard]] bool operator!=(const TypeErasedVectorIteratorBase& other) const noexcept {
        return !(*this == other);
    }

    friend void swap(TypeErasedVectorIteratorBase& lhs, TypeErasedVectorIteratorBase& rhs) noexcept {
        std::swap(lhs.mAddress, rhs.mAddress);
        std::swap(lhs.mStepSize, rhs.mStepSize);
    }

protected:
    TypeErasedVectorIteratorBase(void* address, std::size_t stepSize) noexcept
        : mAddress{ address },
          mStepSize{ stepSize } { }

protected:
    void* mAddress{ nullptr };
    std::size_t mStepSize{ 0 };

    friend class TypeErasedVector;
};

template<typename T>
class TypedTypeErasedVectorIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;

public:
    TypedTypeErasedVectorIterator() = default;

    TypedTypeErasedVectorIterator(const TypedTypeErasedVectorIterator& other) = default;

    TypedTypeErasedVectorIterator(TypedTypeErasedVectorIterator&& other) {
        mAddress = other.mAddress;
        mStepSize = other.mStepSize;
        other.mAddress = nullptr;
        other.mStepSize = 0;
    }

    TypedTypeErasedVectorIterator& operator=(const TypedTypeErasedVectorIterator& other) noexcept {
        if (this == &other) {
            return *this;
        }
        mAddress = other.mAddress;
        mStepSize = other.mStepSize;
        return *this;
    }

    TypedTypeErasedVectorIterator& operator=(TypedTypeErasedVectorIterator&& other) noexcept {
        swap(*this, other);
        return *this;
    }

    [[nodiscard]] TypedTypeErasedVectorIterator operator+(std::size_t increment) noexcept {
        return TypedTypeErasedVectorIterator{ static_cast<std::uint8_t*>(mAddress) + increment * mStepSize, mStepSize };
    }

    TypedTypeErasedVectorIterator operator++(int) noexcept {// postfix
        const auto copy{ *this };
        ++(*this);
        return copy;
    }

    TypedTypeErasedVectorIterator& operator++() noexcept {// prefix
        mAddress = (static_cast<std::uint8_t*>(mAddress) + mStepSize);
        return *this;
    }

    difference_type operator-(const TypedTypeErasedVectorIterator& other) noexcept {
        return static_cast<difference_type>(static_cast<std::uint8_t*>(mAddress) -
                                            static_cast<std::uint8_t*>(other.mAddress)) /
               mStepSize;
    }

    [[nodiscard]] bool operator==(const TypedTypeErasedVectorIterator& other) const noexcept {
        assert(mStepSize == other.mStepSize);
        return mAddress == other.mAddress;
    }

    [[nodiscard]] bool operator!=(const TypedTypeErasedVectorIterator& other) const noexcept {
        return !(*this == other);
    }

    friend void swap(TypedTypeErasedVectorIterator& lhs, TypedTypeErasedVectorIterator& rhs) noexcept {
        std::swap(lhs.mAddress, rhs.mAddress);
        std::swap(lhs.mStepSize, rhs.mStepSize);
    }

    [[nodiscard]] reference operator*() noexcept {
        return *static_cast<pointer>(mAddress);
    }

    pointer operator->() const {
        return static_cast<pointer>(mAddress);
    }

protected:
    TypedTypeErasedVectorIterator(void* address, std::size_t stepSize) noexcept
        : mAddress{ address },
          mStepSize{ stepSize } { }

protected:
    void* mAddress{ nullptr };
    std::size_t mStepSize{ 0 };

    friend class TypeErasedVector;
};

template<typename T>
class ConstTypedTypeErasedVectorIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;

public:
    ConstTypedTypeErasedVectorIterator() = default;

    ConstTypedTypeErasedVectorIterator(const ConstTypedTypeErasedVectorIterator& other) = default;

    ConstTypedTypeErasedVectorIterator(ConstTypedTypeErasedVectorIterator&& other) {
        mAddress = other.mAddress;
        mStepSize = other.mStepSize;
        other.mAddress = nullptr;
        other.mStepSize = 0;
    }

    ConstTypedTypeErasedVectorIterator& operator=(const ConstTypedTypeErasedVectorIterator& other) noexcept {
        if (this == &other) {
            return *this;
        }
        mAddress = other.mAddress;
        mStepSize = other.mStepSize;
        return *this;
    }

    ConstTypedTypeErasedVectorIterator& operator=(ConstTypedTypeErasedVectorIterator&& other) noexcept {
        swap(*this, other);
        return *this;
    }

    [[nodiscard]] ConstTypedTypeErasedVectorIterator operator+(std::size_t increment) noexcept {
        return ConstTypedTypeErasedVectorIterator{ static_cast<std::uint8_t*>(mAddress) + increment * mStepSize,
                                                   mStepSize };
    }

    ConstTypedTypeErasedVectorIterator operator++(int) noexcept {// postfix
        const auto copy{ *this };
        ++(*this);
        return copy;
    }

    ConstTypedTypeErasedVectorIterator& operator++() noexcept {// prefix
        mAddress = (static_cast<std::uint8_t*>(mAddress) + mStepSize);
        return *this;
    }

    difference_type operator-(const ConstTypedTypeErasedVectorIterator& other) noexcept {
        return static_cast<difference_type>(static_cast<std::uint8_t*>(mAddress) -
                                            static_cast<std::uint8_t*>(other.mAddress)) /
               mStepSize;
    }

    [[nodiscard]] bool operator==(const ConstTypedTypeErasedVectorIterator& other) const noexcept {
        assert(mStepSize == other.mStepSize);
        return mAddress == other.mAddress;
    }

    [[nodiscard]] bool operator!=(const ConstTypedTypeErasedVectorIterator& other) const noexcept {
        return !(*this == other);
    }

    friend void swap(ConstTypedTypeErasedVectorIterator& lhs, ConstTypedTypeErasedVectorIterator& rhs) noexcept {
        std::swap(lhs.mAddress, rhs.mAddress);
        std::swap(lhs.mStepSize, rhs.mStepSize);
    }

    [[nodiscard]] const T& operator*() const noexcept {
        return *static_cast<const T*>(mAddress);
    }

    const T* operator->() const {
        return static_cast<const T*>(mAddress);
    }

protected:
    ConstTypedTypeErasedVectorIterator(void* address, std::size_t stepSize) noexcept
        : mAddress{ address },
          mStepSize{ stepSize } { }

protected:
    void* mAddress{ nullptr };
    std::size_t mStepSize{ 0 };

    friend class TypeErasedVector;
};


class TypeErasedVectorIterator : public TypeErasedVectorIteratorBase {
public:
    using typename TypeErasedVectorIteratorBase::difference_type;
    using typename TypeErasedVectorIteratorBase::iterator_category;

public:
    using TypeErasedVectorIteratorBase::TypeErasedVectorIteratorBase;

    [[nodiscard]] void* operator*() noexcept {
        return mAddress;
    }

    [[nodiscard]] const void* operator*() const noexcept {
        return mAddress;
    }
};