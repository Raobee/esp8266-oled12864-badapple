# coding:utf-8
import json

from flask import Flask, request

fp = open("./badapple/allHex.json","r")
js = fp.read()
allHex = json.loads(js)

startmillis = 0
currentIndex = 0

app = Flask(__name__)




# get
@app.route('/badapple', methods=['POST'])
def post_task():
    param = request.json
    print(param)

    global currentIndex,startmillis
    ret = {}

    if('millis' not in param.keys()):
        return json.dumps({})
    
    millis = int(param['millis'])

    if('start' in param.keys()):
        startmillis = param['millis'] + 1000
        currentIndex = 0
    
    if(int((millis-startmillis)/200)>currentIndex):
        if((int((millis-startmillis)/200)>len(allHex))):
            startmillis = millis
            currentIndex = 0
            return json.dumps({})
        print({"currentIndex":currentIndex,"startmillis":startmillis})
        ret = {"displayHex":allHex[currentIndex]} #
        print(ret)
        currentIndex += 1
        pass
    
    return json.dumps(ret) 


if __name__ == '__main__':
    app.run(debug=False,host='0.0.0.0', port=8002)