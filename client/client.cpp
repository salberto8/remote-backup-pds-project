//
// Created by giacomo on 03/08/20.
//

#include <thread>
#include <nlohmann/json.hpp>

#include "client.h"
#include "backup.h"
#include "configuration.h"
#include "Session.h"

// define the target for using the server API
#define probefile "/probefile/"
#define probefolder "/probefolder/"
#define backup "/backup/"

// define folder and file standards
#define folder 1
#define file 2


using json = nlohmann::json;

void replaceSpaces(std::string &str);
bool send_request(http::verb method, const char* target, const std::string &abs_path, int type);


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

bool send_request(http::verb method, const char* target, const std::string &abs_path, int type) {
    http::request<http::string_body> req;
    http::response<http::string_body> res;
    res.result(http::status::unknown);
    json j;

    // make the relative path
    std::string relative_path = abs_path.substr(configuration::backup_path.length());
    // substitute spaces with %20
    replaceSpaces(relative_path);

    // prepare the request message
    req.method(method);
    req.target(target + relative_path);

    if(std::strcmp(target, probefolder) == 0) {
        std::set<std::string> children_set = get_children(abs_path);
        j["children"] = children_set;
    }
    else if(std::strcmp(target, backup) == 0 && type == folder) {
        j["type"] = "folder";
    }
    else if(std::strcmp(target, backup) == 0 && type == file) {
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
    else if(res.result() == http::status::not_found)
        return false;
    else
        throw (ExceptionBackup(res.body(), static_cast<int>(res.result())));
}

bool probe_file(const std::string& abs_path) {
    http::request<http::string_body> req;
    http::response<http::string_body> res;
    res.result(http::status::unknown);

    // make the relative path
    std::string relative_path = abs_path.substr(configuration::backup_path.length());
    // substitute spaces with %20
    replaceSpaces(relative_path);

    // prepare the request message
    req.method(http::verb::get);
    req.target(probefile + relative_path);

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
    else if(res.result() == http::status::not_found)
        return false;
    else
        throw (ExceptionBackup(res.body(), static_cast<int>(res.result())));
}

void backup_file(const std::string& abs_path) {
    send_request(http::verb::post, backup, abs_path, file);
}

bool probe_folder(const std::string& abs_path) {
    return send_request(http::verb::post, probefolder, abs_path, folder);
}

void backup_folder(const std::string& abs_path) {
    send_request(http::verb::post, backup, abs_path, folder);
}

void delete_path(const std::string& abs_path) {
    // the delete method is valid for both folders and files
    send_request(http::verb::delete_, backup, abs_path, 0);
}

bool authenticateToServer(){
    http::request<http::string_body> req;
    http::response<http::string_body> res;
    std::string password ;

    std::cout << "Hello " + configuration::username
              << "\nIn order to authenticate to server, type your password: " ;

    std::cin >> password;

    // prepare the request message
    req.method(http::verb::post);
    req.target("/login");
    req.set(http::field::content_type, "application/json");
    // the body contains a json with username and password
    json j = {
            {"username", configuration::username},
            {"password", password}
    };
    req.body() = j.dump();
    req.content_length(j.dump().length());

    net::io_context ioc;
    // Launch the asynchronous operation
    std::make_shared<Session>(ioc, req, res)->run();
    // Run the I/O service. The call will return when the get operation is complete.
    ioc.run();

    if(res.result() == http::status::ok) {
        std::string token = res.body();

        // save the token got from the server in the configuration
        configuration::token = std::move(token);

        return true;
    }

    return false;
}

bool logout(){
    http::request<http::string_body> req;
    http::response<http::string_body> res;

    // prepare the request message
    req.method(http::verb::post);
    req.target("/logout");

    net::io_context ioc;
    // Launch the asynchronous operation
    std::make_shared<Session>(ioc, req, res)->run();
    // Run the I/O service. The call will return when the get operation is complete.
    ioc.run();

    if(res.result() == http::status::ok) {

        std::string token{""};
        // delete the token from the configuration because invalid
        configuration::token = std::move(token);

        return true;
    }

    return false;
}