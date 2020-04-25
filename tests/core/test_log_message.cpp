#include <cool/core/log_message.h>

#include <gtest/gtest.h>

namespace cool {

TEST(LogMessage, BasicTest) {

  /// Create a debug message
  {
    auto message = LogMessage::MakeDebugMessage("Plain message");
    ASSERT_EQ(message.severity(), LogMessageSeverity::DEBUG);
    ASSERT_EQ(message.logMessage(), "Plain message");
  }

  /// Create a debug message with format
  {
    auto message = LogMessage::MakeDebugMessage("Message with format: %d", 15);
    ASSERT_EQ(message.severity(), LogMessageSeverity::DEBUG);
    ASSERT_EQ(message.logMessage(), "Message with format: 15");
  }

  /// Create an error message
  {
    auto message = LogMessage::MakeErrorMessage("Error message");
    ASSERT_EQ(message.severity(), LogMessageSeverity::ERROR);
    ASSERT_EQ(message.logMessage(), "Error message");
  }
}

} // namespace cool

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
