#pragma once

#include <type_traits>
#include <iterator>
#include <cassert>

namespace std {
    template <class _Ty, size_t _Size>
    class array;
}

namespace Ubpa {
	// reference : MSVC span
	
    inline constexpr std::size_t DynamicExtent = static_cast<std::size_t>(-1);

    // STRUCT TEMPLATE Span_iterator
    template <class _Ty>
    struct Span_iterator {
        using iterator_category = std::random_access_iterator_tag;
        using value_type = std::remove_cv_t<_Ty>;
        using difference_type = ptrdiff_t;
        using pointer = _Ty*;
        using reference = _Ty&;

        constexpr reference operator*() const noexcept {
            return *_Myptr;
        }

        constexpr pointer operator->() const noexcept {
            return _Myptr;
        }

        constexpr Span_iterator& operator++() noexcept {
            ++_Myptr;
            return *this;
        }

        constexpr Span_iterator operator++(int) noexcept {
            Span_iterator _Tmp{ *this };
            ++* this;
            return _Tmp;
        }

        constexpr Span_iterator& operator--() noexcept {
            --_Myptr;
            return *this;
        }

        constexpr Span_iterator operator--(int) noexcept {
            Span_iterator _Tmp{ *this };
            --* this;
            return _Tmp;
        }

        constexpr void _Verify_offset([[maybe_unused]] const difference_type _Off) const noexcept {
#ifndef NDEBUG
            if (_Off != 0) {
                assert(_Mybegin && "cannot seek value-initialized span iterator");
            }

            if (_Off < 0) {
                assert(_Myptr - _Mybegin >= -_Off && "cannot seek span iterator before begin");
            }

            if (_Off > 0) {
                assert(_Myend - _Myptr >= _Off && "cannot seek span iterator after end");
            }
#endif
        }

        constexpr Span_iterator& operator+=(const difference_type _Off) noexcept {
            _Verify_offset(_Off);
            _Myptr += _Off;
            return *this;
        }

        constexpr Span_iterator operator+(const difference_type _Off) const noexcept {
            Span_iterator _Tmp{ *this };
            _Tmp += _Off;
            return _Tmp;
        }

        friend constexpr Span_iterator operator+(const difference_type _Off, Span_iterator _Next) noexcept {
            return _Next += _Off;
        }

        constexpr Span_iterator& operator-=(const difference_type _Off) noexcept {
            _Verify_offset(-_Off);
            _Myptr -= _Off;
            return *this;
        }

        constexpr Span_iterator operator-(const difference_type _Off) const noexcept {
            Span_iterator _Tmp{ *this };
            _Tmp -= _Off;
            return _Tmp;
        }

        constexpr difference_type operator-(const Span_iterator& _Right) const noexcept {
            assert(_Mybegin == _Right._Mybegin && _Myend == _Right._Myend && "cannot subtract incompatible span iterators");
            return _Myptr - _Right._Myptr;
        }

        constexpr reference operator[](const difference_type _Off) const noexcept {
            return *(*this + _Off);
        }

        constexpr bool operator==(const Span_iterator& _Right) const noexcept {
            assert(_Mybegin == _Right._Mybegin && _Myend == _Right._Myend && "cannot compare incompatible span iterators for equality");
            return _Myptr == _Right._Myptr;
        }
    	
        constexpr bool operator!=(const Span_iterator& _Right) const noexcept {
            return !(*this == _Right);
        }

        constexpr bool operator<(const Span_iterator& _Right) const noexcept {
            assert(_Mybegin == _Right._Mybegin && _Myend == _Right._Myend && "cannot compare incompatible span iterators");
            return _Myptr < _Right._Myptr;
        }

        constexpr bool operator>(const Span_iterator& _Right) const noexcept {
            return _Right < *this;
        }

        constexpr bool operator<=(const Span_iterator& _Right) const noexcept {
            return !(_Right < *this);
        }

