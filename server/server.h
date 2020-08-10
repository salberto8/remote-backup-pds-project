//
// Created by stealbi on 14/07/20.
//

#ifndef SERVER_PROGETTO_SERVER_H
#define SERVER_PROGETTO_SERVER_H

#include <boost/beast/core/detail/base64.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <nlohmann/json.hpp>


#include "backup.h"
#include "authorization.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using json = nlohmann::json;
namespace base64 = boost::beast::detail::base64;

// Report a failure
void fail(beast::error_code ec, const std::string  &what);


// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<
        class Body, class Allocator,
        class Send>
void handle_request(
        http::request<Body, http::basic_fields<Allocator>>&& req,
        Send&& send)
{


    // Returns a bad request response
    auto const bad_request =
            [&req](beast::string_view why)
            {
                http::response<http::string_body> res{http::status::bad_request, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/plain");
                res.keep_alive(req.keep_alive());
                res.body() = std::string(why);
                res.prepare_payload();
                return res;
            };

    // Returns a not found response
    auto const not_found =
            [&req]()
            {
                http::response<http::empty_body> res{http::status::not_found, req.version()};
                //res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                //res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                //res.body() = "The resource '" + std::string(target) + "' was not found.";
                //res.prepare_payload();
                return res;
            };

    // Returns a server error response
    auto const server_error =
            [&req](beast::string_view what)
            {
                http::response<http::string_body> res{http::status::internal_server_error, req.version()};
                //res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/plain");
                res.keep_alive(req.keep_alive());
                res.body() = "An error occurred: '" + std::string(what) + "'";
                res.prepare_payload();
                return res;
            };


    auto const okay_response =
            [&req]()
            {
                http::response<http::empty_body> res{http::status::ok, req.version()};
                //res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                //res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                //res.body() = "The resource '" + std::string(target) + "' was not found.";
                //res.prepare_payload();
                return res;
            };

    auto const forbidden_response =
            [&req](beast::string_view what)
            {
                http::response<http::string_body> res{http::status::forbidden, req.version()};
                res.set(http::field::content_type, "text/plain");
                res.keep_alive(req.keep_alive());
                res.body() = "Forbidden: '" + std::string(what) + "'";
                res.prepare_payload();
                return res;
            };


    // Make sure we can handle the method
    if( req.method() != http::verb::get &&
        req.method() != http::verb::post &&
        req.method() != http::verb::delete_)
        return send(bad_request("Unknown HTTP-method"));

    // GET
    if(req.method() == http::verb::get) {
        std::string path = req.target().to_string();

        //check if authorized
        auto auth = req[http::field::authorization];
        if(auth.empty()){
            return send(forbidden_response("Token needed"));
        }
        std::string token = auth.to_string();
        std::optional<std::string> user = verifyToken(token);
        if (!user.has_value()) {
            //invalid token
            return send(forbidden_response("Invalid token"));
        }

        //if starts with probe
        if (path.rfind("/probefile/", 0) == 0) {
            path = path.substr(11);

            std::optional<std::string> digest_opt = get_file_digest(user.value(), path);

            if(digest_opt){
                //file exists
                std::string digest = digest_opt.value();
                http::response<http::string_body> res{
                        http::status::ok,
                        req.version(),
                        digest};
                //res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/plain");
                res.content_length(digest.size());
                res.keep_alive(req.keep_alive());
                return send(std::move(res));
            } else {
                //file not found
                return send(not_found());
            }
        }


        /*
        if (path.rfind("/probefolder/", 0) == 0) {
            path = path.substr(13);
            bool res = probe_directory(user.value(),path);
            if(res){
                //folder exists
                return send(okay_response());
            } else {
                //folder not found
                return send(not_found());
            }
        }
         */

        return send(not_found());
    }

    // POST
    if(req.method() == http::verb::post) {
        std::string req_path = req.target().to_string();

        //login request
        if (req_path.rfind("/login", 0) == 0){
            json j = json::parse(req.body());

            std::string username;
            std::string password;

            try {
                username = j.at("username");
                password = j.at("password");
            }
            catch (json::out_of_range& e){
                //missing login parameters
                return send(bad_request("Missing login parameters"));
            }

            if (verifyUserPassword(username, password)){
                // the user exists and the password is verified

                // create token
                std::string token = createToken(32);

                // save token related to user
                if(!saveTokenToUser(username, token))
                    return send(server_error("Error in creating token to user"));

                // send token
                http::response<http::string_body> res{
                        http::status::ok,
                        req.version(),
                        token};
                res.set(http::field::content_type, "text/plain");
                res.content_length(token.size());
                res.keep_alive(req.keep_alive());
                return send(std::move(res));
            }
            else {
                // if the verification of the password fails send error
                return send(server_error("Authentication failed"));
            }
        }


        //check if authorized
        auto auth = req[http::field::authorization];
        if(auth.empty()){
            std::cout << "token empty" << std::endl;
            return send(forbidden_response("Token needed"));
        }
        std::string token = auth.to_string();
        std::optional<std::string> user = verifyToken(token);
        if (!user.has_value()) {
            std::cout << "invalid token" << std::endl;
            return send(forbidden_response("Invalid token"));
        }

        //avoid path traversal
        if(req_path.find("..")!=std::string::npos)
            return send(bad_request("Bad path"));

        //if starts with backup
        if (req_path.rfind("/backup/", 0) == 0){

            const std::string path = req_path.substr(8);

            json j = json::parse(req.body());

            std::string type;
            try{
                //path = j.at("path");
                type = j.at("type");
            }catch(json::out_of_range& e){
                //missing parameters
                return send(bad_request("Missing parameters"));
            }



            if(type == "file") {
                std::string encodedfile;
                try{
                    encodedfile = j.at("encodedfile");
                }catch(json::out_of_range& e){
                    //missing parameters
                    return send(bad_request("Missing parameters"));
                }


                //should check if valid base64?

                std::size_t max_l = base64::decoded_size(encodedfile.size());
                std::unique_ptr<char[]> raw_file{new char[max_l]};
                std::pair<std::size_t, std::size_t> res = base64::decode(raw_file.get(), encodedfile.c_str(), encodedfile.size());

                if (save_file(user.value(), path, std::move(raw_file), res.first)) {
                    std::cout << user.value() << "/" << path << std::endl;
                    return send(okay_response());
                } else
                    return send(server_error("Impossible save the file, retry"));

            } else if (type == "folder"){
                if(new_directory(user.value(),path)){
                    std::cout << user.value() << "/" << path << std::endl;
                    return send(okay_response());
                } else {
                    return send(server_error("Impossible create the folder"));
                }
            } else {
                return send(bad_request("Illegal type"));
            }
        }

        if (req_path.rfind("/probefolder/", 0) == 0) {
            std::string path = req_path.substr(13);

            json j = json::parse(req.body());

            std::set<std::string> children;
            try{
                //path = j.at("path");
                j.at("children").get_to(children);
            }catch(json::exception & e){
                //missing parameters
                return send(bad_request("Bad request body"));
            }


            bool res = probe_directory(user.value(),path,children);

            if(res){
                //folder exists
                return send(okay_response());
            } else {
                //folder not found
                return send(not_found());
            }
        }

        if (req_path.rfind("/logout", 0) == 0){

            if (logoutUser(user.value())) {
                return send(okay_response());
            } else {
                return send(server_error("Error during logout"));
            }

        }
        return send(bad_request("Illegal request"));
    }

    //DELETE
    if (req.method() == http::verb::delete_){
        std::string req_path = req.target().to_string();

        auto auth = req[http::field::authorization];
        if(auth.empty()){
            return send(forbidden_response("Token needed"));
        }

        std::string token = auth.to_string();
        std::optional<std::string> user = verifyToken(token);

        if (!user.has_value()) {
            //invalid token
            return send(forbidden_response("Invalid token"));
        }


        //if starts with backup
        if (req_path.rfind("/backup/", 0) == 0){
            std::string path = req_path.substr(8);

            //avoid path traversal
            if(path.find("..")!=std::string::npos)
                return send(bad_request("Bad path"));

            if(backup_delete(user.value(),path))
                return send(okay_response());
            else
                return send(not_found());
        }

        return send(bad_request("Illegal request"));
    }


    /*
    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return send(bad_request("Illegal request-target"));
    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());
    if(req.target().back() == '/')
        path.append("index.html");
    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);
    // Handle the case where the file doesn't exist
    if(ec == beast::errc::no_such_file_or_directory)
        return send(not_found(req.target()));
    // Handle an unknown error
    if(ec)
        return send(server_error(ec.message()));
    // Cache the size since we need it after the move
    auto const size = body.size();
    // Respond to HEAD request
    if(req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }
    // Respond to GET request
    http::response<http::file_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
     */
}


#endif //SERVER_PROGETTO_SERVER_H