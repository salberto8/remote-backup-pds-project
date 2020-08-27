//
// Created by giacomo on 07/08/20.
//

#ifndef CLIENT_EXCEPTIONBACKUP_H
#define CLIENT_EXCEPTIONBACKUP_H


#include <exception>
#include <string>
#include <utility>
#include <boost/beast/http.hpp>

namespace http = boost::beast::http;       // from <boost/beast/http.hpp>


/**  ERROR_NUMBER:
 *   1) async resolver error
 *   2) async connection error
 *   3) async write error
 *   4) async read error
 *   5) async shutdown error
 */
 enum ErrorType {async_resolver_error,async_connection_error,async_write_error,async_read_error,async_shutdown_error,http_error};
/*
#define async_resolver_error 1
#define async_connection_error 2
#define async_write_error 3
#define async_read_error 4
#define async_shutdown_error 5
*/

/**  ERROR_CATEGORY:
 *   0)  async error
 *   10) http error
 */
 /*
#define async_error 0
#define http_error 10
*/

class ExceptionBackup: virtual public std::exception {
protected:
    /*
    int error_number;               ///< Error number
    int error_category;             ///< Error category
    */
    std::string error_message;      ///< Error message

    ErrorType error_type;
    http::status http_error_number;
public:

    /** Constructor (C++ STL string, int).
     *  @param msg The error message
     *  @param err_num Error number
     */
    explicit
    ExceptionBackup(std::string  msg, ErrorType type):
            error_type(type),
            error_message(std::move(msg)),
            http_error_number(http::status::unknown)
    {}

    explicit
    ExceptionBackup(std::string  msg, http::status http_error_number):
            error_type(http_error),
            error_message(std::move(msg)),
            http_error_number(http_error_number)
    {}
    /** Destructor.
     */
    ~ExceptionBackup() noexcept override = default;

    /** Returns a pointer to the (constant) error description.
     *  @return A pointer to a const char*. The underlying memory
     *  is in possession of the Except object. Callers must
     *  not attempt to free the memory.
     */
    const char* what() const noexcept override {
        return error_message.c_str();
    }



    virtual int getErrorNumber() const noexcept {
        if(error_type == http_error)
            return static_cast<int>(http_error_number);
        else
            return error_type;
    }

    virtual ErrorType getErrorType() const noexcept {
        return error_type;
    }

    virtual int getHttpError() const noexcept {
        return static_cast<int>(http_error_number);
    }
};

#endif //CLIENT_EXCEPTIONBACKUP_H