        constexpr bool operator>=(const Span_iterator& _Right) const noexcept {
            return !(*this < _Right);
        }

#ifndef NDEBUG
        friend constexpr void _Verify_range(const Span_iterator& _First, const Span_iterator& _Last) noexcept {
            assert(_First._Mybegin == _Last._Mybegin && _First._Myend == _Last._Myend &&
                "span iterators from different views do not form a range");
            assert(_First._Myptr <= _Last._Myptr && "span iterator range transposed");
        }
#endif // !NDEBUG

        using _Prevent_inheriting_unwrap = Span_iterator;

        [[nodiscard]] constexpr pointer _Unwrapped() const noexcept {
            return _Myptr;
        }

        static constexpr bool _Unwrap_when_unverified =
#ifdef NDEBUG
            true
#else
            false
#endif
            ;

        constexpr void _Seek_to(const pointer _It) noexcept {
            _Myptr = _It;
        }

        pointer _Myptr = nullptr;
#ifndef NDEBUG
        pointer _Mybegin = nullptr;
        pointer _Myend = nullptr;
#endif // !NDEBUG
    };

    //template <class _Ty>
    //struct pointer_traits<Span_iterator<_Ty>> {
    //    using pointer = Span_iterator<_Ty>;
    //    using element_type = _Ty;
    //    using difference_type = ptrdiff_t;

    //    static constexpr element_type* to_address(const pointer _Iter) noexcept {
    //        return _Iter._Unwrapped();
    //    }
    //};

    // STRUCT TEMPLATE Span_extent_type
    template <std::size_t _Extent>
    struct Span_extent_type {
        constexpr Span_extent_type() noexcept = default;

        constexpr explicit Span_extent_type(std::size_t) noexcept {}

        [[nodiscard]] constexpr std::size_t size() const noexcept {
            return _Extent;
        }
    };

    template <>
    struct Span_extent_type<DynamicExtent> {
        constexpr Span_extent_type() noexcept = default;

        constexpr explicit Span_extent_type(const size_t _Size) noexcept : _Mysize(_Size) {}

        [[nodiscard]] constexpr size_t size() const noexcept {
            return _Mysize;
        }

    private:
        size_t _Mysize{ 0 };
    };
	
    template <class _Ty, std::size_t _Extent>
    class Span;

    // STRUCT TEMPLATE IsSpan
    template <class>
    struct IsSpan : std::false_type {};

    template <class _Ty, std::size_t _Extent>
    struct IsSpan<Span<_Ty, _Extent>> : std::true_type {};

    // STRUCT TEMPLATE Is_std_array
    template <class>
    struct Is_std_array : std::false_type {};

    template <class _Ty, std::size_t _Size>
    struct Is_std_array<std::array<_Ty, _Size>> : std::true_type {};

    // STRUCT TEMPLATE IsSpan_convertible_range
    template <class _Rng, class _Ty>
    struct IsSpan_convertible_range
        : std::bool_constant<std::is_convertible_v<std::remove_pointer_t<decltype(std::data(std::declval<_Rng&>()))>(*)[], _Ty(*)[]>> {};

    // STRUCT TEMPLATE Has_container_interface
    template <class, class = void>
    struct Has_container_interface : std::false_type {};

    template <class _Rng>
    struct Has_container_interface<_Rng,
        std::void_t<decltype(std::data(std::declval<_Rng&>())), decltype(std::size(std::declval<_Rng&>()))>> : std::true_type {};

    // VARIABLE TEMPLATE IsSpan_compatible_range
    // clang-format off
    template <class _Rng, class _Ty>
    inline constexpr bool IsSpan_compatible_range = std::conjunction_v<
        std::negation<std::is_array<_Rng>>,
        std::negation<IsSpan<std::remove_const_t<_Rng>>>,
        std::negation<Is_std_array<std::remove_const_t<_Rng>>>,
        Has_container_interface<_Rng>,
        IsSpan_convertible_range<_Rng, _Ty>>;
    // clang-format on

// [views.span]
// CLASS TEMPLATE span
    template <class _Ty, std::size_t _Extent = DynamicExtent>
    class Span : public Span_extent_type<_Extent> {
    public:
        using element_type = _Ty;
        using value_type = std::remove_cv_t<_Ty>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = _Ty*;
        using const_pointer = const _Ty*;
        using reference = _Ty&;
        using const_reference = const _Ty&;
        using iterator = Span_iterator<_Ty>;
        using reverse_iterator = std::reverse_iterator<iterator>;

