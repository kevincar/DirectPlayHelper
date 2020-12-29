
#ifndef TESTS_INCLUDE_INET_TCPACCEPTOR_TEST_HPP_
#define TESTS_INCLUDE_INET_TCPACCEPTOR_TEST_HPP_

#include <memory>
#include <mutex>
#include <string>
#include <thread>

std::unique_ptr<std::thread> startTestServer(
    std::string const& serviceAddress, std::string const& status,
    std::mutex const& statusMutex, std::condition_variable const& statusCV);
std::unique_ptr<std::thread> startTestClient(
    std::string const& serviceAddress, std::string const& status,
    std::mutex const& statusMutex, std::condition_variable const& statusCV);

#endif  // TESTS_INCLUDE_INET_TCPACCEPTOR_TEST_HPP_
