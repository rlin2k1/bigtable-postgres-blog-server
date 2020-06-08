/* mock_database.cc
Consists of mock database related operations primarily relating to unit
testing. All model related operations are abstracted into this class.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 7th, 2020
*/
#include <boost/log/trivial.hpp>

#include "mock_database.h"

mock_database::mock_database(){}

mock_database::~mock_database() {}

int mock_database::insert_blog(std::string title, std::string body){
    std::lock_guard<std::mutex> guard(this->mtx_);

    int postid = -1;

    postid = this->db_.size() + 1;
    Blog blog = {
        postid,
        title,
        body
    };
    this->db_.push_back(blog);

    BOOST_LOG_TRIVIAL(info) \
    << "Finished attempt to insert blog post with postid: " \
    << std::to_string(postid);

    return postid;
} // auto unlock (lock_guard, RAII)

Blog mock_database::get_blog(int postid) {
    Blog blog = { // Sentinel, check for -1 as the postid
        -1,
        "",
        ""
    };

    std::lock_guard<std::mutex> guard(this->mtx_);

    try {
        blog = this->db_.at(postid - 1);
        BOOST_LOG_TRIVIAL(info) \
        << "Finished attempt to obtain blog post with postid: " \
        << std::to_string(postid);
    } catch (std::out_of_range& err) {
        BOOST_LOG_TRIVIAL(error) \
        << "Invalid postid: " \
        << std::to_string(postid);
    }

    return blog;
} // auto unlock (lock_guard, RAII)

std::vector<Blog> mock_database::get_all_blogs() {
    std::lock_guard<std::mutex> guard(this->mtx_);

    return this->db_;
} // auto unlock (lock_guard, RAII)
