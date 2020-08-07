//
// Created by giacomo on 31/07/20.
//

#include <thread>
#include <iostream>

#include "FileWatcher.h"
#include "client.h"

FileWatcher::FileWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay)
    : path_to_watch{path_to_watch}, delay{delay} {

    for(auto &path_entry : std::filesystem::recursive_directory_iterator(path_to_watch)) {
        // check if the file/folder exists in the server, if not, backup it
        if(path_entry.is_directory()) {
            if(!probe_folder(path_entry.path().string())) {
                if(!backup_folder(path_entry.path().string())) {
                    // gestire in caso di errore di connessione
                }
            }
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
            // file / folder elimination
            if (!std::filesystem::exists(it->first)) {
                if(!delete_path(it->first)) {
                    // gestire in caso di errore di connessione
                }
                it = paths_.erase(it);
            } else {
                it++;
            }
        }

        for (auto &path_entry : std::filesystem::recursive_directory_iterator(path_to_watch)) {
            auto current_file_last_write_time = std::filesystem::last_write_time(path_entry);

            // file / folder creation
            if (!contains(path_entry.path().string())) {
                if(path_entry.is_directory()) {
                    if(!backup_folder(path_entry.path().string())) {
                        // gestire in caso di errore di connessione
                    }
                }
                else if(path_entry.is_regular_file()) {
                    if(!backup_file(path_entry.path().string())) {
                        // gestire in caso di errore di connessione
                    }
                }
                paths_[path_entry.path().string()] = current_file_last_write_time;

            } else {
                // file / folder modification
                if (paths_[path_entry.path().string()] != current_file_last_write_time) {
                    if(delete_path(path_entry.path().string())) {
                        if(path_entry.is_directory()) {
                            if(!backup_folder(path_entry.path().string())) {
                                // gestire in caso di errore di connessione
                            }
                        }
                        else if(path_entry.is_regular_file()) {
                            if(!backup_file(path_entry.path().string())) {
                                // gestire in caso di errore di connessione
                            }
                        }
                    }
                    paths_[path_entry.path().string()] = current_file_last_write_time;
                }
            }
        }
    }
}
