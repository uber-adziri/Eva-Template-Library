#pragma once
#include <cstddef>

namespace etl {

// Helper class:
template <typename T, T v>
struct integral_constant {
    typedef T                 value_type;
    typedef integral_constant type;

    static T constexpr const value = v;
    constexpr value_type operator()() const noexcept { return value; }
};
template <typename T, T v>
T constexpr const integral_constant<T, v>::value;

typedef integral_constant<bool, true>  true_type;
typedef integral_constant<bool, false> false_type;

// Helper traits:
template <bool, typename T = void>
struct enable_if {};
template <typename T>
struct enable_if<true, T> { typedef T type; };

template <typename T>
struct void_t { typedef void type; };

//----------------------------------------------------------------------------------------------------------------------
// Allocator
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
struct allocator;

template <>
struct allocator<void> {
    typedef void*       pointer;
    typedef void const* const_pointer;
    typedef void        value_type;

    template <typename U>
    struct rebind { typedef allocator<U> other; };
};

template <>
struct allocator<void const> {
    typedef void const* pointer;
    typedef void const* const_pointer;
    typedef void const  value_type;

    template <typename U>
    struct rebind { typedef allocator<U> other; };
};

//----------------------------------------------------------------------------------------------------------------------
// Remove reference
//----------------------------------------------------------------------------------------------------------------------
template <typename T> struct remove_reference      { typedef T type; };
template <typename T> struct remove_reference<T&>  { typedef T type; };
template <typename T> struct remove_reference<T&&> { typedef _Tp type; };

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

//----------------------------------------------------------------------------------------------------------------------
// Pointer traits
//----------------------------------------------------------------------------------------------------------------------
template <typename T, typename U = void>
struct has_element_type : false_type {};
template <typename T>
struct has_element_type<T, typename void_t<typename T::element_type>::type> : true_type {};

template <typename Ptr, bool = has_element_type<Ptr>::value>
struct pointer_traits_element_type;
template <typename Ptr>
struct pointer_trairs_element_type {
    typedef typename Ptr::element_type type;
};

template <typename Ptr>
struct pointer_traits {
    typedef Ptr                                                 pointer;
    typedef typename pointer_traits_element_type<pointer>::type element_type;
};
template <typename T>
struct pointer_traits<T*> {
    typedef T* pointer;
    typedef T  element_type;
};

template <typename T, typename U>
struct has_rebind {
private:
    struct hidden { char l; char r; };
    template <typename X> static hidden test(...);
    template <typename X> static char   test(typename X::template rebind<U>* = 0);

public:
    static bool constexpr const value = sizeof(test<T>(0)) == 1;
};

template <typename T, typename U, bool = has_rebind<T, U>::value>
struct pointer_traits_rebind {
    typedef typename T::template rebind<U>::other type;
};

template <typename From, typename To>
struct rebind_pointer {
    typedef typename pointer_traits<From>::template rebind<To>::other type;
};

template <typename T, typename U = void>
struct has_pointer_type : false_type {};
template <typename T>
struct has_pointer_type<T, typename void_t<typename T::pointer>::type> : true_type {};

namespace poiter_type_impl {

template <typename T, typename U, bool = has_pointer_type<U>::value>
struct pointer_type {
    typedef typename U::pointer type;
};

template <typename T, typename U>
struct pointer_type<T, U, false> {
    typedef T* type;
};

} // namespace poiter_type_impl

template <typename T, typename U>
struct pointer_type {
    typedef typename poiter_type_impl::pointer_type<T, typename remove_reference<U>::type>::type type;
};

//----------------------------------------------------------------------------------------------------------------------
// Const pointer
//----------------------------------------------------------------------------------------------------------------------
template <typename T, typename U = void>
struct has_const_pointer : false_type {};

template <typename T>
struct has_const_pointer<T, typename void_t<typename T::const_pointer>::type> : true_type {};

template <typename T, typename Ptr, typename Alloc, bool = has_const_pointer<Alloc>::value>
struct const_pointer { typedef typename Alloc::const_pointer type; };

template <typename T, typename Ptr, typename Alloc>
struct const_pointer<T, Ptr, Alloc, false> {
    typedef typename pointer_traits<Ptr>::template rebind<T const>::other type;
};

//----------------------------------------------------------------------------------------------------------------------
// Type list
//----------------------------------------------------------------------------------------------------------------------
template <typename H, typename T>
struct type_list {
    typedef H head;
    typedef T tail;
};

struct nat {};

//----------------------------------------------------------------------------------------------------------------------
// Types
//----------------------------------------------------------------------------------------------------------------------
typedef type_list<unsigned char, type_list<unsigned int, type_list<unsigned long, nat>>> unsigned_types;

//----------------------------------------------------------------------------------------------------------------------
// Find first
//----------------------------------------------------------------------------------------------------------------------
template <typename TypeList, size_t Size, bool = Size <= sizeof(typename TypeList::head)>
struct find_first;

template <typename H, typename T, size_t Size>
struct find_first<type_list<H, T>, Size, true> {
    typedef T type;
};

template <typename H, typename T, size_t Size>
struct find_first<type_list<H, T>, Size, false> {
    typedef typename find_first<T, Size>::type type;
};

//----------------------------------------------------------------------------------------------------------------------
// Make unsigned
//----------------------------------------------------------------------------------------------------------------------
template <typename T, bool = true /* is_integral<T>::value || is_enum<Tp>::value */>
struct make_unsigned__ {};

template <typename T>
struct make_unsigned__<T, true> {
    typedef typename find_first<unsigned_types, sizeof(T)>::type type;
};

template <> struct make_unsigned__<bool, true> {};
template <> struct make_unsigned__<signed int, true>    { typedef unsigned int type; };
template <> struct make_unsigned__<unsigned int, true>  { typedef unsigned int type; };
template <> struct make_unsigned__<signed long, true>   { typedef unsigned long type; };
template <> struct make_unsigned__<unsigned long, true> { typedef unsigned long type; };

template <typename T>
struct make_unsigned {
    typedef typename make_unsigned__<T>::type type;
};

//----------------------------------------------------------------------------------------------------------------------
// Size type
//----------------------------------------------------------------------------------------------------------------------
template <typename T, typename U = void>
struct has_size_type : false_type {};

template <typename T>
struct has_size_type<T, typename void_t<typename T::size_type>::type> : true_type {};

template <typename Alloc, typename DiffType, bool = has_size_type<Alloc>::value>
struct size_type {
    typedef typename make_unsigned<DiffType>::type type;
};

template <typename Alloc, typename DiffType>
struct size_type<Alloc, DiffType, true> {
    typedef typename Alloc::size_type type;
};

//----------------------------------------------------------------------------------------------------------------------
// Difference type
//----------------------------------------------------------------------------------------------------------------------
template <typename T, typename U = void>
struct has_difference_type : false_type {};

template <typename T>
struct has_difference_type<T, typename void_t<typename T::difference_type>::type> : true_type {};

template <typename Ptr, bool = has_difference_type<Ptr>::value>
struct pointer_traits_difference_type {
    typedef ptrdiff_t type;
};

template <typename Ptr>
struct pointer_traits_difference_type<Ptr, true> {
    typedef typename Ptr::difference_type type;
};

template <class Alloc, class Ptr, bool = has_difference_type<Alloc>::value>
struct alloc_traits_difference_type {
    typedef typename pointer_traits<Ptr>::difference_type type;
};

template <class Alloc, class Ptr>
struct alloc_traits_difference_type<Alloc, Ptr, true> {
    typedef typename Alloc::difference_type type;
};

//----------------------------------------------------------------------------------------------------------------------
// Allocator traits
//----------------------------------------------------------------------------------------------------------------------
template <typename Alloc>
struct allocator_traits {
    typedef Alloc                               allocator_type;
    typedef typename allocator_type::value_type value_type;

    typedef typename pointer_type<value_type, allocator_type>::type           pointer;
    typedef typename const_pointer<value_type, pointer, allocator_type>::type const_pointer;

    typedef typename alloc_traits_difference_type<allocator_type, pointer>::type difference_type;
    typedef typename size_type<allocator_type, difference_type>::type            size_type;
};

} // namespace etl