        static constexpr size_type extent = _Extent;

        // [span.cons] Constructors, copy, and assignment
        template <std::size_t _Ext = _Extent, std::enable_if_t<_Ext == 0 || _Ext == DynamicExtent, int> = 0>
        constexpr Span() noexcept {}

        constexpr Span(pointer _Ptr, size_type _Count) noexcept // strengthened
            : _Mybase(_Count), _Mydata(_Ptr) {
#ifndef NDEBUG
            if constexpr (_Extent != DynamicExtent) {
                assert(_Count == _Extent &&
                    "Cannot construct span with static extent from range [ptr, ptr + count) as count != extent");
            }
#endif // !NDEBUG
        }

        constexpr Span(pointer _First, pointer _Last) noexcept // strengthened
            : _Mybase(static_cast<size_type>(_Last - _First)), _Mydata(_First) {
            _Adl_verify_range(_First, _Last);
#ifndef NDEBUG
            if constexpr (_Extent != DynamicExtent) {
                assert(_Last - _First == _Extent &&
                    "Cannot construct span with static extent from range [first, last) as last - first != extent");
            }
#endif // !NDEBUG
        }

        template <std::size_t _Size, std::enable_if_t<_Extent == DynamicExtent || _Extent == _Size, int> = 0>
        constexpr Span(element_type(&_Arr)[_Size]) noexcept : _Mybase(_Size), _Mydata(_Arr) {}

        template <class _OtherTy, std::size_t _Size,
            std::enable_if_t<std::conjunction_v<std::bool_constant<_Extent == DynamicExtent || _Extent == _Size>,
            std::is_convertible<_OtherTy(*)[], element_type(*)[]>>,
            int> = 0>
            constexpr Span(std::array<_OtherTy, _Size>& _Arr) noexcept : _Mybase(_Size), _Mydata(_Arr.data()) {}

        template <class _OtherTy, std::size_t _Size,
            std::enable_if_t<std::conjunction_v<std::bool_constant<_Extent == DynamicExtent || _Extent == _Size>,
            std::is_convertible<const _OtherTy(*)[], element_type(*)[]>>,
            int> = 0>
            constexpr Span(const std::array<_OtherTy, _Size>& _Arr) noexcept : _Mybase(_Size), _Mydata(_Arr.data()) {}

        template <class _Rng, std::enable_if_t<IsSpan_compatible_range<_Rng, element_type>, int> = 0>
        constexpr Span(_Rng& _Range)
            : _Mybase(static_cast<size_type>(std::size(_Range))), _Mydata(std::data(_Range)) {
#ifndef NDEBUG
            if constexpr (_Extent != DynamicExtent) {
                assert(std::size(_Range) == _Extent &&
                    "Cannot construct span with static extent from range r as std::size(r) != extent");
            }
#endif // !NDEBUG
        }

        template <class _Rng, std::enable_if_t<IsSpan_compatible_range<const _Rng, element_type>, int> = 0>
        constexpr Span(const _Rng& _Range)
            : _Mybase(static_cast<size_type>(std::size(_Range))), _Mydata(std::data(_Range)) {
#ifndef NDEBUG
            if constexpr (_Extent != DynamicExtent) {
                assert(std::size(_Range) == _Extent &&
                    "Cannot construct span with static extent from range r as std::size(r) != extent");
            }
#endif // !NDEBUG
        }

