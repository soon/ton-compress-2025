#include <iostream>

struct S
{
  bool advance_ext(unsigned bits, unsigned refs)
  {
    unsigned x;
    std::cin >> x;
    return (bits + refs) > x;
  }

  bool advance_ext(unsigned bits_refs)
  {
    return advance_ext(bits_refs >> 16, bits_refs & 0xffff);
  }
};

int main(int argc, char const *argv[])
{
  unsigned x;
  std::cin >> x;
  std::cout << S().advance_ext(x);
  return 0;
}
