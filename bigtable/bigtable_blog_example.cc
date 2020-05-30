/* bigtable_blog_example.cc
Example code of using Google-Cloud-Bigtable API to insert and retrieve
blog posts from Google Bigtable.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 5th, 2020
*/

#include "blog_bigtable.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // -------------------------------------------------------------------------- //
    // Mock Bigtable API Example
    // -------------------------------------------------------------------------- //
    blog_bigtable bb("mrjk-273504", "mrjk-database", "posts");
    std::cout << "INSERTING BLOG POST with Title: TitlePost and Body: BodyPost and Generated PostID:" << std::endl;
    std::cout << bb.insert_blog("TitlePost", "BodyPost") << std::endl;
    std::cout << "GETTING ALL BLOG POSTS IN BIGTABLE------------------------------------" << std::endl;
    std::vector<Blog> blogs = bb.get_all_blogs();
    for (auto vectorit = blogs.begin(); vectorit != blogs.end(); ++vectorit) {
        std::cout << "PostID = " << std::to_string((*vectorit).postid) << std::endl;
        std::cout << "Title = " << (*vectorit).title << std::endl;
        std::cout << "Body = " << (*vectorit).body << std::endl;
    }

    std::cout << "GETTING BLOG POST WITH POSTID 3------------------------------------" << std::endl;
    Blog blog = bb.get_blog(3);
    std::cout << "PostID = " << std::to_string(blog.postid) << std::endl;
    std::cout << "Title = " << blog.title << std::endl;
    std::cout << "Body = " << blog.body << std::endl;

    // End of Mock Bigtable API Example
    //----------------------------------------------------------------------------------------------
}
