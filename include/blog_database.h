/* blog_database.h
Header file for all database related operations. All model related operations
are abstracted into this class.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 3rd, 2020
*/
#ifndef BLOG_DATABASE_HPP
#define BLOG_DATABASE_HPP

#include <string>
#include <vector>
#include <mutex>
#include <pqxx/pqxx>

#include "database.h"

class blog_database : public database{
 public:
    blog_database(std::string dbname, std::string user, std::string password, std::string hostaddr, std::string port);
    ~blog_database();

    virtual int insert_blog(std::string title, std::string body); // Returns postid of the blog post -> To be used in 302 redirect
    virtual Blog get_blog(int postid); // Returns a blog struct
    virtual std::vector<Blog> get_all_blogs(); // List of all blog posts

 private:
    pqxx::connection* conn_;
};

#endif  // BLOG_DATABASE_HPP
