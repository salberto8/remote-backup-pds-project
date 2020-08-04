//
// Created by giacomo on 31/07/20.
//
#include <iostream>
#include "FileWatcher.h"
#include "client.h"
#include "configuration.h"

FileWatcher::FileWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay)
    : path_to_watch{path_to_watch}, delay{delay} {

    for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
        // check if the file/folder exists in the server, if not send it
        if(file.is_directory()) {
            //std::cout << "path: " << file.path().string().erase(0, configuration::backup_path.length()) << std::endl;
            if(!probe_directory(file.path().string())) {
                backup(file.path().string());
            }
        }
        else if(file.is_regular_file()) {
            if(!probe_file(file.path().string())) {
                //creo la copia sul server
                backup(file.path().string());
            }
        }

        // add the file/folder to the paths_ unordered_map
        paths_[file.path().string()] = std::filesystem::last_write_time(file);
    }
}

void FileWatcher::start() {
    while (running_) {
        // Wait for "delay" milliseconds
        std::this_thread::sleep_for(delay);

        auto it = paths_.begin();
        while (it != paths_.end()) {
            // File elimination
            if (!std::filesystem::exists(it->first)) {
                std::pair<std::string, FileStatus> path(it->first, FileStatus::erased);
                change_queue.insert(path);
                it = paths_.erase(it);
            } else {
                it++;
            }
        }

        for (auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
            auto current_file_last_write_time = std::filesystem::last_write_time(file);

            // File creation
            if (!contains(file.path().string())) {
                paths_[file.path().string()] = current_file_last_write_time;
                std::pair<std::string, FileStatus> path(file.path().string(), FileStatus::created);
                change_queue.insert(path);
            } else {
                // File modification
                if (paths_[file.path().string()] != current_file_last_write_time) {
                    paths_[file.path().string()] = current_file_last_write_time;
                    std::pair<std::string, FileStatus> path(file.path().string(), FileStatus::modified);
                    change_queue.insert(path);
                }
            }
        }

        //if(!change_queue.empty())
        //    try_to_upload();
    }
}
