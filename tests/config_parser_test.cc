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

TEST_F(NginxConfigParserTest, EchoAndStaticConfig) {
  bool success = parser.Parse("echo_static_config", &out_config);

  EXPECT_TRUE(success);

  std::string root_path = "/usr/src/projects";
  std::string server_static_path_1 = "/server_static_1";
  std::string client_static_path_1 = "/client_static_1";
  std::string server_static_path_2 = "/server_static_2";
  std::string client_static_path_2 = "/client_static_2";
  std::string echo_path = "/echo";

  EXPECT_EQ(out_config.static_locations_.size(), 2);
  EXPECT_EQ(out_config.root_path_, root_path);
  EXPECT_EQ(out_config.echo_location_, echo_path);
  EXPECT_EQ(out_config.static_locations_[client_static_path_1], server_static_path_1);
  EXPECT_EQ(out_config.static_locations_[client_static_path_2], server_static_path_2);
}
