//
// Created by stealbi on 20/07/20.
//

#ifndef SERVER_DAO_H
#define SERVER_DAO_H

#include <sqlite3.h>
#include <iostream>
#include <future>
#include <mutex>
#include <thread>

#include "configuration.h"


//singleton, interface with database
class Dao{

    sqlite3* db;
    bool conn_open;

    Dao();;

    ~Dao(){
        if(conn_open)
            sqlite3_close(db);
    };


public:
    static Dao* instance;
    static std::once_flag inited;

    static Dao *getInstance(){

        std::call_once(inited, []() {
            instance = new Dao;
        });

        return instance;
    }

    std::optional<std::string> getUserFromToken(const std::string &token);
    std::optional<std::string> getPasswordFromUser(const std::string &username);

    Dao(const Dao&)= delete;
    Dao& operator=(const Dao&)= delete;
};



#endif //SERVER_DAO_H
