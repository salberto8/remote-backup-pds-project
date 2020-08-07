#include <iostream>

#include "configuration.h"
#include "FileWatcher.h"
#include "client.h"

int main() {
    // load the config file
    if(!configuration::load_config_file("backup.conf")){
        return EXIT_FAILURE;
    }
    if(!std::filesystem::exists(configuration::backup_path)) {
        std::cerr << configuration::backup_path << " not exists" << std::endl;
        return EXIT_FAILURE;
    }

    if(!authenticateToServer()){
        std::cerr << "Impossible to authenticate to server " << std::endl;
        return EXIT_FAILURE;
    }

    // FileWatcher refer to a path with a time interval at which we check for changes
    FileWatcher fw{configuration::backup_path, std::chrono::milliseconds(5000)};
    fw.start();
    //std::thread thread_fw(&FileWatcher::start, &fw);

    // ...

    //thread_fw.join();


    return 0;
}
