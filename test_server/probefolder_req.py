import requests
import json  
import base64
import sys


if(len(sys.argv) != 2):
	print("Usage: " + sys.argv[0] + " folder_path")
	exit(-1)



#token for 'user0' 
token = 'aaa'

headers = {'Authorization' : token}

body = {'children' : ['aaa','bbb','folder1']}

myurl = "http://127.0.0.1:12345/probefolder/" + sys.argv[1]
req = requests.post(myurl,headers=headers,json=body)

print(req)
print(req.text)