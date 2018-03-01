#include "test_Helpers.hpp"
#include <canbus/DriverFTDI.hpp>
#include <imu_advanced_navigation_anpp/Protocol.hpp>

using namespace std;
using namespace canbus;
using ::testing::ElementsAre;
using ::testing::ContainerEq;

struct DriverTest : ::testing::Test, iodrivers_base::Fixture<DriverFTDI>
{
    DriverTest()
    {
    }

    void open()
    {
        { IODRIVERS_BASE_MOCK();
            EXPECT_REPLY("Z1\n", "\n");
            EXPECT_REPLY("S2\n", "\n");
            EXPECT_REPLY("O\n", "\n");
            driver.open("test://:50k");
        }
    }


    void EXPECT_REPLY(const char* cmd, const char* reply)
    {
        uint8_t const* cmd_int8 = reinterpret_cast<uint8_t const*>(cmd);
        uint8_t const* reply_int8 = reinterpret_cast<uint8_t const*>(reply);
        iodrivers_base::Fixture<DriverFTDI>::EXPECT_REPLY(
            std::vector<uint8_t>(cmd_int8, cmd_int8 + strlen(cmd)),
            std::vector<uint8_t>(reply_int8, reply_int8 + strlen(reply))
        );
    }

    void pushDataToDriver(char const* msg)
    {
        uint8_t const* msg_int8 =
            reinterpret_cast<uint8_t const*>(msg);
        std::vector<uint8_t> packet(msg_int8, msg_int8 + strlen(msg));
        iodrivers_base::Fixture<DriverFTDI>::pushDataToDriver(packet);
    }
};

TEST_F(DriverTest, open_sets_a_CAN_rate_provided_in_the_URI)
{ IODRIVERS_BASE_MOCK();
    EXPECT_REPLY("Z1\n", "\n");
    EXPECT_REPLY("S2\n", "\n");
    EXPECT_REPLY("O\n", "\n");
    driver.open("test://:50k");
}

TEST_F(DriverTest, open_does_not_set_an_explicit_rate_if_it_is_not_provided_in_the_URI)
{ IODRIVERS_BASE_MOCK();
    EXPECT_REPLY("Z1\n", "\n");
    EXPECT_REPLY("O\n", "\n");
    driver.open("test://");
}

TEST_F(DriverTest, open_raises_if_one_of_the_messages_gets_a_0x7_reply)
{ IODRIVERS_BASE_MOCK();
    EXPECT_REPLY("Z1\n", "\x7");
    ASSERT_ANY_THROW( driver.open("test://"); );
}

canbus::Message writeTestMessage()
{
    Message msg;
    msg.can_id = 0x345;
    msg.size = 8;
    const uint8_t data[8] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
    };
    for (int i = 0; i < 8; ++i)
        msg.data[i] = data[i];
    return msg;
}

TEST_F(DriverTest, write_encodes_a_standard_frame_and_returns_when_it_is_acknowledged)
{
    open();

    IODRIVERS_BASE_MOCK();
    canbus::Message msg = writeTestMessage();
    EXPECT_REPLY("t34580123456789ABCDEF\n", "z\n");
    driver.write(msg);
}

TEST_F(DriverTest, write_rejects_the_wrong_ack)
{
    open();

    IODRIVERS_BASE_MOCK();
    canbus::Message msg = writeTestMessage();
    EXPECT_REPLY("t34580123456789ABCDEF\n", "\n");
    ASSERT_ANY_THROW(driver.write(msg));
}

TEST_F(DriverTest, write_throws_if_it_gets_an_error_reply)
{
    open();

    IODRIVERS_BASE_MOCK();
    canbus::Message msg = writeTestMessage();
    EXPECT_REPLY("t34580123456789ABCDEF\n", "\x7");
    ASSERT_ANY_THROW(driver.write(msg));
}

TEST_F(DriverTest, it_parses_a_standard_frame_with_timestamp)
{
    open();

    IODRIVERS_BASE_MOCK();
    pushDataToDriver("t34580123456789ABCDEF2345\n");
    Message msg = driver.read();

    ASSERT_EQ(msg.can_id, 0x345);
    ASSERT_EQ(msg.can_time, base::Time::fromMilliseconds(0x2345));
    ASSERT_EQ(msg.size, 8);
    ASSERT_EQ(msg.data[0], 0x01);
    ASSERT_EQ(msg.data[1], 0x23);
    ASSERT_EQ(msg.data[2], 0x45);
    ASSERT_EQ(msg.data[3], 0x67);
    ASSERT_EQ(msg.data[4], 0x89);
    ASSERT_EQ(msg.data[5], 0xAB);
    ASSERT_EQ(msg.data[6], 0xCD);
    ASSERT_EQ(msg.data[7], 0xEF);
}

TEST_F(DriverTest, it_rejects_a_frame_with_an_invalid_character_in_the_can_ID)
{
    open();

    IODRIVERS_BASE_MOCK();
    pushDataToDriver("t34@80123456789ABCDEF2345\n");
    ASSERT_ANY_THROW( driver.read() );
}

TEST_F(DriverTest, it_rejects_a_frame_with_an_invalid_character_in_the_can_length)
{
    open();

    IODRIVERS_BASE_MOCK();
    pushDataToDriver("t345901234567890ABCDEF2345\n");
    ASSERT_ANY_THROW( driver.read() );
}

TEST_F(DriverTest, it_rejects_a_frame_with_an_invalid_character_in_the_payload)
{
    open();

    IODRIVERS_BASE_MOCK();
    pushDataToDriver("t34580123456@89ABCDEF2345\n");
    ASSERT_ANY_THROW( driver.read() );
}

TEST_F(DriverTest, it_rejects_a_frame_with_an_invalid_character_in_the_timestamp)
{
    open();

    IODRIVERS_BASE_MOCK();
    pushDataToDriver("t34580123456789ABCDEF2@45\n");
    ASSERT_ANY_THROW( driver.read() );
}

TEST_F(DriverTest, it_parses_a_status_message)
{
    open();

    IODRIVERS_BASE_MOCK();
    EXPECT_REPLY("F\n", "C6\n");
    DriverFTDI::Status status = driver.getStatus();
    ASSERT_EQ(status.tx_state, DriverFTDI::WARNING);
    ASSERT_EQ(status.rx_state, DriverFTDI::WARNING);
    ASSERT_TRUE(status.rx_buffer0_overflow);
    ASSERT_TRUE(status.rx_buffer1_overflow);
}

TEST_F(DriverTest, it_determines_the_state_using_the_worst_case)
{
    open();

    IODRIVERS_BASE_MOCK();
    EXPECT_REPLY("F\n", "3E\n");
    DriverFTDI::Status status = driver.getStatus();
    ASSERT_EQ(status.tx_state, DriverFTDI::OFF);
    ASSERT_EQ(status.rx_state, DriverFTDI::PASSIVE);
}

TEST_F(DriverTest, it_handles_an_all_OK_message)
{
    open();

    IODRIVERS_BASE_MOCK();
    EXPECT_REPLY("F\n", "00\n");
    DriverFTDI::Status status = driver.getStatus();
    ASSERT_EQ(status.tx_state, DriverFTDI::OK);
    ASSERT_EQ(status.rx_state, DriverFTDI::OK);
    ASSERT_FALSE(status.rx_buffer0_overflow);
    ASSERT_FALSE(status.rx_buffer1_overflow);
}