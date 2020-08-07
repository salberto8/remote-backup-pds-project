//
// Created by giacomo on 05/08/20.
//

#ifndef CLIENT_BACKUP_H
#define CLIENT_BACKUP_H


#include <string>
#include <set>

std::string calculate_digest(std::string path);
std::unique_ptr<char[]> encode(const std::string &original_path);
std::set<std::string> get_children(const std::string &path);


#endif //CLIENT_BACKUP_H
