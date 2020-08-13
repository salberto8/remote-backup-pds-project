//
// Created by giacomo on 31/07/20.
//

#include <thread>
#include <iostream>
#include <deque>
#include <condition_variable>

#include "FileWatcher.h"
#include "client.h"
#include "ExceptionBackup.h"

std::mutex om;

void myout(std::string msg){
    std::lock_guard og(om);
    std::cout<<msg<<std::endl;
}


template <class T>
class Jobs {
    // FIFO queue
    std::deque<T> jobs;
    bool active = true;
    std::mutex qm;
    std::condition_variable not_empty;

public:
    void put(T job){
        std::lock_guard _g(qm);
        jobs.push_back(job);
        not_empty.notify_one();
    }

    std::optional<T> get() {
        std::unique_lock lock(qm);
        while (true){

            if(jobs.size()>0){
                T job = jobs.front();
                jobs.pop_front();
                return job;
            }
            if(!active){
                return std::optional<T>();
            }
            not_empty.wait(lock, [this](){ return this->jobs.size()>0 || !this->active;});
        }
    }

    void ended(){
        std::lock_guard g(qm);
        active = false;
        not_empty.notify_all();
    }

    int size(){
        return jobs.size();
    }
};

FileWatcher::FileWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay)
    : path_to_watch{path_to_watch}, delay{delay} {

    try {
        initialization();
    }
    catch (const ExceptionBackup& e) {
        // server error or connection lost
        std::cerr << e.what() << ". Error number " << e.getErrorNumber() << std::endl;
        if(check_connection_and_retry())
            std::cout << "Connection is back" << std::endl;
    }
}

void FileWatcher::initialization() {
    Jobs<std::filesystem::path> jobs;

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    std::atomic<int> count_leaves(0);

    // delete any files or folders no longer present in the root folder
    probe_folder(path_to_watch);

    // erase all the elements in the path_ map
    paths_.clear();


    jobs.put(path_to_watch);
    count_leaves.fetch_add(1);
    /*
    // producer (single thread)
    producers.push_back(std::thread([&jobs, this](){
        int i=1;
        for (auto &path_entry: fs::recursive_directory_iterator(path_to_watch)) {
            if (path_entry.is_directory()){
                myout("inserting directory " + std::to_string(i++) + " " + path_entry.path().string());
                jobs.put(path_entry.path());
            }
        }
        myout("producer terminated");
    }));*/


    // 5 consumer threads
    for(int i=0; i<5; i++){
        consumers.push_back(std::thread([&jobs, this, &count_leaves, i](){
            while(true){
                std::optional<fs::path>  path_ = jobs.get();
                if(path_.has_value()){
                    // safe perche' solo un consumer ricevera' il path
                    fs::path path_entry = path_.value();

                    int directories = 0;
                   /* fs::path parent = path_entry.parent_path();
                    myout("path " + path_entry.string() );
                    myout(" waiting for root " + parent.string());
                    while (!probe_folder(parent)){
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                    }*/
                   // myout("path " + path_entry.string() + " now have root " + parent.string());
                   //myout("here");
                    if(!probe_folder(path_entry.string())) {
                        backup_folder(path_entry.string());
                    }
                    // add the folder to the paths_ unordered_map
                    mutex_paths_.lock();
                    paths_[path_entry.string()] = fs::last_write_time(path_entry);
                    mutex_paths_.unlock();

                    for (auto p : fs::directory_iterator(path_entry))
                        if(p.is_regular_file()) {
                            if(!probe_file(p.path().string())) {
                                myout("trying to backup " + p.path().string());
                                backup_file(p.path().string());
                            }
                            // add the file to the paths_ unordered_map
                            mutex_paths_.lock();
                            paths_[p.path().string()] = fs::last_write_time(p);
                            mutex_paths_.unlock();
                        }
                        else if (p.is_directory()){
                            if (directories == 0 | directories == 1 )
                                directories++;
                            else
                                count_leaves.fetch_add(1);
                            jobs.put(p.path());
                        }
                    if (directories == 0)
                        count_leaves.fetch_sub(1);

                    if (count_leaves.load() == 0){
                        jobs.ended();
                    }
                } else {
                    myout("consumer " + std::to_string(i) + " terminated");
                    break;
                }
            }
        }));
    }

    /*for (auto &p: producers) {
        if(p.joinable()) p.join();
    }*/

    myout("producer terminati");
  //  jobs.ended();

    myout("in attesa consumer, job ancora aperti: " + std::to_string(jobs.size()));

    for (auto &c: consumers) {
        if(c.joinable()) c.join();
    }

    myout("consumer terminati job ancora aperti: " + std::to_string(jobs.size()));


}

bool FileWatcher::check_connection_and_retry() {
    while(retry) {
        // sleep 30 seconds (+ potentially another 30 seconds of connection timeout)
        std::this_thread::sleep_for(std::chrono::seconds(30));

        try {
          /*  if(probe_folder(path_to_watch)) {
                // the server connection is active
                initialization();
            }*/
            authenticateToServer();
            initialization();
            return true;
        }
        catch (const ExceptionBackup& e) {
            // server error or connection lost
            std::cerr << e.what() << ". Error number " << e.getErrorNumber() << std::endl;
            if(e.getErrorCategory() == http_error)
                retry--;
        }
    }
    return false;
}

void FileWatcher::start() {
    while (retry) {
        try {
            // Wait for "delay" milliseconds
            std::this_thread::sleep_for(delay);

            auto it = paths_.begin();
            while (it != paths_.end()) {
                // file / folder elimination
                if (!fs::exists(it->first)) {
                    delete_path(it->first);
                    it = paths_.erase(it);
                } else {
                    it++;
                }
            }

            for (auto &path_entry : fs::recursive_directory_iterator(path_to_watch)) {
                auto current_file_last_write_time = fs::last_write_time(path_entry);

                // file / folder creation
                if (!contains(path_entry.path().string())) {
                    if(path_entry.is_directory()) {
                        backup_folder(path_entry.path().string());
                    }
                    else if(path_entry.is_regular_file()) {
                        backup_file(path_entry.path().string());
                    }
                    paths_[path_entry.path().string()] = current_file_last_write_time;

                } else {
                    // file / folder modification
                    if (paths_[path_entry.path().string()] != current_file_last_write_time) {
                        delete_path(path_entry.path().string());
                        if(path_entry.is_directory()) {
                            backup_folder(path_entry.path().string());
                        }
                        else if(path_entry.is_regular_file()) {
                            backup_file(path_entry.path().string());
                        }
                        paths_[path_entry.path().string()] = current_file_last_write_time;
                    }
                }
            }
        }
        catch (const ExceptionBackup& e) {
            // server error or connection lost
            std::cerr << e.what() << ". Error number " << e.getErrorNumber() << std::endl;
            if(check_connection_and_retry())
                std::cout << "Connection is back" << std::endl;
        }
    }
}
