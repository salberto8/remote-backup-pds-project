//
// Created by giacomo on 31/07/20.
//

#ifndef CLIENT_FILEWATCHER_H
#define CLIENT_FILEWATCHER_H


#include <string>
#include <filesystem>
#include <unordered_map>


enum class FileStatus {created, modified, erased};

class FileWatcher {
public:
    FileWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay);

    void start();

private:
    std::string path_to_watch;
    std::chrono::duration<int, std::milli> delay;

    // unordered_map: path of the file and its last modification time
    std::unordered_map<std::string, std::filesystem::file_time_type> paths_;
//    // unordered_multimap: path of the file and its status
//    std::unordered_multimap<std::string, FileStatus> change_queue;

    bool running_ = true;

    // Check if "paths_" contains a given key
    bool contains(const std::string &key) {
        auto el = paths_.find(key);
        return el != paths_.end();
    }
};


#endif //CLIENT_FILEWATCHER_H
