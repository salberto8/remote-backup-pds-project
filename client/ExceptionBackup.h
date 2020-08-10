//
// Created by giacomo on 07/08/20.
//

#ifndef CLIENT_EXCEPTIONBACKUP_H
#define CLIENT_EXCEPTIONBACKUP_H


#include <exception>
#include <string>
#include <utility>

/**  ERROR_NUMBER:
 *   1) async resolver error
 *   2) async connection error
 *   3) async write error
 *   4) async read error
 *   5) async shutdown error
 */
#define async_resolver_error 1
#define async_connection_error 2
#define async_write_error 3
#define async_read_error 4
#define async_shutdown_error 5

/**  ERROR_CATEGORY:
 *   0)  async error
 *   10) http error
 */
#define async_error 0
#define http_error 10


class ExceptionBackup: virtual public std::exception {
protected:
    int error_number;               ///< Error number
    int error_category;             ///< Error category
    std::string error_message;      ///< Error message

public:

    /** Constructor (C++ STL string, int).
     *  @param msg The error message
     *  @param err_num Error number
     */
    explicit
    ExceptionBackup(std::string  msg, int err_num):
            error_number(err_num),
            error_message(std::move(msg))
    {
        if(err_num < 10)
            error_category = async_error;
        else
            error_category = http_error;
    }

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

    /** Returns error number.
     *  @return #error_number
     */
    virtual int getErrorNumber() const noexcept {
        return error_number;
    }

    /** Returns error category.
 *  @return #error_category
 */
    virtual int getErrorCategory() const noexcept {
        return error_category;
    }
};

#endif //CLIENT_EXCEPTIONBACKUP_H
