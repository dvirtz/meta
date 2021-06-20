// RUN: %clang_cc1 -std=c++2a -freflection %s

#include "../reflection_query.h"

#define FWD(x) static_cast<decltype(x)&&>(x)

using string_type = const char *;

constexpr bool string_eq(string_type s1, string_type s2) {
  while (*s1 != '\0' && *s1 == *s2) {
    s1++;
    s2++;
  }

  return *s1 == *s2;
}

namespace std {
  inline namespace v1 {
    template<typename T> struct basic_ostream;
  }
  typedef basic_ostream<char> ostream;
  extern ostream cout;
  template<typename T>
  ostream &operator<<(ostream &out, T&&);
}

consteval string_type name_of(meta::info reflection) {
  return __reflect(query_get_name, reflection);
}

consteval bool is_valid(meta::info reflection) {
  return !__reflect(query_is_invalid, reflection);
}

consteval void decorate(meta::info source) {
  for (auto member = __reflect(query_get_begin_member, source);
       is_valid(member); member = __reflect(query_get_next_member, member)) {
    if (__reflect(query_has_attribute, member, "decorator") && __reflect(query_is_function, member)) {
      for (auto attribute = __reflect(query_get_begin_attribute, member); is_valid(attribute); 
           attribute = __reflect(query_get_next_attribute, attribute)) {
        if (string_eq(name_of(attribute), "decorator")) {
          for (auto decorator = __reflect(query_get_begin_attribute_argument, attribute); 
               is_valid(decorator); decorator = __reflect(query_get_next_attribute_argument, decorator)) {
            ->fragment struct {
              template <typename... Args>
              static auto unqualid(%{member})(Args&&... args) {
                return idexpr(%{decorator})(idexpr(%{member}))(FWD(args)...);
              }
            };
          }
          __reflect_mod(query_set_new_name, member, __concatenate("orig_", name_of(member)));
          ->member;
        }
      }
    }
  }
}

template <typename F>
auto logger(F&& f) {
  return [f]<typename... Args>(Args && ... args) {
    std::cout << "calling " << name_of(reflexpr(f));
    std::cout << "(";
    (void)(std::cout << ... << args);
    std::cout << ")\n";
    return f(FWD(args)...);
  };
}

struct(decorate) Foo{
  [[decorator(logger)]] static void fn(int){}
};

int main() {
  __reflect_pretty_print(reflexpr(Foo));
  Foo::fn(42);
}
