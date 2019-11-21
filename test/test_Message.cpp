#include <gtest/gtest.h>
#include <canbus/Message.hpp>

using namespace std;
using namespace canbus;

struct MessageTest : public ::testing::Test {
};

TEST_F(MessageTest, it_produces_a_zeroed_message)
{
    auto msg = Message::Zeroed();
    ASSERT_TRUE(msg.time.isNull());
    ASSERT_TRUE(msg.can_time.isNull());
    ASSERT_EQ(0, msg.can_id);
    ASSERT_EQ(0, msg.size);
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(0, msg.data[i]);
    }
}
