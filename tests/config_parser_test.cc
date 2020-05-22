#include "gtest/gtest.h"
#include "config_parser.h"
#include <algorithm>

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

TEST_F(NginxConfigParserTest, IPPortNumberConfig) {
    bool parsed_correctly = parser.Parse("ip_port_config", &out_config);
    bool success = parsed_correctly && out_config.port_number == CORRECT_PORT;
    EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, DNEFileConfig) {
  bool failure = parser.Parse("DNE_config", &out_config);

  EXPECT_FALSE(failure);
}

TEST_F(NginxConfigParserTest, QuotesConfig) {
  bool success = parser.Parse("quotes_config", &out_config);

  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, StatementsConfig) {
  bool success = parser.Parse("statements_config", &out_config);

  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, MatchedBracketsConfig) {
  bool success = parser.Parse("matched_brackets_config", &out_config);

  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, UnmatchedBracketsConfig) {
  bool failure = parser.Parse("unmatched_brackets_config", &out_config);

  EXPECT_FALSE(failure);
}

TEST_F(NginxConfigParserTest, EmptyConfig) {
  bool success = parser.Parse("empty_config", &out_config);

  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, CommentsConfig) {
  bool success = parser.Parse("comments_config", &out_config);

  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, EmptyBracesConfig) {
  bool success = parser.Parse("empty_braces_config", &out_config);

  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, BadEndConfig) {
  bool failure = parser.Parse("bad_end_config", &out_config);

  EXPECT_FALSE(failure);
}

TEST_F(NginxConfigParserTest, OpenBracesConfig) {
  bool failure = parser.Parse("open_braces_config", &out_config);

  EXPECT_FALSE(failure);
}

TEST_F(NginxConfigParserTest, QuoteEndConfig) {
  bool failure = parser.Parse("quote_end_config", &out_config);

  EXPECT_FALSE(failure);
}

TEST_F(NginxConfigParserTest, StatementEndConfig) {
  bool failure = parser.Parse("statement_end_config", &out_config);

  EXPECT_FALSE(failure);
}

TEST_F(NginxConfigParserTest, ForeignLanguageConfig) {
  bool success = parser.Parse("foreign_language_config", &out_config);

  EXPECT_TRUE(success);
}

TEST_F(NginxConfigParserTest, EchoStaticProxyConfig) {
  bool success = parser.Parse("echo_static_proxy_config", &out_config);

  EXPECT_TRUE(success);
  std::string echo_handler = "EchoHandler";
  std::string static_handler = "StaticHandler";
  std::string proxy_handler = "ProxyHandler";
  std::string server_static_path = "./files";
  std::string client_static_path_1 = "/static";
  std::string client_static_path_2 = "/static2";
  std::string first_echo_path = "/echo";
  std::string second_echo_path = "/echo2";
  std::string proxy_path_1 = "/proxy1";
  std::string proxy_server_1 = "/server1";
  int proxy_server_port_1 = 42069;
  std::string proxy_path_2 = "/proxy2";
  std::string proxy_server_2 = "/server2";
  int proxy_server_port_2 = 12345;

  bool found_first_echo = out_config.echo_locations_.find(first_echo_path) != out_config.echo_locations_.end();
  bool found_second_echo = out_config.echo_locations_.find(second_echo_path) != out_config.echo_locations_.end();
  bool found_echo_handler = std::find(out_config.handler_types_.begin(), out_config.handler_types_.end(), echo_handler) != out_config.handler_types_.end();
  bool found_static_handler = std::find(out_config.handler_types_.begin(), out_config.handler_types_.end(), static_handler) != out_config.handler_types_.end();
  bool found_proxy_handler = std::find(out_config.handler_types_.begin(), out_config.handler_types_.end(), proxy_handler) != out_config.handler_types_.end();

  EXPECT_EQ(out_config.echo_locations_.size(), 2);
  EXPECT_TRUE(found_first_echo);
  EXPECT_TRUE(found_second_echo);
  EXPECT_TRUE(found_echo_handler);
  EXPECT_TRUE(found_static_handler);
  EXPECT_TRUE(found_proxy_handler);

  EXPECT_EQ(out_config.static_locations_.size(), 2);
  EXPECT_EQ(out_config.static_locations_[client_static_path_1], server_static_path);
  EXPECT_EQ(out_config.static_locations_[client_static_path_2], server_static_path);
  std::pair<std::string, int> proxy_config_1 = {proxy_server_1, proxy_server_port_1};
  std::pair<std::string, int> proxy_config_2 = {proxy_server_2, proxy_server_port_2};
  EXPECT_EQ(out_config.proxy_locations_[proxy_path_1], proxy_config_1);
  EXPECT_EQ(out_config.proxy_locations_[proxy_path_2], proxy_config_2);
}

TEST_F(NginxConfigParserTest, SpacePathConfig) {
  bool success = parser.Parse("space_path_config", &out_config);

  EXPECT_TRUE(success);
  std::string echo_handler = "EchoHandler";
  std::string echo_path = "/e cho";
  bool found_echo = out_config.echo_locations_.find(echo_path) != out_config.echo_locations_.end();
  bool found_echo_handler = std::find(out_config.handler_types_.begin(), out_config.handler_types_.end(), echo_handler) != out_config.handler_types_.end();

  EXPECT_EQ(out_config.echo_locations_.size(), 1);
  EXPECT_TRUE(found_echo);
  EXPECT_TRUE(found_echo_handler);
}
