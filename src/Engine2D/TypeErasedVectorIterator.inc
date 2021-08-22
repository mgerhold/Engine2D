class TypeErasedVectorIteratorBase {
public:
    TypeErasedVectorIteratorBase(const TypeErasedVectorIteratorBase& other) = default;

    TypeErasedVectorIteratorBase& operator=(const TypeErasedVectorIteratorBase& other) noexcept {
        mAddress = other.mAddress;
        mStepSize = other.mStepSize;
        return *this;
    }

    [[nodiscard]] TypeErasedVectorIteratorBase operator+(std::size_t increment) noexcept {
        return TypeErasedVectorIteratorBase{ static_cast<std::uint8_t*>(mAddress) + increment * mStepSize, mStepSize };
    }

    TypeErasedVectorIteratorBase operator++(int) noexcept {
        const auto copy{ *this };
        ++(*this);
        return copy;
    }

    TypeErasedVectorIteratorBase& operator++() noexcept {
        mAddress = (static_cast<std::uint8_t*>(mAddress) + mStepSize);
        return *this;
    }

    [[nodiscard]] bool operator==(const TypeErasedVectorIteratorBase& other) const noexcept {
        assert(mStepSize == other.mStepSize);
        return mAddress == other.mAddress;
    }

protected:
    TypeErasedVectorIteratorBase(void* address, std::size_t stepSize) noexcept
        : mAddress{ address },
          mStepSize{ stepSize } { }

protected:
    void* mAddress;
    std::size_t mStepSize;

    friend class TypeErasedVector;
};


class TypeErasedVectorIterator : public TypeErasedVectorIteratorBase {
public:
    using TypeErasedVectorIteratorBase::TypeErasedVectorIteratorBase;

    [[nodiscard]] void* operator*() noexcept {
        return mAddress;
    }

    [[nodiscard]] void const* operator*() const noexcept {
        return mAddress;
    }
};

template<typename T>
class TypedTypeErasedVectorIterator : public TypeErasedVectorIteratorBase {
public:
    using TypeErasedVectorIteratorBase::TypeErasedVectorIteratorBase;

    [[nodiscard]] T& operator*() noexcept {
        return *static_cast<T*>(mAddress);
    }

    [[nodiscard]] const T& operator*() const noexcept {
        return *static_cast<const T*>(mAddress);
    }
};