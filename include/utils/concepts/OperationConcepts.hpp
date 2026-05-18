/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 18/05/2026 by @author Tsukini

File Name:
##  @file OperationConcepts.hpp

File Description:
##  Definition of the different operation concepts
\**************************************************************/

#ifndef OPERATIONCONCEPTS_H
    #define OPERATIONCONCEPTS_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <iostream> // std::convertible_to ????

namespace utils::concepts { // namespace start
//----------------------------------------------------------------//
/* CONCEPTS */

// ------- Basic-Arithmetic ------- //
template<typename T>
concept Addable = requires(T a, T b) {{a + b} -> std::convertible_to<T>;};

template<typename T, typename U>
concept AddableWith = requires(T a, U b) {a + b;};

template<typename T>
concept Subtractable = requires(T a, T b) {{a - b} -> std::convertible_to<T>;};

template<typename T, typename U>
concept SubtractableWith = requires(T a, U b) {a - b;};

template<typename T>
concept Multipliable = requires(T a, T b) {{a * b} -> std::convertible_to<T>;};

template<typename T, typename U>
concept MultipliableWith = requires(T a, U b) {a * b;};

template<typename T>
concept Divisible = requires(T a, T b) {{a / b} -> std::convertible_to<T>;};

template<typename T, typename U>
concept DivisibleWith = requires(T a, U b) {a / b;};

template<typename T>
concept Arithmetic = Addable<T> && Subtractable<T> && Multipliable<T> && Divisible<T>;

template<typename T, typename U>
concept ArithmeticWith = AddableWith<T, U> && SubtractableWith<T, U> && MultipliableWith<T, U> && DivisibleWith<T, U>;

template<typename T>
concept Incrementable = requires(T& a)
{
    {++a} -> std::same_as<T&>;
    {a++} -> std::same_as<T>;
};
template<typename T>
concept Decrementable = requires(T& a)
{
    {--a} -> std::same_as<T&>;
    {a--} -> std::same_as<T>;
};

// ------- Bitwise ------- //
template<typename T>
concept BitwiseAndable = requires(T a, T b) {{a & b} -> std::convertible_to<T>;};

template<typename T, typename U>
concept BitwiseAndableWith = requires(T a, U b) {a & b;};

template<typename T>
concept BitwiseOrable = requires(T a, T b) {{a | b} -> std::convertible_to<T>;};

template<typename T, typename U>
concept BitwiseOrableWith = requires(T a, U b) {a | b;};

template<typename T>
concept BitwiseXorable = requires(T a, T b) {{a ^ b} -> std::convertible_to<T>;};

template<typename T, typename U>
concept BitwiseXorableWith = requires(T a, U b) {a ^ b;};

template<typename T>
concept Shiftable = requires(T a, T b) {
    {a << b} -> std::convertible_to<T>;
    {a >> b} -> std::convertible_to<T>;
};

template<typename T, typename U>
concept ShiftableWith = requires(T a, U b) {
    {a << b} -> std::convertible_to<T>;
    {a >> b} -> std::convertible_to<T>;
};

// ----- Assignment-Operations ---- //
template<typename T>
concept AddAssignable = requires(T a, T b) {{a += b} -> std::same_as<T&>;};

template<typename T, typename U>
concept AddAssignableWith = requires(T a, U b) {{a += b} -> std::same_as<T&>;};

template<typename T>
concept SubtractAssignable = requires(T a, T b) {{a -= b} -> std::same_as<T&>;};

template<typename T, typename U>
concept SubtractAssignableWith = requires(T a, U b) {{a -= b} -> std::same_as<T&>;};

template<typename T>
concept MultiplyAssignable = requires(T a, T b) {{a *= b} -> std::same_as<T&>;};

template<typename T, typename U>
concept MultiplyAssignableWith = requires(T a, U b) {{a *= b} -> std::same_as<T&>;};

template<typename T>
concept DivideAssignable = requires(T a, T b) {{a /= b} -> std::same_as<T&>;};

template<typename T, typename U>
concept DivideAssignableWith = requires(T a, U b) {{a /= b} -> std::same_as<T&>;};

// ---------- Comparison ---------- //
template<typename T>
concept EqualityComparable = requires(T a, T b) {
    {a == b} -> std::convertible_to<bool>;
    {a != b} -> std::convertible_to<bool>;
};

template<typename T, typename U>
concept EqualityComparableWith = requires(T a, U b) {
    {a == b} -> std::convertible_to<bool>;
    {a != b} -> std::convertible_to<bool>;
};

template<typename T>
concept Comparable = requires(T a, T b) {
    {a < b} -> std::convertible_to<bool>;
    {a <= b} -> std::convertible_to<bool>;
    {a > b} -> std::convertible_to<bool>;
    {a >= b} -> std::convertible_to<bool>;
};

template<typename T, typename U>
concept ComparableWith = requires(T a, U b) {
    {a < b} -> std::convertible_to<bool>;
    {a <= b} -> std::convertible_to<bool>;
    {a > b} -> std::convertible_to<bool>;
    {a >= b} -> std::convertible_to<bool>;
};

// ------------- Unary ------------ //
template<typename T>
concept Negatable = requires(T a) {{-a} -> std::convertible_to<T>;};

} // namespace end
#endif /* OPERATIONCONCEPTS_H */
