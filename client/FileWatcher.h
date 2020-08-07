//
// Created by giacomo on 31/07/20.
//

#ifndef CLIENT_FILEWATCHER_H
#define CLIENT_FILEWATCHER_H


#include <string>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;


class FileWatcher {
public:
    FileWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay);

    void start();

private:
    void initialization();

    std::string path_to_watch;
    std::chrono::duration<int, std::milli> delay;

    // unordered_map: path of the file and its last modification time
    std::unordered_map<std::string, std::filesystem::file_time_type> paths_;

    bool running_ = true;

    // Check if "paths_" contains a given key
    bool contains(const std::string &key) {
        auto el = paths_.find(key);
        return el != paths_.end();
    }

    void check_connection_and_retry();
};


#endif //CLIENT_FILEWATCHER_H
