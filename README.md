# progetto-pds

## Server
### Configuration
Before the first start you should create a config file called backupserver.conf in your home directory.
Here is an example:

```
address=127.0.0.1
port=12345
nthreads=4
backuppath=/home/user/Desktop/test_server/
dbpath=/home/user/Desktop/test_server/backup.db
```

### API
All the APIs require authorization with a token (in the authorization header).
Without a valid token -> 403 FORBIDDEN  
See examples in test_server folder
- GET /probefile/{filepath}
  - file exists: return the digest (SHA256) of the file (200 OK)
  - file doesn't exist: 404 NOT FOUND
- POST /login send a json with 'username' and 'password'
  - authentication success: 200 OK containing the token for the client
  - authentication fail: SERVER ERROR 
- POST /probefolder/{folderpath}
  send a json with 'children', an array with all the (direct) children of the folder
  - folder exists: 
    - check if there are files/folders not in 'children' and remove it
    - answer with 200 OK
  - folder doesn't exist: 404 NOT FOUND
- POST /backup/{path} 
  send a json body with type ('file' or 'folder'), encodedfile (if is of type file) in base64
  - file/folder saved: 200 OK
  - error otherwise (BAD REQUEST or SERVER ERROR)
- POST /logout 
  - token of the user deleted from the database: 200 OK
  - error otherwise: SERVER ERROR
- DELETE /backup/{path}  
  remove the file or folder in the specified path (if it's a folder remove RECURSIVELY)
  - file/folder removed: 200 OK
  - file/folder not foud: 404 NOT FOUND
  
### Libraries used
- boost 1.73.0 (at least program_options must be built)
- nlohmann/json (nlohmann-json3-dev)
- openssl (libssl-dev)
- sqlite (libsqlite3-dev)
  
## Client
### Configuration
Before the first start you should create a config file called backup.conf in your home directory.
Here is an example:

```
address=127.0.0.1
port=12345
backup_path=/home/user/Downloads
username=user1
```

### Libraries used
- boost 1.73.0 (at least program_options must be built)
- nlohmann/json (nlohmann-json3-dev)
- openssl (libssl-dev)
