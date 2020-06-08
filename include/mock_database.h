/* mock_database.h
Header file for all mocked database related operations primarily relating to
unit testing. All model related operations are abstracted into this class.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 7th, 2020
*/
#ifndef MOCK_DATABASE_HPP
#define MOCK_DATABASE_HPP

#include <string>
#include <vector>

#include "database.h"

class mock_database : public database{
 public:
    mock_database();
    ~mock_database();

    virtual int insert_blog(std::string title, std::string body); // Returns postid of the blog post -> To be used in 302 redirect
    virtual Blog get_blog(int postid); // Returns a blog struct
    virtual std::vector<Blog> get_all_blogs(); // List of all blog posts

 private:
    std::vector<Blog> db_;
};

#endif  // MOCK_DATABASE_HPP
