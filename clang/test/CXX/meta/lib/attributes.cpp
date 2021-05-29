// RUN: %clang_cc1 -std=c++2a -freflection %s

#include "../reflection_query.h"

using string_type = const char *;

constexpr bool string_eq(string_type s1, string_type s2) {
  while (*s1 != '\0' && *s1 == *s2) {
    s1++;
    s2++;
  }

  return *s1 == *s2;
}

consteval string_type name_of(meta::info reflection) {
  return __reflect(query_get_name, reflection);
}

consteval bool is_invalid(meta::info reflection) {
  return __reflect(query_is_invalid, reflection);
}

void no_attr_fn();

constexpr meta::info no_attr_fn_refl = reflexpr(no_attr_fn);
static_assert(!__reflect(query_has_attribute, no_attr_fn_refl, "noreturn"));

[[noreturn]] [[gnu::always_inline]] [[nodiscard("reason")]] int fn();

constexpr meta::info fn_refl = reflexpr(fn);
static_assert(__reflect(query_has_attribute, fn_refl, "noreturn"));
static_assert(__reflect(query_has_attribute, fn_refl, "nodiscard"));
static_assert(__reflect(query_has_attribute, fn_refl, "gnu::always_inline"));
static_assert(!__reflect(query_has_attribute, fn_refl, "deprecated"));

consteval bool indirect_has_attribute(meta::info fn_refl, const char *attribute_name) {
  return __reflect(query_has_attribute, fn_refl, attribute_name);
}

static_assert(indirect_has_attribute(fn_refl, "noreturn"));
static_assert(!indirect_has_attribute(fn_refl, "deprecated"));

constexpr meta::info noreturn = __reflect(query_get_begin_attribute, fn_refl);
static_assert(__reflect(query_is_attribute, noreturn));
static_assert(string_eq(name_of(noreturn), "noreturn"));

constexpr meta::info always_inline = __reflect(query_get_next_attribute, noreturn);
static_assert(__reflect(query_is_attribute, always_inline));
static_assert(string_eq(name_of(always_inline), "gnu::always_inline"));

constexpr meta::info nodiscard = __reflect(query_get_next_attribute, always_inline);
static_assert(__reflect(query_is_attribute, nodiscard));
static_assert(string_eq(name_of(nodiscard), "nodiscard"));

constexpr meta::info message = __reflect(query_get_begin_attribute_argument, nodiscard);
static_assert(string_eq(valueof(message), "reason"));

constexpr meta::info endarg = __reflect(query_get_next_attribute_argument, message);
static_assert(is_invalid(endarg));

constexpr meta::info end = __reflect(query_get_next_attribute, nodiscard);
static_assert(is_invalid(end));
