//
// Created by giacomo on 07/08/20.
//

#ifndef CLIENT_EXCEPTIONBACKUP_H
#define CLIENT_EXCEPTIONBACKUP_H


#include <exception>
#include <string>
#include <utility>

class ExceptionBackup: virtual public std::exception {
protected:
    int error_number;               ///< Error number
    std::string error_message;      ///< Error message

public:

    /** Constructor (C++ STL string, int, int).
     *  @param msg The error message
     *  @param err_num Error number
     */
    explicit
    ExceptionBackup(std::string  msg, int err_num):
            error_number(err_num),
            error_message(std::move(msg))
    {}

    /** Destructor.
     *  Virtual to allow for subclassing.
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
};

#endif //CLIENT_EXCEPTIONBACKUP_H
