import requests
import json  
import base64
import sys


if(len(sys.argv) != 2):
	print("Usage: " + sys.argv[0] + " folderpath")
	exit(-1)



body = {'path': sys.argv[1],
	'type': 'folder'}


#token for 'user0'
token = 'aaa'

headers = {'content-type': 'application/json',
			'Authorization' : token}

myurl = "http://127.0.0.1:12345/backup/"
req = requests.post(myurl,json=body,headers=headers)

print(req)
print(req.text)