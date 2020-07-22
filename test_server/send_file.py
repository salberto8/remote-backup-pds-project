import requests
import json  
import base64
import sys


if(len(sys.argv) != 2):
	print("Usage: " + sys.argv[0] + " file_to_send")
	exit(-1)

f = open(sys.argv[1],'rb')
encodedfile = base64.b64encode(f.read()).decode()


body = {'path': sys.argv[1],
	'type': 'file',
	'encodedfile': encodedfile}


#token for 'user0'
token = 'aaa'

headers = {'content-type': 'application/json',
			'Authorization' : token}

myurl = "http://127.0.0.1:12345/backup/"
req = requests.post(myurl,json=body,headers=headers)

print(req)
print(req.text)