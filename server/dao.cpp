//
// Created by stealbi on 20/07/20.
//

#include "dao.h"

Dao* Dao::instance= nullptr;

std::optional<std::string> Dao::getUserFromToken(const std::string &token) {
    if(!conn_open)
        return {};


    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2( db, "SELECT username FROM users WHERE token = ?", -1, &stmt, 0 );
    if ( rc != SQLITE_OK )
        return {};


    //  Bind-parameter indexing is 1-based.
    rc = sqlite3_bind_text( stmt, 1, token.c_str(), token.size(), nullptr); // Bind first parameter.

    rc = sqlite3_step( stmt );

    if( rc  == SQLITE_ROW ) { // if query has result-rows.
        std::string result = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        rc = sqlite3_finalize(stmt);
        if (rc == SQLITE_OK)
            return result;
    }

    sqlite3_finalize( stmt );
    return {};
}

std::optional<std::string> Dao::getPasswordFromUser(const std::string &username){
    if(!conn_open)
        return {};


    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2( db, "SELECT hash FROM users WHERE username = ?", -1, &stmt, 0 );
    if ( rc != SQLITE_OK )
        return {};


    //  Bind-parameter indexing is 1-based.
    rc = sqlite3_bind_text( stmt, 1, username.c_str(), username.size(), nullptr); // Bind first parameter.

    rc = sqlite3_step( stmt );

    if( rc  == SQLITE_ROW ) { // if query has result-rows.
        std::string result = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        rc = sqlite3_finalize(stmt);
        if (rc == SQLITE_OK)
            return result;
    }

    sqlite3_finalize( stmt );
    return {};
}

bool Dao::insertTokenToUser(const std::string &username, const std::string &token){
    if(!conn_open)
        return {};

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2( db, "UPDATE users SET token=? WHERE username = ?", -1, &stmt, 0 );
    if ( rc != SQLITE_OK )
        return false;


    //  Bind-parameter indexing is 1-based.
    if ( sqlite3_bind_text( stmt, 1, token.c_str(), token.size(), nullptr) != SQLITE_OK) // Bind first parameter.
    {
        sqlite3_finalize( stmt );
        return false;
    }
    if ( sqlite3_bind_text( stmt, 2, username.c_str(), username.size(), nullptr) != SQLITE_OK)  // Bind second parameter.
    {
        sqlite3_finalize( stmt );
        return false;
    }

    rc = sqlite3_step( stmt );

    sqlite3_finalize( stmt );

    if( rc  != SQLITE_DONE ) { // if query has been executed without errors.
        return false;
    }
    return true;
}

Dao::Dao() {
    std::string path = configuration::dbpath;

    if( sqlite3_open(path.c_str(), &db) != SQLITE_OK ){
        std::cerr << "DB Open Error: " << sqlite3_errmsg(db) << std::endl;
        conn_open = false;
        return;
    }

    conn_open = true;

}

std::once_flag Dao::inited;