        template <class _OtherTy, std::size_t _OtherExtent,
            std::enable_if_t<std::conjunction_v<std::bool_constant<_Extent == DynamicExtent || _OtherExtent == DynamicExtent
            || _Extent == _OtherExtent>,
            std::is_convertible<_OtherTy(*)[], element_type(*)[]>>,
            int> = 0>
            constexpr
            Span(const Span<_OtherTy, _OtherExtent>& _Other) noexcept
            : _Mybase(_Other.size()), _Mydata(_Other.data()) {
#ifndef NDEBUG
            if constexpr (_Extent != DynamicExtent) {
                assert(_Other.size() == _Extent &&
                    "Cannot construct span with static extent from other span as other.size() != extent");
            }
#endif // !NDEBUG
        }

        // [span.sub] Subviews
        template <std::size_t _Count>
        [[nodiscard]] constexpr auto first() const noexcept /* strengthened */ {
            if constexpr (_Extent != DynamicExtent) {
                static_assert(_Count <= _Extent, "Count out of range in span::first()");
            }
#ifndef NDEBUG
            else {
                assert(_Count <= this->size() && "Count out of range in span::first()");
            }
#endif // !NDEBUG
            return Span<element_type, _Count>{_Mydata, _Count};
        }

        [[nodiscard]] constexpr auto first(const size_type _Count) const noexcept
            /* strengthened */ {
#ifndef NDEBUG
            assert(_Count <= this->size() && "Count out of range in span::first(count)");
#endif // !NDEBUG
            return Span<element_type, DynamicExtent>{_Mydata, _Count};
        }

        template <std::size_t _Count>
        [[nodiscard]] constexpr auto last() const noexcept /* strengthened */ {
            if constexpr (_Extent != DynamicExtent) {
                static_assert(_Count <= _Extent, "Count out of range in span::last()");
            }
#ifndef NDEBUG
            else {
                assert(_Count <= this->size() && "Count out of range in span::last()");
            }
#endif // !NDEBUG
            return Span<element_type, _Count>{_Mydata + (this->size() - _Count), _Count};
        }

        [[nodiscard]] constexpr auto last(const size_type _Count) const noexcept /* strengthened */ {
#ifndef NDEBUG
            assert(_Count <= this->size() && "Count out of range in span::last(count)");
#endif // !NDEBUG
            return Span<element_type, DynamicExtent>{_Mydata + (this->size() - _Count), _Count};
        }

        template <std::size_t _Offset, std::size_t _Count = DynamicExtent>
        [[nodiscard]] constexpr auto subspan() const noexcept /* strengthened */ {
            if constexpr (_Extent != DynamicExtent) {
                static_assert(_Offset <= _Extent, "Offset out of range in span::subspan()");
                static_assert(
                    _Count == DynamicExtent || _Count <= _Extent - _Offset, "Count out of range in span::subspan()");
            }
#ifndef NDEBUG
            else {
                assert(_Offset <= this->size() && "Offset out of range in span::subspan()");

                if constexpr (_Count != DynamicExtent) {
                    assert(_Count <= this->size() - _Offset && "Count out of range in span::subspan()");
                }
            }
#endif // !NDEBUG
            using _ReturnType = Span<element_type,
                _Count != DynamicExtent ? _Count : (_Extent != DynamicExtent ? _Extent - _Offset : DynamicExtent)>;
            return _ReturnType{ _Mydata + _Offset, _Count == DynamicExtent ? this->size() - _Offset : _Count };
        }

        [[nodiscard]] constexpr auto subspan(const size_type _Offset, const size_type _Count = DynamicExtent) const noexcept
            /* strengthened */ {
#ifndef NDEBUG
            assert(_Offset <= this->size() && "Offset out of range in span::subspan(offset, count)");
            assert((_Count == DynamicExtent || _Count <= this->size() - _Offset) &&
                "Count out of range in span::subspan(offset, count)");
#endif // !NDEBUG
            using _ReturnType = Span<element_type, DynamicExtent>;
            return _ReturnType{ _Mydata + _Offset, _Count == DynamicExtent ? this->size() - _Offset : _Count };
        }

