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

void replaceSpaces(std::string &str);
bool send_request(http::verb method, const char* target, const std::string &abs_path, const char* type);


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

bool send_request(http::verb method, const char* target, const std::string &abs_path, const char* type) {
    http::request<http::string_body> req;
    http::response<http::string_body> res;
    json j;

    // make the relative path
    std::string relative_path = abs_path.substr(configuration::backup_path.length());
    // substitute spaces with %20
    replaceSpaces(relative_path);

    // prepare the request message
    req.method(method);
    req.target(target + relative_path);

    if(std::strcmp(target, "/probefolder") == 0) {
        std::set<std::string> children_set = get_children(abs_path);
        j["children"] = children_set;
    }
    else if(std::strcmp(target, "/backup") == 0 && std::strcmp(type, "folder") == 0) {
        j["type"] = "folder";
    }
    else if(std::strcmp(target, "/backup") == 0 && std::strcmp(type, "file") == 0) {
        auto encoded_file = encode(abs_path);
        j["type"] = "file";
        j["encodedfile"] = encoded_file.get();
    }

    if(!j.empty()) {
        req.set(http::field::content_type, "application/json");
        req.body() = j.dump();
        req.content_length(j.dump().length());
    }

    net::io_context ioc;
    // Launch the asynchronous operation
    std::make_shared<Session>(ioc, req, res)->run();
    // Run the I/O service. The call will return when the get operation is complete.
    ioc.run();

    if(res.result() == http::status::ok)
        return true;

    return false;
}

bool probe_file(const std::string& abs_path) {
    http::request<http::string_body> req;
    http::response<http::string_body> res;

    // make the relative path
    std::string relative_path = abs_path.substr(configuration::backup_path.length());
    // substitute spaces with %20
    replaceSpaces(relative_path);

    // prepare the request message
    req.method(http::verb::get);
    req.target("/probefile" + relative_path);

    net::io_context ioc;
    // Launch the asynchronous operation
    std::make_shared<Session>(ioc, req, res)->run();
    // Run the I/O service. The call will return when the get operation is complete.
    std::thread t_probe_file(
            [&ioc] {
                ioc.run();
            });

    // digest calculation while waiting for the http response
    std::string local_digest = calculate_digest(abs_path);

    t_probe_file.join();

    if(res.result() == http::status::ok && res.body() == local_digest)
        return true;

    return false;
}

bool backup_file(const std::string& abs_path) {
    return send_request(http::verb::post, "/backup", abs_path, "file");
}

bool probe_folder(const std::string& abs_path) {
    return send_request(http::verb::post, "/probefolder", abs_path, "folder");
}

bool backup_folder(const std::string& abs_path) {
    return send_request(http::verb::post, "/backup", abs_path, "folder");
}

bool delete_path(const std::string& abs_path) {
    return send_request(http::verb::delete_, "/backup", abs_path, "");
}
