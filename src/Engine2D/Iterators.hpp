#pragma once

#include <cassert>

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

    }// namespace Iterators

}// namespace c2k