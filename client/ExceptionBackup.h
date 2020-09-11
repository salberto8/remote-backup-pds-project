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

 enum ErrorType {
     async_resolver_error,
     async_connection_error,
     async_write_error,
     async_read_error,
     async_shutdown_error,
     http_error
 };


class ExceptionBackup: virtual public std::exception {
protected:
    ErrorType error_type;
    http::status http_error_number;
    std::string error_message;

public:

    /**
     * Constructor (C++ STL string, ErrorType)
     *
     *  @param msg The error message
     *  @param type Async error type
     */
    explicit
    ExceptionBackup(std::string  msg, ErrorType type):
            error_type(type),
            error_message(std::move(msg)),
            http_error_number(http::status::unknown)
    {}

    /**
     * Contructor (C++ STL string, boost::beast::http::status)
     *
     * @param msg The error message
     * @param http_error_number HTTP error type
     */
    explicit
    ExceptionBackup(std::string  msg, http::status http_error_number):
            error_type(http_error),
            error_message(std::move(msg)),
            http_error_number(http_error_number)
    {}

    /**
     * Destructor
     */
    ~ExceptionBackup() noexcept override = default;

    /**
     * Returns a pointer to the (constant) error description.
     *
     *  @return A pointer to a const char*. The underlying memory
     *  is in possession of the Except object. Callers must
     *  not attempt to free the memory.
     */
    const char* what() const noexcept override {
        return error_message.c_str();
    }

    /**
     * @return The int value which indicates a ErrorType or a http_error
     */
    virtual int getErrorNumber() const noexcept {
        if(error_type == http_error)
            return static_cast<int>(http_error_number);
        else
            return error_type;
    }

    /**
     * @return The async ErrorType of the exception
     */
    virtual ErrorType getErrorType() const noexcept {
        return error_type;
    }

    /**
     * @return The HTTP error number of the exception
     */
    virtual int getHttpError() const noexcept {
        return static_cast<int>(http_error_number);
    }
};

#endif //CLIENT_EXCEPTIONBACKUP_H
