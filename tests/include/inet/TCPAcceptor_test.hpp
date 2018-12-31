
#ifndef INET_TCP_ACCEPTOR_TEST
#define INET_TCP_ACCEPTOR_TEST

#include <thread>

std::unique_ptr<std::thread> startTestServer(std::string& serviceAddress, std::string& status, std::mutex& statusMutex, std::condition_variable& statusCV);
std::unique_ptr<std::thread> startTestClient(std::string& serviceAddress, std::string& status, std::mutex& statusMutex, std::condition_variable& statusCV);

#endif /* INET_TCP_ACCEPTOR_TEST */
