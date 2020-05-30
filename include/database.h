/* database.h
Header file for the database (abstract base class) regarding insertion and
retrieval of blog posts.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 5th, 2020
*/

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <vector>
#include <mutex>

#include "blog.h"

// The common database for blog post insertion and
class database {
 public:
    // Pure virtual functions. We need to derive from and then implement this method
    virtual int insert_blog(std::string title, std::string body) = 0; // Returns postid of the blog post -> To be used in 302 redirect
    virtual Blog get_blog(int postid) = 0; // Returns a blog struct
    virtual std::vector<Blog> get_all_blogs() = 0; // List of all blog posts
 protected: // Now, only derived classes can access the mutex as a private variable
    mutable std::mutex mtx_;
};

#endif  // DATABASE_HPP
