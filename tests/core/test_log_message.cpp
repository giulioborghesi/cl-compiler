#include <cool/core/log_message.h>

#include <gtest/gtest.h>

namespace cool {

TEST(LogMessage, BasicTest) {

  /// Create a debug message
  {
    auto logMessage = LogMessage::MakeDebugMessage("Plain message");
    ASSERT_EQ(logMessage.severity(), LogMessageSeverity::DEBUG);
    ASSERT_EQ(logMessage.message(), "Plain message");
  }

  /// Create a debug message with format
  {
    auto logMessage =
        LogMessage::MakeDebugMessage("Message with format: %d", 15);
    ASSERT_EQ(logMessage.severity(), LogMessageSeverity::DEBUG);
    ASSERT_EQ(logMessage.message(), "Message with format: 15");
  }

  /// Create an error message
  {
    auto logMessage = LogMessage::MakeErrorMessage("Error message");
    ASSERT_EQ(logMessage.severity(), LogMessageSeverity::ERROR);
    ASSERT_EQ(logMessage.message(), "Error message");
  }
}

} // namespace cool

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
