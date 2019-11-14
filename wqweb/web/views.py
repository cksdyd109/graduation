from django.shortcuts import render
from django.http import JsonResponse
from datetime import datetime, timedelta
from pymongo import MongoClient
# Test real-time graph using matplotlib
import matplotlib.animation as anim
import matplotlib.pyplot as plt
from collections import deque
from collections import OrderedDict
import random
import json
import PIL, PIL.Image
import io
import base64

# Create your views here.

client = MongoClient('mongodb://admin:sps1@52.78.54.131:27017/')
database = client.test
collection = database.test

# time to sync(seconds)
syncTime = 30
timenow = datetime.now()

phLine = deque([0.0] * 10, maxlen=10)
turLine = deque([0.0] * 10, maxlen=10)
temLine = deque([0.0] * 10, maxlen=10)
doLine = deque([0.0] * 10, maxlen=10)

weeklyph = [None] * 7
weeklytem = [None] * 7

def home(request):
    global syncTime
    global weeklyph, weeklytem
    a = []
    b = []
    c = []
    d = []
    dayArray = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday']
    day = datetime.today().weekday()

    for i in range(7):
        time = timenow + timedelta(days=(-1 * i))
        timeend = time + timedelta(days=1)
        time = time.strftime('%Y-%m-%d')
        timeend = timeend.strftime('%Y-%m-%d')

        pipeline = list()
        pipeline.append({'$match': {'date': {'$gte': time, '$lte': timeend}}})
        pipeline.append({'$group': {'_id': None, 'ph': {'$avg': '$ph'}, 'temperature': {'$avg': '$temperature'}}})
        result = collection.aggregate(pipeline)
        for re in result:
            weeklyph[6 - i] = re['ph']
            weeklytem[6 - i] = re['temperature']

    print(weeklytem)
    print(weeklyph)

    waters = collection.find({}, {'_id':0}).sort('date', -1).limit(1)

    for temp in waters:
        water = temp

    informs = collection.find({}, {'_id':0}).sort('date', -1).limit(10)
    for inform in informs:
        a.append(inform['turbidity'])
        b.append(inform['ph'])
        c.append(inform['temperature'])
        d.append(inform['do'])

    syncValue = [30, 60, 300]
    if (request.method == 'POST'):
        syncTime = request.POST.get('sources')

    print(syncTime)

    return render(request, 'content/index.html', {'day':dayArray[day], 'water':water, 'a':a, 'b':b, 'syncTime': int(syncTime), 'syncValue':syncValue})

def settings(request):
    global syncTime

    syncValue = [5, 10, 30, 60, 300]

    if (request.method == 'POST'):
        syncTime = request.POST.get('syncTime')
    print(syncTime)
    return render(request, 'content/settings.html', {'syncTime':int(syncTime), 'syncValue':syncValue})

line = deque([0.0] * 10, maxlen=10)

def realTime(request):
    # test real-time

    global line
    ranNum = random.uniform(4, 8)
    line.append(ranNum)

    dataJson = OrderedDict()
    dataJson['data'] = list(line)
    testjson = json.dumps(dataJson, ensure_ascii=False, indent='\t')

    return JsonResponse({'test': list(line)})

def todayDash(request):
    global phLine, turLine, temLine, doLine
    global timenow
    dayArray = ['월요일', '화요일', '수요일', '목요일', '금요일', '토요일', '일요일']
    day = datetime.today().weekday()

    waterQuality = collection.find({}, {'_id':0}).sort('date', -1)[0]

    dataJson = OrderedDict()
    dataLine = OrderedDict()

    # for water, (key, value) in enumerate(waterQuality.items()):
    #     dataJson[key] = value
    dataJson['today'] = dayArray[day]
    dataJson['turbidity'] = waterQuality['turbidity']
    dataJson['ph'] = waterQuality['ph']
    dataJson['temperature'] = waterQuality['temperature']
    dataJson['do'] = waterQuality['do']

    phLine.append(dataJson['ph'])
    turLine.append(dataJson['turbidity'])
    temLine.append(dataJson['temperature'])
    doLine.append(dataJson['do'])

    dataLine['phline'] = list(phLine)
    dataLine['turline'] = list(turLine)
    dataLine['temline'] = list(temLine)
    dataLine['doline'] = list(doLine)

    checknow = datetime.now()
    if (checknow.strftime('%Y-%m-%d') == (timenow + timedelta(days=1)).strftime('%Y-%m-%d')):
        timenow = checknow
        for i in range(1, 7):
            weeklyph[i - 1] = weeklyph[i]
            weeklytem[i - 1] = weeklytem[i]
        pipeline = list()
        pipeline.append({'$match': {'date': {'$gte': timenow.strftime('%Y-%m-%d'), '$lte': (timenow + timedelta(days=1)).strftime('%Y-%m-%d')}}})
        pipeline.append({'$group': {'_id': None, 'ph': {'$avg': '$ph'}, 'temperature': {'$avg': '$temperature'}}})
        result = collection.aggregate(pipeline)
        for re in result:
            weeklyph[6] = re['ph']
            weeklytem[6] = re['temperature']

    dataLine['weekph'] = list(weeklyph)
    dataLine['weektem'] = list(weeklytem)
    # encodeJson = json.dumps(dataJson)

    return JsonResponse({'todayWater': dataJson, 'realgraph': dataLine})


