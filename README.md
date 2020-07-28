# progetto-pds

## Server
### API
All the APIs require authorization with a token (in the authorization header).
Without a valid token -> 403 FORBIDDEN  
See examples in test_server folder
- GET /probefile/{filepath}
  - file exists: return the digest (SHA256) of the file (200 OK)
  - file doesn't exist: 404 NOT FOUND
- GET /probefolder/{folderpath}
  - folder exists: 200 OK
  - folder doesn't exist: 404 NOT FOUND
- POST /backup/{path} 
  send a json body with type ('file' or 'folder'), encodedfile (if is of type file) in base64
  - file/folder saved: 200 OK
  - error otherwise (BAD REQUEST or SERVER ERROR)
- DELETE /backup/{path}  
  remove the file or folder in the specified path (if it's a folder remove RECURSIVELY)
  - file/folder removed: 200 OK
  - file/folder not foud: 404 NOT FOUND
  
### librerie usate nel server
- boost (completo bisogna fare la build almeno di program_options e filesystem)
- nlohmann/json
- openssl (apt install libssl-dev)
- sqlite (apt instal libsqlite3-dev)
