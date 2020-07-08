#define G3_DYNAMIC_LOGGING

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include "../include/CustomSink.hpp"
#include "gtest/gtest.h"

int main(int argc, char* argv[])
{
	// Initialize the logger
	std::unique_ptr<g3::LogWorker> logWorker{g3::LogWorker::createLogWorker()};
	logWorker->addSink(std::make_unique<CustomSink>(), &CustomSink::ReceiveLogMessage);
	g3::initializeLogging(logWorker.get());
	g3::log_levels::setHighest(DEBUG);
	
	// Begin testing
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
