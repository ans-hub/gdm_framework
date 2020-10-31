// *************************************************************
// File:    helpers.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_HELPERS_H
#define AH_ECS_HELPERS_H

#include <array>
#include <cstddef>
#include <tuple>
#include <utility>

#include "system/hash_utils.h"

// Iterate over all variadic arguments in macro and call f (implementation details)
// Copyright (C) 2012 William Swanson (https://github.com/swansontec/map-macro)

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...)  EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,
#define MAP_XOR |
#define MAP_AND &&

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next)  MAP_NEXT1(MAP_GET_END test, next)

#define MAP0(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP1)(f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP0)(f, peek, __VA_ARGS__)

#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next)  MAP_LIST_NEXT1(MAP_GET_END test, next)

#define MAP_LIST0(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST1)(f, peek, __VA_ARGS__)
#define MAP_LIST1(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST0)(f, peek, __VA_ARGS__)

#define MAP_XOR_NEXT1(test, next) MAP_NEXT0(test, MAP_XOR next, 0)
#define MAP_XOR_NEXT(test, next)  MAP_XOR_NEXT1(MAP_GET_END test, next)

#define MAP_XOR0(f, x, peek, ...) f(x) MAP_XOR_NEXT(peek, MAP_XOR1)(f, peek, __VA_ARGS__)
#define MAP_XOR1(f, x, peek, ...) f(x) MAP_XOR_NEXT(peek, MAP_XOR0)(f, peek, __VA_ARGS__)

#define MAP_AND_NEXT1(test, next) MAP_NEXT0(test, MAP_AND next, 0)
#define MAP_AND_NEXT(test, next)  MAP_AND_NEXT1(MAP_GET_END test, next)

#define MAP_AND0(f, x, peek, ...) f(x) MAP_AND_NEXT(peek, MAP_AND1)(f, peek, __VA_ARGS__)
#define MAP_AND1(f, x, peek, ...) f(x) MAP_AND_NEXT(peek, MAP_AND0)(f, peek, __VA_ARGS__)

#define FOR_EACH(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))
#define FOR_EACH_LIST(f, ...) EVAL(MAP_LIST1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))
#define FOR_EACH_XOR(f, ...) EVAL(MAP_XOR1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))
#define FOR_EACH_AND(f, ...) EVAL(MAP_AND1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

namespace ecs {

namespace helpers {

// Iterates each argument in variadic template
// https://isocpp.org/blog/2015/01/for-each-argument-sean-parent

template <class F, class... Args>
void ForEachArgument(F f, Args&&... args) {
  [](...){}((f(std::forward<Args>(args)), 0)...);
}

// Concatenates std::arrays with different sizes
// https://stackoverflow.com/questions/42749032/concatenating-a-sequence-of-stdarrays

template <typename F, typename T, typename T2>
auto ConcatArr(F f, T&& t, T2&& t2)
{
  return f(std::forward<T>(t), std::forward<T2>(t2));
}
template <typename F, typename T, typename T2, typename ... Ts>
auto ConcatArr(F f, T&& t, T2&& t2, Ts&&...args)
{
  return ConcatArr(f, f(std::forward<T>(t), std::forward<T2>(t2)), std::forward<Ts>(args)...);
}
struct ConcatArr_t
{
  template<typename T, std::size_t N, std::size_t M>
  auto operator()(const std::array<T, N>& ar1, const std::array<T, M>& ar2) const
  {
    std::array<T,N+M> result;
    std::copy(ar1.cbegin(), ar1.cend(), result.begin());
    std::copy(ar2.cbegin(), ar2.cend(), result.begin() + N);
    return result;
  }
};

// Compile time foreach for tuple. I don't know why standart avoid partial
// function template specialization. Had to do in such way
// Usage (very ugly thing):
//  P<2, std::tuple<MoveComponent,Shield>>::CompileTimeXor(std::tuple<MoveComponent,Shield>{})>
//  P<1, std::tuple<int,unsigned>>::CompileTimeXor(std::tuple<int,unsigned>{});

template<std::size_t N, class Tuple>
struct P {
  constexpr static unsigned CompileTimeXor(Tuple&& t){
    return std::get<N>(t).Sig() | P<N-1,Tuple>::CompileTimeXor(std::move(t));
  }
};

template<class Tuple>
struct P<0,Tuple> {
  constexpr static unsigned CompileTimeXor(Tuple&& t){ 
    return std::get<0>(t).Sig();
  }
};

// Another one foreach tuple implementation

template <typename Tuple, typename F, std::size_t ...Indices>
constexpr void ForeachTupleImpl(Tuple&& tuple, F&& f, std::index_sequence<Indices...>) {
  using swallow = int[];
  (void)swallow{1,
      (f(std::get<Indices>(std::forward<Tuple>(tuple))), void(), int{})...
  };
}

template <typename Tuple, typename F>
constexpr void ForeachTuple(Tuple&& tuple, F&& f)
{
  constexpr std::size_t N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
  ForeachTupleImpl(std::forward<Tuple>(tuple), std::forward<F>(f), std::make_index_sequence<N>{});
}

// Helper class that assert on using ecs_register_... macroses in header files

template<class...Args>
struct RedefinitionAssert
{
  RedefinitionAssert()
  {
    static int val = 0;
    assert(val == 0 && "Redefinition detected, don't include ECS_REGISTER_... in header files"); // todo: legacy notice?
    val++;
  }
};

// Hash strings

#define ECS_HASH(str) GDM_HASH(str)

} // namespace helpers

} // namespace ecs

#endif // AH_ECS_HELPERS_H
