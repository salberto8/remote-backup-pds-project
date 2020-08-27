//
// Created by giacomo on 03/08/20.
//

#include <thread>
#include <nlohmann/json.hpp>
#include <boost/asio/signal_set.hpp>

#include "client.h"
#include "backup.h"
#include "configuration.h"
#include "Session.h"

// define the target for using the server API
#define api_probefile "/probefile/"
#define api_probefolder "/probefolder/"
#define api_backup "/backup/"

// define folder and file standards
enum TargetType { probefolder, probefile, backupfile, backupfolder, delete_ };



using json = nlohmann::json;

void replaceSpaces(std::string &str);
bool send_request(http::verb method, const std::string &abs_path, TargetType type);


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

bool send_request(http::verb method, const std::string &abs_path, TargetType type) {
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

    std::string target;
    switch (type){
        case probefolder : target = api_probefolder; break;
        case probefile : target = api_probefile; break;
        case backupfile :  case backupfolder : case delete_ : target = api_backup; break;
    }
    req.target(target + relative_path);

    if(type == probefolder) {
        std::set<std::string> children_set = get_children(abs_path);
        j["children"] = children_set;
    }
    else if(type == backupfolder) {
        j["type"] = "folder";
    }
    else if(type == backupfile) {
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

    if(res.result() == http::status::ok) {
        return true;
    }
    else if(res.result() == http::status::not_found) {
        return false;
    }
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
    req.target(api_probefile + relative_path);

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

    if(res.result() == http::status::ok){
        if ( res.body() == local_digest) {
            // the file is the same on the server
            return true;
        }
        else {
            // digests are different -> delete file from server and re-send it. Then check with probe file
            delete_path(abs_path);
            backup_file(abs_path);
            return probe_file(abs_path);
        }
    }
    else if(res.result() == http::status::not_found) {
        return false;
    }
    else {
        throw (ExceptionBackup(res.body(), static_cast<int>(res.result())));
    }
}

void backup_file(const std::string& abs_path) {
    send_request(http::verb::post, abs_path, backupfile);
}

bool probe_folder(const std::string& abs_path) {
    return send_request(http::verb::post, abs_path, probefolder);
}

void backup_folder(const std::string& abs_path) {
    send_request(http::verb::post, abs_path, backupfolder);
}

void delete_path(const std::string& abs_path) {
    // the delete method is valid for both folders and files
    send_request(http::verb::delete_, abs_path, delete_);
}

void authenticateToServer(){
    http::request<http::string_body> req;
    http::response<http::string_body> res;
    res.result(http::status::unknown);
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

        ioc.stop();
        return;
    }
    else {
        throw (ExceptionBackup(res.body(), static_cast<int>(res.result())));
    }
}

void logout(){
    http::request<http::string_body> req;
    http::response<http::string_body> res;
    res.result(http::status::unknown);

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

        return ;
    }
    else
        throw (ExceptionBackup(res.body(), static_cast<int>(res.result())));
}