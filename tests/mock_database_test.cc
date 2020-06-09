#include "gtest/gtest.h"
#include "mock_database.h"

class MockDatabaseTest : public ::testing::Test {
 protected:
    // Database to be mocked
    mock_database md;
};

TEST_F(MockDatabaseTest, BlogNone) {
    std::vector<Blog> blogs = md.get_all_blogs();
    EXPECT_TRUE(blogs.empty());
}

TEST_F(MockDatabaseTest, BlogDNE) {
    Blog expected_blog = Blog{-1, "", ""};
    Blog actual_blog = md.get_blog(4);
    EXPECT_EQ(actual_blog, expected_blog);
}

TEST_F(MockDatabaseTest, BlogInsertGet) {
    int expected_postid = 1;
    int actual_postid = md.insert_blog("Title: Hydroflask", "Body: Starbucks");
    EXPECT_EQ(actual_postid, expected_postid);

    expected_postid = 2;
    actual_postid = md.insert_blog("Title: Flowers", "Body: Balloons");
    EXPECT_EQ(actual_postid, expected_postid);

    expected_postid = 3;
    actual_postid = md.insert_blog("Title: Leasing", "Body: Clothes");
    EXPECT_EQ(actual_postid, expected_postid);

    expected_postid = 4;
    actual_postid = md.insert_blog("Title: Western", "Body: Express");
    EXPECT_EQ(actual_postid, expected_postid);

    std::vector<Blog> expected_blogs {
        Blog{1, "Title: Hydroflask", "Body: Starbucks"},
        Blog{2, "Title: Flowers", "Body: Balloons"},
        Blog{3, "Title: Leasing", "Body: Clothes"},
        Blog{4, "Title: Western", "Body: Express"}
    };

    std::vector<Blog> actual_blogs = md.get_all_blogs();
    EXPECT_EQ(expected_blogs, actual_blogs);

    Blog expected_blog = Blog{1, "Title: Hydroflask", "Body: Starbucks"};
    Blog actual_blog = md.get_blog(1);
    EXPECT_EQ(actual_blog, expected_blog);

    expected_blog = Blog{2, "Title: Flowers", "Body: Balloons"};
    actual_blog = md.get_blog(2);
    EXPECT_EQ(actual_blog, expected_blog);

    expected_blog = Blog{3, "Title: Leasing", "Body: Clothes"};
    actual_blog = md.get_blog(3);
    EXPECT_EQ(actual_blog, expected_blog);

    expected_blog = Blog{4, "Title: Western", "Body: Express"};
    actual_blog = md.get_blog(4);
    EXPECT_EQ(actual_blog, expected_blog);
}
