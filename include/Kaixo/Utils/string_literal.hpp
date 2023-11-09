#pragma once
#include <cstddef>
namespace Kaixo {
    template<std::size_t N, class CharType = char>
    struct string_literal {
        constexpr static std::size_t npos = std::basic_string_view<CharType>::npos;

        using view_type = std::basic_string_view<CharType>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using value_type = CharType;
        using reference = CharType&;
        using const_reference = const CharType&;
        using pointer = CharType*;
        using const_pointer = const CharType*;

        class const_iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using value_type = const CharType;
            using reference = const CharType&;

            constexpr const_iterator(const const_iterator&) = default;
            constexpr const_iterator(const_iterator&&) = default;
            constexpr const_iterator& operator=(const const_iterator&) = default;
            constexpr const_iterator& operator=(const_iterator&&) = default;
            constexpr const_iterator() : m_Ptr(nullptr) {}
            constexpr const_iterator(const CharType* ptr) : m_Ptr(ptr) {}

            constexpr reference operator*() const { return *m_Ptr; }
            constexpr const_iterator& operator+=(difference_type d) { m_Ptr += d; return *this; }
            constexpr const_iterator& operator-=(difference_type d) { m_Ptr -= d; return *this; }
            constexpr const_iterator& operator++() { ++m_Ptr; return *this; }
            constexpr const_iterator& operator--() { --m_Ptr; return *this; }
            constexpr const_iterator operator++(int) { auto _c = *this; ++m_Ptr; return _c; }
            constexpr const_iterator operator--(int) { auto _c = *this; --m_Ptr; return _c; }

            constexpr reference operator[](difference_type d) const { return m_Ptr[d]; }

            constexpr auto operator<=>(const const_iterator& other) const = default;

            friend constexpr const_iterator operator+(difference_type a, const const_iterator& b) { return { a + b.m_Ptr }; }
            friend constexpr const_iterator operator+(const const_iterator& a, difference_type b) { return { a.m_Ptr + b }; }
            friend constexpr const_iterator operator-(difference_type a, const const_iterator& b) { return { a - b.m_Ptr }; }
            friend constexpr const_iterator operator-(const const_iterator& a, difference_type b) { return { a.m_Ptr - b }; }
            friend constexpr difference_type operator-(const const_iterator& a, const const_iterator& b) { return a.m_Ptr - b.m_Ptr; }
        protected:
            const CharType* m_Ptr;
        };

        class iterator : public const_iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using value_type = CharType;
            using reference = CharType&;

            constexpr iterator(const iterator&) = default;
            constexpr iterator(iterator&&) = default;
            constexpr iterator& operator=(const iterator&) = default;
            constexpr iterator& operator=(iterator&&) = default;
            constexpr iterator() : const_iterator(nullptr) {}
            constexpr iterator(CharType* ptr) : const_iterator(ptr) {}

            constexpr reference operator*() const { return *const_cast<CharType*>(this->m_Ptr); }
            constexpr iterator& operator+=(difference_type d) { this->m_Ptr += d; return *this; }
            constexpr iterator& operator-=(difference_type d) { this->m_Ptr -= d; return *this; }
            constexpr iterator& operator++() { ++this->m_Ptr; return *this; }
            constexpr iterator& operator--() { --this->m_Ptr; return *this; }
            constexpr iterator operator++(int) { auto _c = *this; ++this->m_Ptr; return _c; }
            constexpr iterator operator--(int) { auto _c = *this; --this->m_Ptr; return _c; }

            constexpr reference operator[](difference_type d) const { return const_cast<CharType*>(this->m_Ptr)[d]; }

            constexpr auto operator<=>(const iterator& other) const = default;

