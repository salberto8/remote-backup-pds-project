import requests
import json  
import base64
import sys


if(len(sys.argv) != 2):
	print("Usage: " + sys.argv[0] + " file_path")
	exit(-1)



#token for 'user0' 
token = 'aaa'

headers = {'Authorization' : token}

myurl = "http://127.0.0.1:12345/probefile/" + sys.argv[1]
req = requests.get(myurl,headers=headers)

print(req)
print(req.text)