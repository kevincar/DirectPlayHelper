cpplint --filter=-build/c++11,-legal/copyright $(find include -name "*.hpp") $(find src -name "*.cpp") $(find tests -name "*.*pp")
