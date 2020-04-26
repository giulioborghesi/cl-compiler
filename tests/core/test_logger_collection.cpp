#include <cool/core/logger.h>
#include <cool/core/logger_collection.h>

#include <gtest/gtest.h>

namespace cool {

TEST(LoggerCollection, BasicTest) {
  LoggerCollection loggers;
  auto logger = std::make_shared<Logger>(nullptr, LogMessageSeverity::DEBUG);

  /// Register logger
  {
    auto status = loggers.registerLogger("DebugLogger", logger);
    ASSERT_TRUE(status.isOk());
  }

  /// Attempt to register logger with already registered name
  {
    auto status = loggers.registerLogger("DebugLogger", logger);
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(status.getErrorMessage(), "Error: logger is already defined");
  }

  /// Attempt to remove a non-existing logger
  {
    auto status = loggers.removeLogger("NonExistentLogger");
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(status.getErrorMessage(), "Error: logger does not exist");
  }
}

} // namespace cool

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}