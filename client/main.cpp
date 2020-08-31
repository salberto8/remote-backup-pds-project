#include <iostream>
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

    // set handler for signals SIGINT and SIGTERM in order to manage them correctly
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
