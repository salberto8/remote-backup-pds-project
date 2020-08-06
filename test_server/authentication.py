import requests
import json  
import base64
import sys



body = {'username': 'user1',
	'password': 'pwd1'}


headers = {'content-type': 'application/json'}

myurl = "http://127.0.0.1:12345/login"
req = requests.post(myurl,json=body,headers=headers)

print(req)
print(req.text)