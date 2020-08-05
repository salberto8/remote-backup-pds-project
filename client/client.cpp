//
// Created by giacomo on 03/08/20.
//

#include <thread>
#include <nlohmann/json.hpp>

#include "client.h"
#include "backup.h"
#include "configuration.h"
#include "Session.h"


using json = nlohmann::json;

//std::mutex m;
//std::condition_variable cv;
http::status http_status;
std::string remote_digest;

void replaceSpaces(std::string &str) {

    // Getting the length of the string, counting the number of spaces
    int strLen = str.length();
    int i, count = 0;
    for (i = 0; i <= strLen; i++) {
        if(str[i]==' ')
            count++;
    }

    // Determining the new length needed to allocate for replacement characters '%20'
    int newLength = strLen + count * 2;

    str.resize(newLength);
    for (i = strLen - 1; i >= 0; i--) {
        if (str[i] == ' ') {
            str[newLength - 1] = '0';
            str[newLength - 2] = '2';
            str[newLength - 3] = '%';
            newLength = newLength - 3;
        }

        else {
            str[newLength - 1] = str[i];
            newLength = newLength -1;
        }
    }
}

void handle_response(http::response<http::string_body> *res) {
    //std::lock_guard<std::mutex> lg(m);
    remote_digest = res->body();
    http_status = res->result();
    //cv.notify_one();
}

bool probe_file(const std::string& original_path) {
    http_status = http::status::unknown;
    remote_digest = "";
    http::request<http::string_body> req;

    // make the relative path
    std::string relative_path = original_path.substr(configuration::backup_path.length());
    // substitute spaces with %20
    replaceSpaces(relative_path);

    // prepare the request message
    req.method(http::verb::get);
    req.target("/probefile" + relative_path);

    net::io_context ioc;
    // Launch the asynchronous operation
    std::make_shared<Session>(ioc)->run(&req);
    // Run the I/O service. The call will return when the get operation is complete.
    std::thread t_probe_file(
            [&ioc] {
                ioc.run();
            });

    // digest calculation while waiting for the http response
    std::string local_digest = calculate_digest(original_path);

    t_probe_file.join();

//    std::unique_lock<std::mutex> ul(m);
//    cv.wait(ul, [](){
//        return (http_status != http::status::unknown);
//    });
    if(http_status == http::status::ok && local_digest == remote_digest)
        return true;

    return false;
}

bool backup_file(const std::string& original_path) {
    http_status = http::status::unknown;
    http::request<http::string_body> req;

    // make the relative path
    std::string relative_path = original_path.substr(configuration::backup_path.length());
    // substitute spaces with %20
    replaceSpaces(relative_path);

    // create the encoded file
    auto encoded_file = encode(original_path);

    std::cout<< encoded_file.get()<<std::endl;
    // prepare the request message
    req.method(http::verb::post);
    req.target("/backup" + relative_path);
    req.set(http::field::content_type, "application/json");
    json j = {
            {"type", "file"},
            {"encodedfile", encoded_file.get()}
    };
    req.body() = j.dump();
    req.content_length(j.dump().length());

    net::io_context ioc;
    // Launch the asynchronous operation
    std::make_shared<Session>(ioc)->run(&req);
    // Run the I/O service. The call will return when the get operation is complete.
    ioc.run();

    if(http_status == http::status::ok)
        return true;

    return false;
}