        // [span.obs] Observers
        [[nodiscard]] constexpr size_type size_bytes() const noexcept {
#ifndef NDEBUG
            assert(this->size() <= DynamicExtent / sizeof(element_type) &&
                "size of span in bytes exceeds std::numeric_limits<size_t>::max()");
#endif // !NDEBUG
            return this->size() * sizeof(element_type);
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return this->size() == 0;
        }

        // [span.elem] Element access
        constexpr reference operator[](const size_type _Off) const noexcept /* strengthened */ {
#ifndef NDEBUG
            assert(_Off < this->size() && "span index out of range");
#endif // !NDEBUG
            return _Mydata[_Off];
        }

        [[nodiscard]] constexpr reference front() const noexcept /* strengthened */ {
#ifndef NDEBUG
            assert(this->size() > 0 && "front of empty span");
#endif // !NDEBUG
            return _Mydata[0];
        }

        [[nodiscard]] constexpr reference back() const noexcept /* strengthened */ {
#ifndef NDEBUG
            assert(this->size() > 0 && "back of empty span");
#endif // !NDEBUG
            return _Mydata[this->size() - 1];
        }

        [[nodiscard]] constexpr pointer data() const noexcept {
            return _Mydata;
        }

        // [span.iterators] Iterator support
        [[nodiscard]] constexpr iterator begin() const noexcept {
#ifdef NDEBUG
            return { _Mydata };
#else
            return { _Mydata, _Mydata, _Mydata + this->size() };
#endif // NDEBUG
        }

        [[nodiscard]] constexpr iterator end() const noexcept {
            const auto _End = _Mydata + this->size();
#ifdef NDEBUG
            return { _End };
#else
            return { _End, _Mydata, _End };
#endif // NDEBUG
        }

        [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept {
            return reverse_iterator{ end() };
        }

        [[nodiscard]] constexpr reverse_iterator rend() const noexcept {
            return reverse_iterator{ begin() };
        }

        [[nodiscard]] constexpr pointer _Unchecked_begin() const noexcept {
            return _Mydata;
        }

        [[nodiscard]] constexpr pointer _Unchecked_end() const noexcept {
            return _Mydata + this->size();
        }

    private:
        using _Mybase = Span_extent_type<_Extent>;

        pointer _Mydata{ nullptr };
    };

    // DEDUCTION GUIDES
    template <class _Ty, std::size_t _Extent>
    Span(_Ty(&)[_Extent])->Span<_Ty, _Extent>;

    template <class _Ty, std::size_t _Size>
    Span(std::array<_Ty, _Size>&)->Span<_Ty, _Size>;

    template <class _Ty, std::size_t _Size>
    Span(const std::array<_Ty, _Size>&)->Span<const _Ty, _Size>;

    template <class _Rng>
    Span(_Rng&)->Span<typename _Rng::value_type>;

    template <class _Rng>
    Span(const _Rng&)->Span<const typename _Rng::value_type>;

#ifdef __cpp_lib_byte
    // [span.objectrep] Views of object representation
    // FUNCTION TEMPLATE as_bytes
    template <class _Ty, std::size_t _Extent>
    auto as_bytes(Span<_Ty, _Extent> _Sp) noexcept {
        using _ReturnType = Span<const std::byte, _Extent == DynamicExtent ? DynamicExtent : sizeof(_Ty) * _Extent>;
        return _ReturnType{ reinterpret_cast<const std::byte*>(_Sp.data()), _Sp.size_bytes() };
    }

    // FUNCTION TEMPLATE as_writable_bytes
    template <class _Ty, std::size_t _Extent, std::enable_if_t<!std::is_const_v<_Ty>, int> = 0>
    auto as_writable_bytes(Span<_Ty, _Extent> _Sp) noexcept {
        using _ReturnType = Span<std::byte, _Extent == DynamicExtent ? DynamicExtent : sizeof(_Ty) * _Extent>;
        return _ReturnType{ reinterpret_cast<std::byte*>(_Sp.data()), _Sp.size_bytes() };
    }
#endif // __cpp_lib_byte
}
