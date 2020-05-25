import request_om2m

#envoi requête HTTP ici
nameAE = "NavStartStop"
data = '"{ \
                   \\"appID\\": \\"app_'+ nameAE +'\\", \
                   \\"category\\": \\"app_value\\", \
                   \\"on\\": \\"True\\" \
                   }" '
url = "http://localhost:8080/~/in-cse/in-name/"+nameAE+"/DATA"
request_om2m.createContentInstance("admin:admin", url, data, "start_stop")
