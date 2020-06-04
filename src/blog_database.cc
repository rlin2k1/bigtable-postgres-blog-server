/* blog_database.cc
Consists of database related operations. All model related operations
are abstracted into this class.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 3rd, 2020
*/
#include <sstream>
#include <boost/log/trivial.hpp>

#include "blog_database.h"

blog_database::blog_database(std::string dbname, std::string user, std::string password, std::string hostaddr, std::string port){
    std::stringstream ss;
    ss << "dbname = " << dbname << " user = " << user \
    << " password = " << password << " hostaddr = " << hostaddr \
    << " port = " << port;
    // assign to std::string
    std::string credentials = ss.str();

    try {
        conn_ = new pqxx::connection(credentials);
        if (conn_->is_open()) {
            BOOST_LOG_TRIVIAL(info) \
            << "Opened database connection successfully: " << conn_->dbname();
        } else {
            BOOST_LOG_TRIVIAL(error) << "Can't connect to database";
        }

        // -------------------------------------------------------------------------- //
        // Prepared statements - initialized once
        // -------------------------------------------------------------------------- //
        conn_->prepare(
        "insert_blog",
        "INSERT INTO posts (postid, title, body) VALUES (DEFAULT, $1, $2) RETURNING postid;");
        conn_->prepare(
        "get_blog",
        "SELECT * from posts WHERE postid = $1");
        conn_->prepare(
        "get_all_blogs",
        "SELECT * from posts");
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(info) << e.what();
    }
}

blog_database::~blog_database() {
    this->conn_->disconnect();
    delete this->conn_;
}

int blog_database::insert_blog(std::string title, std::string body){
  try {
      int postid = -1;

      std::lock_guard<std::mutex> guard(this->mtx_);

      // Create a transactional object
      pqxx::work W(*(this->conn_));

      // Execute SQL Query
      pqxx::result R = W.prepared("insert_blog")(title)(body).exec();

      // Get result
      R.at(0).at(0).to(postid);

      W.commit();
      BOOST_LOG_TRIVIAL(info) \
      << "Finished attempt to insert blog post with postid: " \
      << std::to_string(postid);
      return postid;
   } catch (const std::exception &e) {
      BOOST_LOG_TRIVIAL(info) << e.what();
      return -1;
   }
} // auto unlock (lock_guard, RAII)

Blog blog_database::get_blog(int postid) {
    Blog blog = { // Sentinel, check for -1 as the postid
        -1,
        "",
        ""
    };

    try {
        std::lock_guard<std::mutex> guard(this->mtx_);

        // Create a non-transactional object
        pqxx::nontransaction N(*(this->conn_));

        // Execute SQL Query
        pqxx::result R = N.prepared("get_blog")(std::to_string(postid)).exec();

        // Get results
        blog = {
            R.at(0).at(0).as<int>(), // postid
            R.at(0).at(1).as<std::string>(), // title
            R.at(0).at(2).as<std::string>() //body
        };

        BOOST_LOG_TRIVIAL(info) \
        << "Finished attempt to obtain blog post with postid: " \
        << std::to_string(postid);
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(info) << e.what();
    }
    return blog;
} // auto unlock (lock_guard, RAII)

std::vector<Blog> blog_database::get_all_blogs() {
    std::vector<Blog> res;

    try {
        std::lock_guard<std::mutex> guard(this->mtx_);

        // Create a non-transactional object
        pqxx::nontransaction N(*(this->conn_));

        // Execute SQL Query
        pqxx::result R = N.prepared("get_all_blogs").exec();

        // Get results
        for (pqxx::result::const_iterator c = R.begin(); c != R.end(); ++c) {
          Blog blog = {
              c[0].as<int>(), // postid
              c[1].as<std::string>(), // title
              c[2].as<std::string>() //body
          };
          res.push_back(blog);
        }
        BOOST_LOG_TRIVIAL(info) << "Finish attempt to obtain all blog posts";
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(info) << e.what();
    }

    return res;
} // auto unlock (lock_guard, RAII)
