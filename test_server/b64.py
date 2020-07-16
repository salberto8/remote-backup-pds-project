import base64
import sys

if(len(sys.argv) != 3):
	print("Usage: " + sys.argv[0] + " input output")
	exit(-1)
		
inp = open(sys.argv[1],'rb')
out = open(sys.argv[2],'wb')
out.write(base64.b64encode(inp.read()))
inp.close()
out.close()
