#include "gtest/gtest.h"
#include "config_parser.h"

#define CORRECT_PORT 8080
#define INCORRECT_PORT -1

class NginxConfigParserTest : public ::testing::Test {
    protected:
        NginxConfigParser parser;
        NginxConfig out_config;
};

// Tests to make sure we are retrieving the right port number
TEST_F(NginxConfigParserTest, SimpleConfig) {
    bool parsed_correctly = parser.Parse("example_config", &out_config);
    bool success = parsed_correctly && out_config.port_number == CORRECT_PORT;
    EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, MissingPortNumberConfig) {
    bool parsed_correctly = parser.Parse("missing_port_number_config", &out_config);
    bool got_default_bad_port_number = parsed_correctly && out_config.port_number == INCORRECT_PORT;
    EXPECT_TRUE(got_default_bad_port_number);
}

TEST_F(NginxConfigParserTest, MissingListenConfig) {
    bool parsed_correctly = parser.Parse("missing_listen_config", &out_config);
    bool got_default_bad_port_number = parsed_correctly && out_config.port_number == INCORRECT_PORT;
    EXPECT_TRUE(got_default_bad_port_number);
}

TEST_F(NginxConfigParserTest, StringPortNumberConfig) {
    bool parsed_correctly = parser.Parse("listen_string_config", &out_config);
    bool got_default_bad_port_number = parsed_correctly && out_config.port_number == INCORRECT_PORT;
    EXPECT_TRUE(got_default_bad_port_number);
}

TEST_F(NginxConfigParserTest, TooBigPortNumberConfig) {
    bool parsed_correctly = parser.Parse("too_big_port_number_config", &out_config);
    bool got_default_bad_port_number = parsed_correctly && out_config.port_number == INCORRECT_PORT;
    EXPECT_TRUE(got_default_bad_port_number);
}

TEST_F(NginxConfigParserTest, NegativePortNumberConfig) {
    bool parsed_correctly = parser.Parse("negative_port_number_config", &out_config);
    bool got_default_bad_port_number = parsed_correctly && out_config.port_number == INCORRECT_PORT;
    EXPECT_TRUE(got_default_bad_port_number);
}