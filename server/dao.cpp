#include "dao.h"

Dao* Dao::instance= nullptr;

/**
 * return the username authenticated with the given token
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
    if ( rc != SQLITE_OK ){
        sqlite3_finalize( stmt );
        return {};
    }

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
 * Get the password(hash) of the user saved in the db
 *
 * @param username of the user
 * @return the password if present, a empty optional otherwise
 */
std::optional<std::string> Dao::getPasswordFromUser(const std::string &username){
    if(!conn_open)
        return {};

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2( db, "SELECT hash FROM users WHERE username = ?", -1, &stmt, 0 );
    if ( rc != SQLITE_OK )
        return {};

    //  Bind-parameter indexing is 1-based.
    rc = sqlite3_bind_text( stmt, 1, username.c_str(), username.size(), nullptr); // Bind first parameter.
    if ( rc != SQLITE_OK ){
        sqlite3_finalize( stmt );
        return {};
    }

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
 * Insert token related to user in the db
 *
 * @param username the username of the user
 * @param token the authentication token
 * @return true if the insertion has been applied, false otherwise
 */
bool Dao::insertTokenToUser(const std::string &username, const std::string &token){
    if(!conn_open)
        return {};

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2( db, "UPDATE users SET token=? WHERE username = ?", -1, &stmt, 0 );
    if ( rc != SQLITE_OK )
        return false;

    //  Bind-parameter indexing is 1-based.
    rc = sqlite3_bind_text( stmt, 1, token.c_str(), token.size(), nullptr); // Bind first parameter.
    if ( rc != SQLITE_OK ){
        sqlite3_finalize( stmt );
        return false;
    }
    rc = sqlite3_bind_text(stmt, 2, username.c_str(), username.size(), nullptr);  // Bind second parameter.
    if ( rc != SQLITE_OK ){
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

/**
 * Delete token to user in the db
 *
 * @param username username of the user who wants to logout
 */
bool Dao::deleteTokenToUser(const std::string &username){
    if(!conn_open)
        return {};

    sqlite3_stmt* stmt = nullptr;
    std::string token;

    int rc = sqlite3_prepare_v2( db, "UPDATE users SET token=? WHERE username = ?", -1, &stmt, 0 );
    if ( rc != SQLITE_OK )
        return false;

    //  Bind-parameter indexing is 1-based.
    rc = sqlite3_bind_text( stmt, 1, token.c_str(), token.size(), nullptr); // Bind first parameter.
    if ( rc != SQLITE_OK ){
        sqlite3_finalize( stmt );
        return false;
    }
    rc = sqlite3_bind_text( stmt, 2, username.c_str(), username.size(), nullptr);  // Bind second parameter.
    if ( rc != SQLITE_OK ){
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

/**
 * get all username of users present in the db
 *
 * @return vector of username
 */
std::vector<std::string> Dao::getAllUsers(){
    std::vector<std::string> result;
    if(!conn_open)
        return {};

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2( db, "SELECT username FROM users ", -1, &stmt, 0 );
    if ( rc != SQLITE_OK )
        return {};

    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::string res = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        result.push_back(res);
    }
    if(rc != SQLITE_DONE) {
        printf("ERROR: while performing sql: %s\n", sqlite3_errmsg(db));
        printf("ret_code = %d\n", rc);
        return {};
    }
    else {
        rc = sqlite3_finalize(stmt);
        if (rc == SQLITE_OK)
            return result;
        else
            return {};
    }
}

/**
 * delete tokens of all users in the db
 *
 */
void Dao::deleteAllTokens(){
    if(!conn_open)
        return ;

    sqlite3_stmt* stmt = nullptr;
    std::string token;

    int rc = sqlite3_prepare_v2( db, "UPDATE users SET token=? ", -1, &stmt, 0 );
    if ( rc != SQLITE_OK )
        return;

    //  Bind-parameter indexing is 1-based.
    rc = sqlite3_bind_text( stmt, 1, token.c_str(), token.size(), nullptr);  // Bind first parameter.
    if( rc != SQLITE_OK ){
        sqlite3_finalize( stmt );
        return;
    }

    rc = sqlite3_step( stmt );
    sqlite3_finalize( stmt );
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

Dao::~Dao() {
    if(conn_open)
        sqlite3_close(db);
}

Dao *Dao::getInstance() {

    std::call_once(inited, []() {
        instance = new Dao;
    });

    return instance;
}

std::once_flag Dao::inited;