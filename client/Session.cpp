//
// Created by giacomo on 04/08/20.
//

#include "Session.h"

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}
