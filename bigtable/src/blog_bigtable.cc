/* blog_bigtable.cc
Consists of bigtable related operations. All model related operations
are abstracted into this class.

Tutorials Used:
https://www.youtube.com/watch?v=KaRbKdMInuc
https://cloud.google.com/bigtable/docs/quickstart-cbt
https://googleapis.github.io/google-cloud-cpp/
https://cloud.google.com/bigtable/docs/samples-cpp-hello
https://github.com/googleapis/google-cloud-cpp/blob/master/doc/packaging.md
https://github.com/googleapis/google-cloud-cpp
https://github.com/googleapis/google-cloud-cpp/tree/master/google/cloud/bigtable/quickstart
https://googleapis.dev/cpp/google-cloud-bigtable/latest/

Notes:
Playing around with Google Bigtable, I felt very limited with a NoSQL database.
First was speed, since BigTable was designed for Pedabytes of steaming data, the
structured use of it as well as HDD instead of SSD and 1 node instead of 30 nodes
really limited my computation power. Also, COST. The minimum Bigtable cluster
with 1 Node and 1GB of HDD is $500/month from ~$1 a minute to run the cluster.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 3rd, 2020
*/
#include <boost/log/trivial.hpp>

#include "blog_bigtable.h"

// Create a namespace alias to make the code easier to read.
namespace cbt = google::cloud::bigtable;

blog_bigtable::blog_bigtable(std::string gcloud_projectid, std::string bigtable_instanceid, std::string table){
    table_ = new cbt::Table(cbt::CreateDefaultDataClient(gcloud_projectid, bigtable_instanceid, cbt::ClientOptions()), table);
    BOOST_LOG_TRIVIAL(info) << "Opened database connection successfully to: " << bigtable_instanceid << "| Table: " << table;
    column_family_ = "postcontent";
}

blog_bigtable::~blog_bigtable() {
    delete this->table_;
}

int blog_bigtable::insert_blog(std::string title, std::string body){
    std::lock_guard<std::mutex> guard(this->mtx_);

    int postid = -1;

    try {
        // Finds latest row key
        cbt::v1::RowReader rd = this->table_->ReadRows(cbt::RowRange::InfiniteRange(), cbt::Filter::PassAllFilter());

        for (cbt::v1::RowReader::iterator it = rd.begin(); it != rd.end(); it++) {
            // RowReader iterators do not have addition
            postid = std::stoi((*it)->row_key()) + 1;
        }

        std::string row_key = std::to_string(postid);
        google::cloud::Status status = this->table_->Apply(cbt::SingleRowMutation(
            std::move(row_key), cbt::SetCell(this->column_family_, "title", title), cbt::SetCell(this->column_family_, "body", body)));
        if (!status.ok()) throw std::runtime_error(status.message());
        BOOST_LOG_TRIVIAL(info) << "Finished attempt to insert blog post with postid: " << std::to_string(postid);
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
    }

    return postid;
} // auto unlock (lock_guard, RAII)

Blog blog_bigtable::get_blog(int postid) {
    std::lock_guard<std::mutex> guard(this->mtx_);

    Blog blog = { // Sentinel, check for -1 as the postid
        -1,
        "",
        ""
    };
    try{
        std::string row_key = std::to_string(postid);

        google::cloud::StatusOr<std::pair<bool, cbt::Row>> result =
            this->table_->ReadRow(row_key, cbt::Filter::FamilyRegex(this->column_family_));
        if (!result) throw std::runtime_error(result.status().message());
        if (!result->first) {
            BOOST_LOG_TRIVIAL(error) << "Cannot find row " << row_key << " in the table";
        }

        std::string title = "";
        std::string body = "";

        for (cbt::Cell const& c : result->second.cells()) {
            if (c.column_qualifier() == "title") {
                title = c.value();
            } else if (c.column_qualifier() == "body") {
                body = c.value();
            }
        }

        blog = {
            postid,
            title,
            body
        };
        BOOST_LOG_TRIVIAL(info) << "Finished attempt to obtain blog post with postid: " << std::to_string(postid);
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
    }

    return blog;
} // auto unlock (lock_guard, RAII)

std::vector<Blog> blog_bigtable::get_all_blogs() {
    std::lock_guard<std::mutex> guard(this->mtx_);

    std::vector<Blog> res;

    try {
        for (google::cloud::StatusOr<cbt::Row> const& row : this->table_->ReadRows(cbt::RowRange::InfiniteRange(), cbt::Filter::PassAllFilter())) {
            if (!row) throw std::runtime_error(row.status().message());

            int postid = std::stoi(row->row_key());
            std::string title = "";
            std::string body = "";

            for (cbt::Cell const& c : row->cells()) {
                if (c.column_qualifier() == "title") {
                    title = c.value();
                } else if (c.column_qualifier() == "body") {
                    body = c.value();
                }
            }
            Blog blog = {
                        postid,
                        title,
                        body
                    };
            res.push_back(blog);
        }
        BOOST_LOG_TRIVIAL(info) << "Finish attempt to obtain all blog posts";
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
    }

    return res;
} // auto unlock (lock_guard, RAII)
