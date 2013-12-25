import time
import os
import string
import httplib
import urllib
import urllib2
import base64

try:
	params=urllib.urlencode({"cmd":"python /home/wei/rox/install.py eth1 192.168.6.1 192.168.6.51"})
	headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}
	httpClient = httplib.HTTPConnection("192.168.6.1", 80, timeout=10)
	httpClient.request("POST", "/index.php", params, headers)
	
	response = httpClient.getresponse()
	print response.read()
except Exception, e:
    print e
finally:
    if httpClient:
        httpClient.close()
	


