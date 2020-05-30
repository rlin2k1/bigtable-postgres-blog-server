/* blog_bigtable.h
Header file for all bigtable related operations. All model related operations
are abstracted into this class.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 3rd, 2020
*/
#ifndef BLOG_BIGTABLE_HPP
#define BLOG_BIGTABLE_HPP

#include "database.h"
#include "google/cloud/bigtable/table.h" // Google BigTable API

// Create a namespace alias to make the code easier to read.
namespace cbt = google::cloud::bigtable;

class blog_bigtable : public database{
 public:
    blog_bigtable(std::string gcloud_projectid, std::string bigtable_instanceid, std::string table);
    ~blog_bigtable();

    virtual int insert_blog(std::string title, std::string body); // Returns postid of the blog post -> To be used in 302 redirect
    virtual Blog get_blog(int postid); // Returns a blog struct
    virtual std::vector<Blog> get_all_blogs(); // List of all blog posts

 private:
    cbt::Table* table_;
    std::string column_family_;
};

#endif  // BLOG_BIGTABLE_HPP
