import requests
import json  
import base64
import sys


if(len(sys.argv) != 2):
	print("Usage: " + sys.argv[0] + " path")
	exit(-1)



#token for 'user0' 
token = 'aaa'

headers = {'Authorization' : token}

myurl = "http://127.0.0.1:12345/backup/" + sys.argv[1]
req = requests.delete(myurl,headers=headers)

print(req)
print(req.text)