/// @file iterator.hpp
#pragma once
#include <iterator>
#include <memory> // std::shared_ptr

#include <rich/fundamental.hpp>

namespace rich {
  template <class T>
  struct erased_output {
  private:
    using base_ptr_t = std::shared_ptr<void>;
    base_ptr_t out_;
    using handler_t = void(base_ptr_t&, T&);
    handler_t* handler_ = nullptr;

  public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    template <std::output_iterator<T> Out>
    constexpr explicit erased_output(Out o)
      : out_(std::make_shared<Out>(std::move(o))),
        handler_([](base_ptr_t& out, T& t) {
          *(*std::static_pointer_cast<Out>(out))++ = std::move(t);
        }) {}

    constexpr erased_output&
    operator=(const T& t) requires std::copy_constructible<T> {
      T tmp{t};
      handler_(out_, tmp);
      return *this;
    }

    constexpr erased_output& operator=(T&& t) {
      handler_(out_, t);
      return *this;
    }

    constexpr erased_output& operator*() { return *this; }
    constexpr erased_output& operator++() { return *this; }
    constexpr erased_output operator++(int) { return *this; }

    template <class Out, class U>
    requires std::output_iterator<Out, U>
    friend constexpr Out out(const erased_output<U>&);
  };

  template <class Out, class U>
  requires std::output_iterator<Out, U>
  constexpr Out out(const erased_output<U>& x) {
    return *std::static_pointer_cast<Out>(x.out_);
  }
} // namespace rich
