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
#include <iostream> //TODO (ROY): Remove after Logging

#include "blog_database.h"

blog_database::blog_database(std::string dbname, std::string user, std::string password, std::string hostaddr, std::string port){
    std::stringstream ss;
    ss << "dbname = " << dbname << " user = " << user << " password = " << password << " hostaddr = " << hostaddr << " port = " << port;
    // TODO(ROY): Possibly put credentials in a permissions locked file
    // assign to std::string
    std::string credentials = ss.str();

    try {
        conn_ = new pqxx::connection(credentials);
        if (conn_->is_open()) {
            std::cout << "Opened database successfully: " << conn_->dbname() << std::endl; //TODO (ROY): LOGGING
        } else {
            std::cout << "Can't open database" << std::endl; //TODO (ROY): LOGGING
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

blog_database::~blog_database() {
    this->conn_->disconnect();
    delete this->conn_;
}

int blog_database::add_blog(std::string title, std::string body){
  try {
      int postid = -1;

      // Create SQL Statement
      std::string sql = "INSERT INTO posts (postid, title, body) "  \
        "VALUES (DEFAULT, '" + title + "', '" + body + "') RETURNING postid;"; // TODO (ROY): Prepared Statements to Prevent SQL Injection

      std::lock_guard<std::mutex> guard(this->mtx_);

      // Create a transactional object
      pqxx::work W(*(this->conn_));

      // Execute SQL Query
      pqxx::result R( W.exec( sql ));

      // Get results
      for (pqxx::result::const_iterator c = R.begin(); c != R.end(); ++c) { // TODO (ROY): Get rid of For Logic
        postid = c[0].as<int>();
      }

      W.commit();
      std::cout << "Records created successfully" << std::endl; //TODO (ROY): LOGGING
      return postid;
   } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
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
        std::string sql;

        // Create SQL Statement
        sql = "SELECT * from posts WHERE postid=" + std::to_string(postid); // TODO (ROY): Prepared Statements to Prevent SQL Injection

        std::lock_guard<std::mutex> guard(this->mtx_);

        // Create a non-transactional object
        pqxx::nontransaction N(*(this->conn_));

        // Execute SQL Query
        pqxx::result R( N.exec( sql ));

        // Get results
        for (pqxx::result::const_iterator c = R.begin(); c != R.end(); ++c) { // TODO (ROY): Get rid of For Logic
            blog = {
                c[0].as<int>(), // postid
                c[1].as<std::string>(), // title
                c[2].as<std::string>() //body
            };
        }
        std::cout << "Operation done successfully" << std::endl; //TODO (ROY): LOGGING
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return blog;
} // auto unlock (lock_guard, RAII)

std::vector<Blog> blog_database::get_all_blogs() {
    std::vector<Blog> res;

    try {
        // Create SQL Statement
        std::string sql = "SELECT * from posts"; // TODO (ROY): Prepared Statements to Prevent SQL Injection

        std::lock_guard<std::mutex> guard(this->mtx_);

        // Create a non-transactional object
        pqxx::nontransaction N(*(this->conn_));

        // Execute SQL Query
        pqxx::result R( N.exec( sql ));

        // Get results
        for (pqxx::result::const_iterator c = R.begin(); c != R.end(); ++c) {
          Blog blog = {
              c[0].as<int>(), // postid
              c[1].as<std::string>(), // title
              c[2].as<std::string>() //body
          };
          res.push_back(blog);
        }
        std::cout << "Operation done successfully" << std::endl; //TODO (ROY): LOGGING
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return res;
} // auto unlock (lock_guard, RAII)
