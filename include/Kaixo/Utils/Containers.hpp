#pragma once
#include <cstddef>
#include <utility>
#include <type_traits>

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------
        
    // Simple vector with no dynamic allocations, only for
    // trivial types, as I don't need anything more complicated
    template<class Ty, std::size_t Max>
        requires (std::is_trivially_copyable_v<Ty>)
    class Vector {
        using byte = std::uint8_t;
        constexpr static std::size_t bytes = sizeof(Ty) * Max;
    public:
        using value_type = Ty;

        constexpr Ty* begin() { return reinterpret_cast<Ty*>(_data); }
        constexpr const Ty* begin() const { return reinterpret_cast<const Ty*>(_data); }
        constexpr const Ty* cbegin() const { return reinterpret_cast<const Ty*>(_data); }
        constexpr Ty* end() { return _end; }
        constexpr const Ty* end() const { return _end; }
        constexpr const Ty* cend() const { return _end; }

        constexpr Ty& front() { return *begin(); }
        constexpr const Ty& front() const { return *begin(); }
        constexpr Ty& back() { return *(end() - 1); }
        constexpr const Ty& back() const { return *(end() - 1); }

        constexpr std::size_t index_of(const Ty& value) const {
            for (auto _it = cbegin(); _it != cend();) {
                if (*_it == value) return _it - begin();
            }
            return npos;
        }

        template<class ...Args>
        constexpr Ty& emplace_back(Args&&...args) {
            return *new(_end++) Ty{ std::forward<Args>(args)... };
        }

        template<class ...Args>
        constexpr Ty& emplace_front(Args&&...args) {
            std::memmove(begin() + 1, begin(), size() * sizeof(Ty));
            ++_end;
            return *new(begin()) Ty{ std::forward<Args>(args)... };
        }

        constexpr void push_back(const Ty& val) { new (_end++) Ty{ val }; }
        constexpr void push_back(Ty&& val) { new (_end++) Ty{ std::move(val) }; }
        
        constexpr void push_front(const Ty& val) {
            std::memmove(begin() + 1, begin(), size() * sizeof(Ty));
            new (begin()) Ty{ val };
            ++_end;
        }

        constexpr void push_front(Ty&& val) {
            std::memmove(begin() + 1, begin(), size() * sizeof(Ty));
            new (begin()) Ty{ std::move(val) };
            ++_end;
        }

        constexpr void pop_back() { --_end; }
        constexpr void pop_front() {
            --_end;
            std::memmove(begin(), begin() + 1, size() * sizeof(Ty));
        }

        constexpr const Ty* erase(const Ty* val) {
            std::ptrdiff_t index = val - begin();
            return erase_index(static_cast<std::size_t>(index));
        }
        
        constexpr const Ty* erase_index(std::size_t index) {
            --_end;
            std::memmove(begin() + index, begin() + index + 1, (size() - index) * sizeof(Ty));
            return begin() + index;
        }

        constexpr std::size_t remove(const Ty& value) {
            std::size_t i = 0;
            for (auto _it = cbegin(); _it != cend();) {
                if (*_it == value) _it = erase(_it), ++i;
                else ++_it;
            }
            return i;
        }
        
        constexpr bool remove_one(const Ty& value) {
            for (auto _it = cbegin(); _it != cend();) {
                if (*_it == value) {
                    erase(_it);
                    return true;
                } else ++_it;
            }
            return false;
        }

        constexpr void insert(const Ty* at, Ty&& value) {
            std::ptrdiff_t index = at - begin();
            std::memmove(begin() + index + 1, begin() + index, (size() - index) * sizeof(Ty));
            new (begin() + index) Ty{ std::move(value) };
            ++_end;
        }
        
        constexpr void insert(const Ty* at, const Ty& value) {
            std::ptrdiff_t index = at - begin();
            std::memmove(begin() + index + 1, begin() + index, (size() - index) * sizeof(Ty));
            new (begin() + index) Ty{ value };
            ++_end;
        }

        constexpr void clear() { _end = begin(); }

        constexpr std::size_t capacity() const { return Max; }
        constexpr std::size_t size() const { return end() - begin(); }
        constexpr bool empty() const { return size() == 0; }
        constexpr bool full() const { return size() == capacity(); }

        constexpr Ty& operator[](std::size_t i) { return *(begin() + i); }
        constexpr const Ty& operator[](std::size_t i) const { return *(begin() + i); }

    private:
        byte _data[bytes]{}; // storage
        Ty* _end = reinterpret_cast<Ty*>(_data); // current size
    };

    // ------------------------------------------------

    // Vector of bits where you can loop over indices.
    template<std::unsigned_integral Type, std::size_t Size>
    class StateVector {
    public:

        // ------------------------------------------------

        using value_type = Type;

        // ------------------------------------------------

        constexpr bool test(value_type i) const { return m_Bits.test(i); }

        // ------------------------------------------------

        constexpr void set(value_type i) {
            if (test(i)) return;

            m_Changing.push_back(i);
            m_Bits.set(i, true);
        }

        constexpr void unset(value_type i) {
            if (!test(i)) return;

            // Kinda expensive if at start and many entries because copy...
            m_Changing.remove_one(i);
            m_Bits.set(i, false);
        }

        // ------------------------------------------------

        constexpr void foreach(auto fun) {
            for (auto& value : m_Changing) fun(value);
        }

        // ------------------------------------------------

    private:
        Vector<value_type, Size> m_Changing{};
        std::bitset<Size> m_Bits{};

        // ------------------------------------------------

    };
}