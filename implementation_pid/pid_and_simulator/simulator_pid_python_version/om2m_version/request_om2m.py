import requests
import json

def handleResponse(r):
	#print (r.status_code)
	#print (r.headers)
	#print (r.text)
	return

"""
##	Create <ACP> 	###
def createACP(origin,url,rn_value,pv_acor_value,pv_acop_value):
	print("Creating <ACP>")
	payload = '{ "m2m:acp": {\
		"rn": "'+rn_value+'",\
		  "pv": {\
			"acr": {\
			  "acor": ["'+pv_acor_value+'"],\
			  "acop": "'+pv_acop_value+'"\
			}\
		  },\
		  "pvs": {\
			"acr": {\
			  "acor": ["'+pv_acor_value+'"],\
			  "acop": "'+pv_acop_value+'"\
			}\
		  }\
	   }\
	}'
	_headers =   {'X-M2M-Origin': origin,'Content-Type': 'application/json;ty=1'}
	json.dumps(json.loads(payload,strict=False),indent=4)
	r = requests.post(url.strip(),data=payload,headers=_headers)
	handleResponse(r)
	return r
"""

###	Create an <AE>	###	
def createAE(CSEurl, api_value, rn_value, label):
	print("Creating <AE> with resourceName and point of access attributes")
	payload = '{ \
		"m2m:ae": { \
		"api": "'+api_value+'", \
		"rr": "true", \
		"rn": "'+rn_value+'", \
		"lbl": "'+label+'" \
		} \
	}'
	_headers = {'X-M2M-Origin': '', 'Content-Type': 'application/json;ty=2'}
	json.dumps(json.loads(payload,strict=False), indent=4)
	r = requests.post(CSEurl.strip(), data=payload, headers=_headers)
	handleResponse(r)
	return r


###	Create a <Container>	###
def createContainer(origin, AEurl, rn_value, label):
	print("Creating <container>")
	payload = '{ \
		"m2m:cnt": { \
			"rn": "'+rn_value+'", \
			"lbl": "'+label+'" \
		} \
	}'
	_headers = {'X-M2M-Origin': origin, 'Content-Type': 'application/json;ty=3'}
	json.dumps(json.loads(payload, strict=False), indent=4)
	r = requests.post(AEurl.strip(), data=payload, headers=_headers)
	handleResponse(r)
	return r


###	Create a <ContentInstance> with mandatory attributes	###
def createContentInstance(origin, CONurl, con_value, label):
	#print("Creating <contentInstance>")
	payload = '{ \
		"m2m:cin": { \
		"con": '+con_value+', \
		"lbl": "'+label+'" \
		} \
	}'
	_headers = {'X-M2M-Origin': origin, 'Content-Type': 'application/json;ty=4'}
	json.dumps(json.loads(payload, strict=False), indent=4)
	r = requests.post(CONurl.strip(), data=payload, headers=_headers)
	handleResponse(r)
	return r

"""
###     Create a <ContentInstance> with mandatory attributes    #$
def createDescriptorContentInstance(origin,CONurl,con_value,label):
		print("Creating <DescriptorcontentInstance>")
		payload = '{ \
			"m2m:cin": { \
			"con": "'+con_value+', \
			"lbl": "'+label+'" \
			} \
		}'
		_headers =   {'X-M2M-Origin': origin,'Content-Type': 'application/json;ty=4'}
		json.dumps(json.loads(payload,strict=False), indent=4)
		r = requests.post(CONurl.strip(),data=payload,headers=_headers)
		handleResponse(r)
		return r


###	Get latest <ContentInstance>	###
def getContentInstanceLatest(origin,CONurl):
	print("Getting Latest  <contentInstance>")
	_headers =   {'X-M2M-Origin': origin,'Accept': 'application/json'}
	r = requests.get(CONurl.strip(),headers=_headers)
	handleResponse(r)
	return r


###	Send an <operation> to a Activator with mandatory attributes	###
def setOperationActuator(origin,url):
	print("Push an operation on Acutator")
	_headers =   {'X-M2M-Origin': origin,'Content-Type': 'application/json'}
	r = requests.post(url.strip(),headers=_headers)
	handleResponse(r)
	return r
"""


def subToAE(origin, listenURL, name, url):
	print("Sub to an Apllication Entity")
	payload = '{ \
				"m2m:sub": { \
				"rn": "' + name + '", \
				"nu": "' + listenURL + '", \
				"nct": 2 \
				} \
			}'
	_headers = {'X-M2M-Origin': origin, 'Content-Type': 'application/json;ty=23'}
	json.dumps(json.loads(payload, strict=False), indent=4)
	r = requests.post(url.strip(), data=payload, headers=_headers)
	handleResponse(r)
	return r


def creationAE_DATA(nameAE):
	# print("test\n")
	url = "http://localhost:8080/~/in-cse/in-name"
	createAE(url, "app_"+nameAE, nameAE, "[Type/sensor]")
	createContainer("admin:admin", url+"/"+nameAE, "DATA", "data")


def deleteSUB(origin, url):
	_headers = {'X-M2M-Origin': origin, 'Content-Type': 'application/json;ty=23'}
	requests.delete(url.strip(), headers=_headers)


def deleteAE(origin, url):
	_headers = {'X-M2M-Origin': origin, 'Content-Type': 'application/json;ty=2'}
	requests.delete(url.strip(), headers=_headers)


def init_om2m(nameAE, port):
	url = "http://localhost:8080/~/in-cse/in-name"
	deleteAE("admin:admin", url+"/"+nameAE)
	creationAE_DATA(nameAE)
	# deleteSUB("admin:admin", url+"/"+nameAE+"/DATA/Sub"+nameAE)
	listen_url = "http://localhost:"+str(port)
	subToAE("admin:admin", listen_url, "Sub" + nameAE, url+"/"+nameAE+"/DATA")
