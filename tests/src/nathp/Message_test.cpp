
#include <vector>

#include <experimental/net>
#include "gtest/gtest.h"

TEST(Messages, addressTest) {
  std::experimental::net::ip::udp::endpoint ep(
      std::experimental::net::ip::udp::v4(),
      255
      );

  std::cout << ep << "\n"
    << "Size: " << ep.size() << "\n"
    << "Capacity: " << ep.capacity() << std::endl;

  int const size = ep.size();
  std::vector<char> data;
  data.resize(size);
  data.assign(reinterpret_cast<char*>(ep.data()), reinterpret_cast<char*>(ep.data())+size);
  for(auto i : data) {
    std::cout << std::hex << static_cast<unsigned short>(i) << std::endl;
  }
}
