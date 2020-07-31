//
// Created by giacomo on 31/07/20.
//

#include "FileWatcher.h"


FileWatcher::FileWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay) : path_to_watch{path_to_watch}, delay{delay} {
    for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
        paths_[file.path().string()] = std::filesystem::last_write_time(file);
    }
}

void FileWatcher::start() {
    std::thread threadfw([this] (){
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
    });
    threadfw.detach();
}