            friend constexpr iterator operator+(difference_type a, const iterator& b) { return { a + b.m_Ptr }; }
            friend constexpr iterator operator+(const iterator& a, difference_type b) { return { a.m_Ptr + b }; }
            friend constexpr iterator operator-(difference_type a, const iterator& b) { return { a - b.m_Ptr }; }
            friend constexpr iterator operator-(const iterator& a, difference_type b) { return { a.m_Ptr - b }; }
            friend constexpr difference_type operator-(const iterator& a, const iterator& b) { return a.m_Ptr - b.m_Ptr; }
        };

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr ~string_literal() = default;
        constexpr string_literal() = default;
        constexpr string_literal(const CharType(&data)[N]) {
            std::copy_n(data, N, m_Data);
        }

        constexpr string_literal(string_literal&&) = default;
        constexpr string_literal(const string_literal&) = default;
        constexpr string_literal& operator=(string_literal&&) = default;
        constexpr string_literal& operator=(const string_literal&) = default;

        template<std::size_t I> requires (I < N)
            constexpr string_literal& operator=(const CharType(&data)[I]) {
            std::copy_n(data, I, m_Data);
        }

        constexpr iterator begin() { return { m_Data }; }
        constexpr iterator end() { return { m_Data + size() }; }
        constexpr const_iterator begin() const { return { m_Data }; }
        constexpr const_iterator end() const { return { m_Data + size() }; }
        constexpr const_iterator cbegin() const { return begin(); }
        constexpr const_iterator cend() const { return end(); }
        constexpr reverse_iterator rbegin() { return end(); }
        constexpr reverse_iterator rend() { return begin(); }
        constexpr const_reverse_iterator rbegin() const { return end(); }
        constexpr const_reverse_iterator rend() const { return begin(); }
        constexpr const_reverse_iterator crbegin() const { return end(); }
        constexpr const_reverse_iterator crend() const { return begin(); }

        constexpr reference at(size_type d) { return m_Data[d]; }
        constexpr const_reference at(size_type d) const { return m_Data[d]; }
        constexpr reference operator[](size_type d) { return m_Data[d]; }
        constexpr const_reference operator[](size_type d) const { return m_Data[d]; }
        constexpr reference front() { return m_Data[0]; }
        constexpr const_reference front() const { return m_Data[0]; }
        constexpr reference back() { return m_Data[size() - 1]; }
        constexpr const_reference back() const { return m_Data[size() - 1]; }

        constexpr pointer data() { return m_Data; }
        constexpr const_pointer data() const { return m_Data; }
        constexpr const_pointer c_str() const { return m_Data; }
        constexpr size_type size() const { return N - 1; }
        constexpr size_type length() const { return size(); }
        constexpr size_type max_size() const { return size(); }
        constexpr bool empty() const { return false; }
        constexpr void swap(string_literal& other) { std::swap(data(), other.data()); }

        constexpr view_type view() const { return { data(), size() }; }
        constexpr operator view_type() const { return { data(), size() }; }

        template<std::size_t I>
        constexpr auto operator==(const string_literal<I, CharType>& other) const {
            if constexpr (I != N) return false;
            else return view() == other.view();
        };
        template<std::size_t I>
        constexpr auto operator<=>(const string_literal<I, CharType>& other) const { return view() <=> other.view(); }

        constexpr auto starts_with(view_type t) const { return view().starts_with(t); }
        constexpr auto ends_with(view_type t) const { return view().ends_with(t); }
        constexpr auto substr(size_type pos = 0, size_type count = npos) const { return view().substr(pos, count); }
        constexpr auto find(std::string_view t, size_type pos = 0) const { return view().find(t, pos); }
        constexpr auto rfind(view_type t, size_type pos = 0) const { return view().rfind(t, pos); }
        constexpr auto find_first_of(view_type t, size_type pos = 0) const { return view().find_first_of(t, pos); }
        constexpr auto find_first_not_of(view_type t, size_type pos = 0) const { return view().find_first_not_of(t, pos); }
        constexpr auto find_last_of(view_type t, size_type pos = 0) const { return view().find_last_of(t, pos); }
        constexpr auto find_last_not_of(view_type t, size_type pos = 0) const { return view().find_last_not_of(t, pos); }

        CharType m_Data[N]{};
    };

    template<std::size_t N>
    struct wstring_literal : public string_literal<N, wchar_t> {
        using string_literal<N, wchar_t>::string_literal;

        constexpr wstring_literal(const wchar_t(&data)[N]) {
            std::copy_n(data, N, this->m_Data);
        }
    };


    template<class CharType>
    struct basic_constexpr_string {
        constexpr static std::size_t MaxSize = 32;
        constexpr static std::size_t npos = std::basic_string_view<CharType>::npos;

        using view_type = std::basic_string_view<CharType>;
        using string_type = std::basic_string<CharType>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using value_type = CharType;
        using reference = CharType&;
        using const_reference = const CharType&;
        using pointer = CharType*;
        using const_pointer = const CharType*;

        class const_iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using value_type = const CharType;
            using reference = const CharType&;

            constexpr const_iterator(const const_iterator&) = default;
            constexpr const_iterator(const_iterator&&) = default;
            constexpr const_iterator& operator=(const const_iterator&) = default;
            constexpr const_iterator& operator=(const_iterator&&) = default;
            constexpr const_iterator() : m_Ptr(nullptr) {}
            constexpr const_iterator(const CharType* ptr) : m_Ptr(ptr) {}

            constexpr reference operator*() const { return *m_Ptr; }
            constexpr const_iterator& operator+=(difference_type d) { m_Ptr += d; return *this; }
            constexpr const_iterator& operator-=(difference_type d) { m_Ptr -= d; return *this; }
            constexpr const_iterator& operator++() { ++m_Ptr; return *this; }
            constexpr const_iterator& operator--() { --m_Ptr; return *this; }
            constexpr const_iterator operator++(int) { auto _c = *this; ++m_Ptr; return _c; }
            constexpr const_iterator operator--(int) { auto _c = *this; --m_Ptr; return _c; }

            constexpr reference operator[](difference_type d) const { return m_Ptr[d]; }

            constexpr auto operator<=>(const const_iterator& other) const = default;

            friend constexpr const_iterator operator+(difference_type a, const const_iterator& b) { return { a + b.m_Ptr }; }
            friend constexpr const_iterator operator+(const const_iterator& a, difference_type b) { return { a.m_Ptr + b }; }
            friend constexpr const_iterator operator-(difference_type a, const const_iterator& b) { return { a - b.m_Ptr }; }
            friend constexpr const_iterator operator-(const const_iterator& a, difference_type b) { return { a.m_Ptr - b }; }
            friend constexpr difference_type operator-(const const_iterator& a, const const_iterator& b) { return a.m_Ptr - b.m_Ptr; }
        protected:
            const CharType* m_Ptr;
        };

        class iterator : public const_iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;
            using value_type = CharType;
            using reference = CharType&;

            constexpr iterator(const iterator&) = default;
            constexpr iterator(iterator&&) = default;
            constexpr iterator& operator=(const iterator&) = default;
            constexpr iterator& operator=(iterator&&) = default;
            constexpr iterator() : const_iterator(nullptr) {}
            constexpr iterator(CharType* ptr) : const_iterator(ptr) {}

            constexpr reference operator*() const { return *const_cast<CharType*>(this->m_Ptr); }
            constexpr iterator& operator+=(difference_type d) { this->m_Ptr += d; return *this; }
            constexpr iterator& operator-=(difference_type d) { this->m_Ptr -= d; return *this; }
            constexpr iterator& operator++() { ++this->m_Ptr; return *this; }
            constexpr iterator& operator--() { --this->m_Ptr; return *this; }
            constexpr iterator operator++(int) { auto _c = *this; ++this->m_Ptr; return _c; }
            constexpr iterator operator--(int) { auto _c = *this; --this->m_Ptr; return _c; }

            constexpr reference operator[](difference_type d) const { return const_cast<CharType*>(this->m_Ptr)[d]; }

            constexpr auto operator<=>(const iterator& other) const = default;

            friend constexpr iterator operator+(difference_type a, const iterator& b) { return { a + b.m_Ptr }; }
            friend constexpr iterator operator+(const iterator& a, difference_type b) { return { a.m_Ptr + b }; }
            friend constexpr iterator operator-(difference_type a, const iterator& b) { return { a - b.m_Ptr }; }
            friend constexpr iterator operator-(const iterator& a, difference_type b) { return { a.m_Ptr - b }; }
            friend constexpr difference_type operator-(const iterator& a, const iterator& b) { return a.m_Ptr - b.m_Ptr; }
        };

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr basic_constexpr_string() {};
        template<std::size_t N> requires (N < MaxSize)
        constexpr basic_constexpr_string(const CharType(&str)[N]) { std::copy_n(str, N, m_Data); }
        constexpr basic_constexpr_string(const string_type& str) { std::copy_n(str.data(), str.size(), m_Data); }
        constexpr basic_constexpr_string(view_type str) { std::copy_n(str.data(), str.size(), m_Data); }

        constexpr const CharType* null_terminator() const {
            const CharType* _c = m_Data;
            while (*_c != '\0') _c++;
            return _c;
        }

        constexpr CharType* null_terminator() {
            CharType* _c = m_Data;
            while (*_c != '\0') _c++;
            return _c;
        }

        template<std::size_t N>
        constexpr basic_constexpr_string operator+(const CharType(&str)[N]) const {
            basic_constexpr_string res = *this;
            std::copy_n(str, N, res.null_terminator());
            return res;
        }

        constexpr basic_constexpr_string operator+(view_type str) const {
            basic_constexpr_string res = *this;
            std::copy_n(str.data(), str.size(), res.null_terminator());
            return res;
        }

        constexpr basic_constexpr_string operator+(const string_type& str) const {
            basic_constexpr_string res = *this;
            std::copy_n(str.data(), str.size(), res.null_terminator());
            return res;
        }

        template<std::integral Ty>
        constexpr basic_constexpr_string operator+(Ty nmr) const {
            basic_constexpr_string res = *this;
            string_type _str;
            while (nmr > 0) {
                _str += '0' + (nmr % 10);
                nmr /= 10;
            }
            std::copy_n(_str.data(), _str.size(), res.null_terminator());
            return res;
        }

        constexpr iterator begin() { return { m_Data }; }
        constexpr iterator end() { return { m_Data + size() }; }
        constexpr const_iterator begin() const { return { m_Data }; }
        constexpr const_iterator end() const { return { m_Data + size() }; }
        constexpr const_iterator cbegin() const { return begin(); }
        constexpr const_iterator cend() const { return end(); }
        constexpr reverse_iterator rbegin() { return end(); }
        constexpr reverse_iterator rend() { return begin(); }
        constexpr const_reverse_iterator rbegin() const { return end(); }
        constexpr const_reverse_iterator rend() const { return begin(); }
        constexpr const_reverse_iterator crbegin() const { return end(); }
        constexpr const_reverse_iterator crend() const { return begin(); }

        constexpr reference at(size_type d) { return m_Data[d]; }
        constexpr const_reference at(size_type d) const { return m_Data[d]; }
        constexpr reference operator[](size_type d) { return m_Data[d]; }
        constexpr const_reference operator[](size_type d) const { return m_Data[d]; }
        constexpr reference front() { return m_Data[0]; }
        constexpr const_reference front() const { return m_Data[0]; }
        constexpr reference back() { return m_Data[size() - 1]; }
        constexpr const_reference back() const { return m_Data[size() - 1]; }

        constexpr pointer data() { return m_Data; }
        constexpr const_pointer data() const { return m_Data; }
        constexpr const_pointer c_str() const { return m_Data; }
        constexpr size_type size() const { return null_terminator() - begin(); }
        constexpr size_type length() const { return size(); }
        constexpr size_type max_size() const { return size(); }
        constexpr bool empty() const { return size() == 0; }
        constexpr void swap(basic_constexpr_string& other) { std::swap(m_Data, other.m_Data); }

        constexpr view_type view() const { return { data(), size() }; }
        constexpr operator view_type() const { return { data(), size() }; }
        
        constexpr string_type string() const { return { data(), size() }; }
        constexpr operator string_type() const { return { data(), size() }; }

        constexpr auto operator==(const basic_constexpr_string& other) const { return view() == other.view(); }
        constexpr auto operator<=>(const basic_constexpr_string& other) const { return view() <=> other.view(); }

        constexpr auto starts_with(view_type t) const { return view().starts_with(t); }
        constexpr auto ends_with(view_type t) const { return view().ends_with(t); }
        constexpr auto substr(size_type pos = 0, size_type count = npos) const { return view().substr(pos, count); }
        constexpr auto find(std::string_view t, size_type pos = 0) const { return view().find(t, pos); }
        constexpr auto rfind(view_type t, size_type pos = 0) const { return view().rfind(t, pos); }
        constexpr auto find_first_of(view_type t, size_type pos = 0) const { return view().find_first_of(t, pos); }
        constexpr auto find_first_not_of(view_type t, size_type pos = 0) const { return view().find_first_not_of(t, pos); }
        constexpr auto find_last_of(view_type t, size_type pos = 0) const { return view().find_last_of(t, pos); }
        constexpr auto find_last_not_of(view_type t, size_type pos = 0) const { return view().find_last_not_of(t, pos); }

        constexpr void replace(view_type replace, view_type with) {
            const std::size_t index = find(replace);
            if (index != npos) {
                basic_constexpr_string result{};
                const std::size_t replace_size = replace.size();
                const std::size_t with_size = with.size();
                const std::size_t orig_size = size();
                // Copy first part over
                for (std::size_t i = 0; i < index; ++i) 
                    result.m_Data[i] = m_Data[i];
                // Insert new data in place of old data
                for (std::size_t i = index; i < index + with_size; ++i)
                    result.m_Data[i] = with[i - index];
                // Copy last part over
                for (std::size_t i = index; i < orig_size - replace_size; ++i)
                    result.m_Data[i + with_size] = m_Data[i + replace_size];
                // Save result to this
                *this = result;
            }
        }

        CharType m_Data[MaxSize]{ '\0' };
    };

    template<class Char>
    constexpr void replace(std::basic_string<Char>& str, std::basic_string_view<Char> replace, std::basic_string_view<Char> with) {
        const std::size_t index = str.find(replace);
        if (index != str.npos) {
            std::basic_string<Char> result{};
            const std::size_t replace_size = replace.size();
            const std::size_t with_size = with.size();
            const std::size_t orig_size = str.size();
            result.resize(orig_size - replace_size + with_size);
            // Copy first part over
            for (std::size_t i = 0; i < index; ++i)
                result[i] = str[i];
            // Insert new data in place of old data
            for (std::size_t i = index; i < index + with_size; ++i)
                result[i] = with[i - index];
            // Copy last part over
            for (std::size_t i = index; i < orig_size - replace_size; ++i)
                result[i + with_size] = str[i + replace_size];
            // Save result to this
            str = result;
        }
    }

    using constexpr_string = basic_constexpr_string<char>;
}