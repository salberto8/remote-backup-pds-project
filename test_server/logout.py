import requests
import json  
import base64
import sys



body = {'username': 'user1',
	'password': 'pwd1'}

token = 'a62878996a5358605685f6e43754513e671046a7159f353abaae8c9eaa324'

headers = {'Authorization' : token}

myurl = "http://127.0.0.1:12345/logout"
req = requests.post(myurl,json=body,headers=headers)

print(req)
print(req.text)