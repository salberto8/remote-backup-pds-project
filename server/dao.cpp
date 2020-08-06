//
// Created by stealbi on 20/07/20.
//

#include "dao.h"

Dao* Dao::instance= nullptr;

/**
 * return the username athenticated with the given token
 *
 * @param token the authentication token
 * @return the authenticated username if present, a empty optional otherwise
 */
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


/**
 * constructor of the Dao
 * open the connection with the DB
 */
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