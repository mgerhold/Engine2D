//
// Created by coder2k on 26.08.2021.
//

#pragma once

namespace c2k {

    template<typename Iterator, typename Predicate, typename GetPointersFunc>
    class ConstComponentHolderPairIterator final {
    public:
        class ConstPair final {
        public:
            [[nodiscard]] const void* get(std::size_t index) const noexcept {
                assert(index < mPointers.size());
                return mPointers[index];
            }

            [[nodiscard]] std::size_t index() const noexcept {
                return mIndex;
            }

        private:
            std::size_t mIndex;
            std::vector<const void*> mPointers;

            friend class ConstComponentHolderPairIterator;
        };

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = ConstPair;
        using pointer = ConstPair;
        using reference = ConstPair;

    public:
        ConstComponentHolderPairIterator(Iterator begin,
                                         Iterator end,
                                         Predicate predicate,
                                         GetPointersFunc getPointersFunc,
                                         std::size_t offset = 0)
            : mCurrent{ begin + offset },
              mBegin{ begin },
              mEnd{ end },
              mPredicate{ predicate },
              mGetPointersFunc{ getPointersFunc } { }

        [[nodiscard]] bool operator==(const ConstComponentHolderPairIterator& other) const noexcept {
            return mBegin == other.mBegin && mEnd == other.mEnd && mCurrent == other.mCurrent;
        }

        [[nodiscard]] bool operator!=(const ConstComponentHolderPairIterator& other) const noexcept {
            return !((*this) == other);
        }

        [[nodiscard]] value_type operator*() const noexcept {
            using ranges::views::transform, ranges::to_vector;
            ConstPair result;
            result.mIndex = (mIndexTransformer == nullptr ? *mCurrent : mIndexTransformer(*mCurrent));
            result.mPointers = mGetPointersFunc(*mCurrent);
            return result;
        }

        [[nodiscard]] pointer operator->() const noexcept { }

        ConstComponentHolderPairIterator operator++(int) noexcept {// postfix
            const auto copy{ *this };
            ++(*this);
            return copy;
        }

        ConstComponentHolderPairIterator& operator++() noexcept {// prefix
            assert(mBegin != mEnd && "no more elements");
            while (true) {
                ++mCurrent;
                if (mCurrent == mEnd || mPredicate(*mCurrent)) {
                    break;
                }
            }
            return *this;
        }

        void setIndexTransformerFunc(std::function<std::size_t(std::size_t)> func) noexcept {
            mIndexTransformer = std::move(func);
        }

    private:
        Iterator mCurrent;
        const Iterator mBegin;// iterator over entities
        const Iterator mEnd;  // iterator over entities
        const Predicate mPredicate;
        const GetPointersFunc mGetPointersFunc;
        std::function<std::size_t(std::size_t)> mIndexTransformer{ nullptr };
    };

    template<typename Iterator, typename Predicate, typename GetPointersFunc>
    class ComponentHolderPairIterator final {
    public:
        class MutablePair final {
        public:
            [[nodiscard]] void* get(std::size_t index) const noexcept {
                assert(index < mPointers.size());
                return mPointers[index];
            }

            [[nodiscard]] std::size_t index() const noexcept {
                return mIndex;
            }

        private:
            std::size_t mIndex;
            std::vector<void*> mPointers;

            friend class ComponentHolderPairIterator;
        };

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = MutablePair;
        using pointer = MutablePair;
        using reference = MutablePair;

    public:
        ComponentHolderPairIterator(Iterator begin,
                                    Iterator end,
                                    Predicate predicate,
                                    GetPointersFunc getPointersFunc,
                                    std::size_t offset = 0)
            : mCurrent{ begin + offset },
              mBegin{ begin },
              mEnd{ end },
              mPredicate{ predicate },
              mGetPointersFunc{ getPointersFunc } { }

        [[nodiscard]] bool operator==(const ComponentHolderPairIterator& other) const noexcept {
            return mBegin == other.mBegin && mEnd == other.mEnd && mCurrent == other.mCurrent;
        }

        [[nodiscard]] bool operator!=(const ComponentHolderPairIterator& other) const noexcept {
            return !((*this) == other);
        }

        [[nodiscard]] value_type operator*() const noexcept {
            using ranges::views::transform, ranges::to_vector;
            MutablePair result;
            result.mIndex = (mIndexTransformer == nullptr ? *mCurrent : mIndexTransformer(*mCurrent));
            result.mPointers = mGetPointersFunc(*mCurrent);
            return result;
        }

        [[nodiscard]] pointer operator->() const noexcept { }

        ComponentHolderPairIterator operator++(int) noexcept {// postfix
            const auto copy{ *this };
            ++(*this);
            return copy;
        }

        ComponentHolderPairIterator& operator++() noexcept {// prefix
            assert(mBegin != mEnd && "no more elements");
            while (true) {
                ++mCurrent;
                if (mCurrent == mEnd || mPredicate(*mCurrent)) {
                    break;
                }
            }
            return *this;
        }

        void setIndexTransformerFunc(std::function<std::size_t(std::size_t)> func) noexcept {
            mIndexTransformer = std::move(func);
        }

    private:
        Iterator mCurrent;
        const Iterator mBegin;// iterator over entities
        const Iterator mEnd;  // iterator over entities
        const Predicate mPredicate;
        const GetPointersFunc mGetPointersFunc;
        std::function<std::size_t(std::size_t)> mIndexTransformer{ nullptr };
    };

}// namespace c2k