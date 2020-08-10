#include <iostream>

#include "configuration.h"
#include "FileWatcher.h"
#include "client.h"
#include "ExceptionBackup.h"


int main() {
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

        authenticateToServer();

//        // check the connection, otherwise it raises an exception
//        probe_folder(configuration::backup_path);

        // FileWatcher refer to a path with a time interval at which we check for changes
        FileWatcher fw{configuration::backup_path, std::chrono::seconds (5)};
        // start a continuous check
        fw.start();

    }
    catch (const ExceptionBackup& e) {
        std::cerr << e.what() << ". Error number " << e.getErrorNumber() << std::endl;
        if(e.getErrorNumber() == async_connection_error)
            std::cerr << "Address and/or port not valid" << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Termination due to an unexpected error" << std::endl;

        // some rescue actions (maybe)

        return EXIT_FAILURE;
    }

    return 0;
}
