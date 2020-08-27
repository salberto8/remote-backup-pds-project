#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <csignal>

#include "configuration.h"
#include "FileWatcher.h"
#include "client.h"
#include "ExceptionBackup.h"

void signalHandler( int signum ) {
    std::stringstream ss;
    ss << "Exit after signal with code " << signum << std::endl;
    std::cout << ss.str();

    // if the client was authenticated, send logout request to server before exit
    if (!configuration::token.empty())
        logout();

    exit(signum);
}

int main() {
    /*// The io_context is required for all I/O (including network)
    boost::asio::io_context io_context;
    // Capture SIGINT and SIGTERM
    // This code allow to perform a clean shutdown of the server (using a signal or ctrl+C)
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

    signals.async_wait(
            [&io_context](boost::beast::error_code const& ec, int sign)
            {
                // send logout request to server before closing client
                logout();

                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                std::stringstream ss;
                ss << "Exit after signal with code " << sign << std::endl;
                std::cout << ss.str();
                io_context.stop();

            });

    std::thread t([&io_context]{ io_context.run(); });*/
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    try {
        // load the config file
        if(!configuration::load_config_file("backup.conf")){
            return EXIT_FAILURE;
        }

        // check for the existence of the backup path
        if(!fs::exists(configuration::backup_path)) {
            std::cerr << configuration::backup_path << " not exists" << std::endl;
            return EXIT_FAILURE;
        }

        // login to server
        authenticateToServer();

        // FileWatcher refer to a path with a time interval at which we check for changes
        FileWatcher fw{configuration::backup_path, std::chrono::seconds (5)};

        // start a continuous check
        fw.start();

    }
    catch (const ExceptionBackup& e) {
        std::cerr << e.what() << ". Error number " << e.getErrorNumber() << std::endl;
        if(e.getErrorType() == async_connection_error)
            std::cerr << "Address and/or port not valid" << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Termination due to an unexpected error" << std::endl;

        // some rescue actions (maybe)

        return EXIT_FAILURE;
    }

   // t.join();

    return 0;
}
