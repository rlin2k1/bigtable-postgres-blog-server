/* blog.h
Header file for a Blog struct which stores the contents of a blog post.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 3rd, 2020
*/

#ifndef BLOG_STRUCT
#define BLOG_STRUCT

#include <string>

struct Blog {
    int postid;
    std::string title;
    std::string body;
};

#endif // BLOG_STRUCT
