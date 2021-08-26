namespace c2k {

    class TypeErasedVector;

    namespace Iterators {

        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = void*;
            using pointer = void* const*;
            using reference = void* const&;

        public:
            Iterator() = default;

            Iterator(const Iterator& other) = default;

            Iterator(Iterator&& other) {
                mAddress = other.mAddress;
                mStepSize = other.mStepSize;
                other.mAddress = nullptr;
                other.mStepSize = 0;
            }

            Iterator& operator=(const Iterator& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                mAddress = other.mAddress;
                mStepSize = other.mStepSize;
                return *this;
            }

            Iterator& operator=(Iterator&& other) noexcept {
                swap(*this, other);
                return *this;
            }

            [[nodiscard]] Iterator operator+(std::size_t increment) noexcept {
                return Iterator{ static_cast<std::uint8_t*>(mAddress) + increment * mStepSize, mStepSize };
            }

            Iterator operator++(int) noexcept {// postfix
                const auto copy{ *this };
                ++(*this);
                return copy;
            }

            Iterator& operator++() noexcept {// prefix
                mAddress = (static_cast<std::uint8_t*>(mAddress) + mStepSize);
                return *this;
            }

            difference_type operator-(const Iterator& other) noexcept {
                return static_cast<difference_type>(static_cast<std::uint8_t*>(mAddress) -
                                                    static_cast<std::uint8_t*>(other.mAddress)) /
                       mStepSize;
            }

            [[nodiscard]] bool operator==(const Iterator& other) const noexcept {
                assert(mStepSize == other.mStepSize);
                return mAddress == other.mAddress;
            }

            [[nodiscard]] bool operator!=(const Iterator& other) const noexcept {
                return !(*this == other);
            }

            friend void swap(Iterator& lhs, Iterator& rhs) noexcept {
                std::swap(lhs.mAddress, rhs.mAddress);
                std::swap(lhs.mStepSize, rhs.mStepSize);
            }

            [[nodiscard]] reference operator*() const noexcept {
                return mAddress;
            }

            pointer operator->() const {
                return &mAddress;
            }

        protected:
            Iterator(void* address, std::size_t stepSize) noexcept : mAddress{ address }, mStepSize{ stepSize } { }

        protected:
            void* mAddress{ nullptr };
            std::size_t mStepSize{ 0 };

            friend class c2k::TypeErasedVector;
        };// Iterator

        class ConstIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = void const*;
            using pointer = void const* const*;
            using reference = void const* const&;

        public:
            ConstIterator() = default;

            ConstIterator(const ConstIterator& other) = default;

            ConstIterator(ConstIterator&& other) {
                mAddress = other.mAddress;
                mStepSize = other.mStepSize;
                other.mAddress = nullptr;
                other.mStepSize = 0;
            }

            ConstIterator& operator=(const ConstIterator& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                mAddress = other.mAddress;
                mStepSize = other.mStepSize;
                return *this;
            }

            ConstIterator& operator=(ConstIterator&& other) noexcept {
                swap(*this, other);
                return *this;
            }

            [[nodiscard]] ConstIterator operator+(std::size_t increment) noexcept {
                return ConstIterator{ static_cast<std::uint8_t*>(mAddress) + increment * mStepSize, mStepSize };
            }

            ConstIterator operator++(int) noexcept {// postfix
                const auto copy{ *this };
                ++(*this);
                return copy;
            }

            ConstIterator& operator++() noexcept {// prefix
                mAddress = (static_cast<std::uint8_t*>(mAddress) + mStepSize);
                return *this;
            }

            difference_type operator-(const ConstIterator& other) noexcept {
                return static_cast<difference_type>(static_cast<std::uint8_t*>(mAddress) -
                                                    static_cast<std::uint8_t*>(other.mAddress)) /
                       mStepSize;
            }

            [[nodiscard]] bool operator==(const ConstIterator& other) const noexcept {
                assert(mStepSize == other.mStepSize);
                return mAddress == other.mAddress;
            }

            [[nodiscard]] bool operator!=(const ConstIterator& other) const noexcept {
                return !(*this == other);
            }

            friend void swap(ConstIterator& lhs, ConstIterator& rhs) noexcept {
                std::swap(lhs.mAddress, rhs.mAddress);
                std::swap(lhs.mStepSize, rhs.mStepSize);
            }

            [[nodiscard]] reference operator*() const noexcept {
                return mAddress;
            }

            pointer operator->() const {
                return &mAddress;
            }

        protected:
            ConstIterator(void* address, std::size_t stepSize) noexcept : mAddress{ address }, mStepSize{ stepSize } { }

        protected:
            void* mAddress{ nullptr };
            std::size_t mStepSize{ 0 };

            friend class c2k::TypeErasedVector;
        };// ConstIterator

        template<std::default_initializable T>
        class TypedIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;

        public:
            TypedIterator() = default;

            TypedIterator(const TypedIterator& other) = default;

            TypedIterator(TypedIterator&& other) {
                mAddress = other.mAddress;
                mStepSize = other.mStepSize;
                other.mAddress = nullptr;
                other.mStepSize = 0;
            }

            TypedIterator& operator=(const TypedIterator& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                mAddress = other.mAddress;
                mStepSize = other.mStepSize;
                return *this;
            }

            TypedIterator& operator=(TypedIterator&& other) noexcept {
                swap(*this, other);
                return *this;
            }

            [[nodiscard]] TypedIterator operator+(std::size_t increment) noexcept {
                return TypedIterator{ static_cast<std::uint8_t*>(mAddress) + increment * mStepSize, mStepSize };
            }

            TypedIterator operator++(int) noexcept {// postfix
                const auto copy{ *this };
                ++(*this);
                return copy;
            }

            TypedIterator& operator++() noexcept {// prefix
                mAddress = (static_cast<std::uint8_t*>(mAddress) + mStepSize);
                return *this;
            }

            difference_type operator-(const TypedIterator& other) noexcept {
                return static_cast<difference_type>(static_cast<std::uint8_t*>(mAddress) -
                                                    static_cast<std::uint8_t*>(other.mAddress)) /
                       mStepSize;
            }

            [[nodiscard]] bool operator==(const TypedIterator& other) const noexcept {
                assert(mStepSize == other.mStepSize);
                return mAddress == other.mAddress;
            }

            [[nodiscard]] bool operator!=(const TypedIterator& other) const noexcept {
                return !(*this == other);
            }

            friend void swap(TypedIterator& lhs, TypedIterator& rhs) noexcept {
                std::swap(lhs.mAddress, rhs.mAddress);
                std::swap(lhs.mStepSize, rhs.mStepSize);
            }

            [[nodiscard]] reference operator*() const noexcept {
                return *static_cast<pointer>(mAddress);
            }

            pointer operator->() const {
                return static_cast<pointer>(mAddress);
            }

        protected:
            TypedIterator(void* address, std::size_t stepSize) noexcept : mAddress{ address }, mStepSize{ stepSize } { }

        protected:
            void* mAddress{ nullptr };
            std::size_t mStepSize{ 0 };

            friend class c2k::TypeErasedVector;
        };// TypedIterator

        template<std::default_initializable T>
        class ConstTypedIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;

        public:
            ConstTypedIterator() = default;

            ConstTypedIterator(const ConstTypedIterator& other) = default;

            ConstTypedIterator(ConstTypedIterator&& other) {
                mAddress = other.mAddress;
                mStepSize = other.mStepSize;
                other.mAddress = nullptr;
                other.mStepSize = 0;
            }

            ConstTypedIterator& operator=(const ConstTypedIterator& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                mAddress = other.mAddress;
                mStepSize = other.mStepSize;
                return *this;
            }

            ConstTypedIterator& operator=(ConstTypedIterator&& other) noexcept {
                swap(*this, other);
                return *this;
            }

            [[nodiscard]] ConstTypedIterator operator+(std::size_t increment) noexcept {
                return ConstTypedIterator{ static_cast<std::uint8_t*>(mAddress) + increment * mStepSize, mStepSize };
            }

            ConstTypedIterator operator++(int) noexcept {// postfix
                const auto copy{ *this };
                ++(*this);
                return copy;
            }

            ConstTypedIterator& operator++() noexcept {// prefix
                mAddress = (static_cast<std::uint8_t*>(mAddress) + mStepSize);
                return *this;
            }

            difference_type operator-(const ConstTypedIterator& other) noexcept {
                return static_cast<difference_type>(static_cast<std::uint8_t*>(mAddress) -
                                                    static_cast<std::uint8_t*>(other.mAddress)) /
                       mStepSize;
            }

            [[nodiscard]] bool operator==(const ConstTypedIterator& other) const noexcept {
                assert(mStepSize == other.mStepSize);
                return mAddress == other.mAddress;
            }

            [[nodiscard]] bool operator!=(const ConstTypedIterator& other) const noexcept {
                return !(*this == other);
            }

            friend void swap(ConstTypedIterator& lhs, ConstTypedIterator& rhs) noexcept {
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
            ConstTypedIterator(void* address, std::size_t stepSize) noexcept
                : mAddress{ address },
                  mStepSize{ stepSize } { }

        protected:
            void* mAddress{ nullptr };
            std::size_t mStepSize{ 0 };

            friend class c2k::TypeErasedVector;
        };// ConstTypedIterator

    }// namespace Iterators

}// namespace c2k