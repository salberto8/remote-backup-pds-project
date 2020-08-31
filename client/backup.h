//
// Created by giacomo on 05/08/20.
//

#ifndef CLIENT_BACKUP_H
#define CLIENT_BACKUP_H


#include <string>
#include <set>

// calculate digest of a file
std::string calculate_digest(std::string path);

// encode a file in base64
std::unique_ptr<char[]> encode(const std::string &original_path);

// get a set of direct children of a directory
std::set<std::string> get_children(const std::string &path);


#endif //CLIENT_BACKUP_H
