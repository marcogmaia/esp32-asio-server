#include <algorithm>
#include <array>
#include <string>

namespace mmrr::pass {

// template <int PasswordSize>
// class PasswordGeneric {
//  public:
//   PasswordGeneric() = default;

//   template <typename Container>
//   PasswordGeneric(const Container &container) {
//     std::copy_n(container.cbegin(), PasswordSize, pass_.begin());
//   }

//   constexpr int size() const { return size_; }

//   template <typename Container>
//   bool operator==(const Container &other) const {
//     return std::equal(pass_.cbegin(), pass_.cend(), other.cbegin());
//   }

//   const char *GetPassAsConstChar() const { return pass_.data(); }

//   const std::array<char, PasswordSize> &get_pass() const { return pass_; }

//  private:
//   std::array<char, PasswordSize> pass_;
//   int size_ = PasswordSize;
// };

// constexpr int kPasswordSize = 6;

// using Password = PasswordGeneric<kPasswordSize>;

}  // namespace mmrr::pass