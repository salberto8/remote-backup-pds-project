import requests
import json  
import base64
import sys


if(len(sys.argv) != 2):
	print("Usage: " + sys.argv[0] + " input")
	exit(-1)

f = open(sys.arcv[1],'rw')
encodedfile = base64.b64encode(f.read()).decode()


body = {'filename': 'provapy',
	'encodedfile': encodedfile}


#token for 'user0' signed with secret.txt 
token = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjoidXNlcjAifQ.WJ8sE4vIBp2SOpsZlXVFPTMtQa8jdhfPme4FBQhrTUs'

headers = {'content-type': 'application/json',
			'Authorization' : token}

myurl = "http://127.0.0.1:12345/backup/"
req = requests.post(myurl,json=body,headers=headers)

print(req)
print(req.text)