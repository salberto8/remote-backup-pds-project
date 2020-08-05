//
// Created by giacomo on 31/07/20.
//

#include <thread>

#include "FileWatcher.h"
#include "client.h"

FileWatcher::FileWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay)
    : path_to_watch{path_to_watch}, delay{delay} {

    for(auto &path_entry : std::filesystem::recursive_directory_iterator(path_to_watch)) {
        // check if the file/folder exists in the server, if not, backup it
        // check_and_backup(path_entry);
        if(path_entry.is_directory()) {
/*            if(!probe_folder(path_entry.path().string())) {
                if(!backup_folder(path_entry.path().string())) {
                    // gestire in caso di errore di connessione
                    // si può creare una funzione nel main per controllare la connessione
                }
            }*/
        }
        else if(path_entry.is_regular_file()) {
            if(!probe_file(path_entry.path().string())) {
                if(!backup_file(path_entry.path().string())) {
                    // gestire in caso di errore di connessione
                }
            }
        }

        // add the file/folder to the paths_ unordered_map
        paths_[path_entry.path().string()] = std::filesystem::last_write_time(path_entry);
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
