#!/usr/bin/python

# Reddit metrics monitor proxy.
# This script gets data from the reddit metrics server and
# relays it to a connected device for display.
#
# V1.0.1.0 - 2022-08-25
#    Updated to output the subreddit name to the connected device.
#
# V1.0.0.0 - 2022-08-23
#    Initial version.


import sys
import os
import math
import requests
import json
import datetime
import time
import serial

VERSION = "1.0.1.0"

memberHistory = {}


def getSubscriberCount(date):
  if date in memberHistory:
    return memberHistory[date]
  else:
    return None

def calcSubscriberDifference(current, previous):
  if previous is not None:
    return current - previous
  else:
    return "--"



def sendArduinoData(arduino,
    activeUserCount, subscribers,
    newSinceYesterday,
    newThisWeek,
    newThisThirtyDays,
    newThisNinetyDays,
    nextTarget, estimatedTargetDate,
    subreddit):

  targetDate = estimatedTargetDate if estimatedTargetDate is not None else "--"
  msg = f"{activeUserCount},{subscribers},{calcSubscriberDifference(subscribers, yesterdaySubscribers)},{calcSubscriberDifference(subscribers, lastSevenDaysSubscribers)},{calcSubscriberDifference(subscribers, lastThirtyDaysSubscribers)},{calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers)},{nextTarget},{targetDate},{subreddit}"
#   print(msg)
  print("Sending command {}".format(msg))
  arduino.write("{}\n".format(msg).encode())
  arduino.flush()



print("Subreddit monitor")
print("by: gm310509")
print("    2022-08-24")
print(f"Version: {VERSION}\n")


subreddit = "Arduino"
arduinoPort = None
arduinoBaud = None

if len(sys.argv) >= 3:
  arduinoPort = sys.argv[1]
  arduinoBaud = sys.argv[2]
else:
  print(f"{os.path.basename(sys.argv[0])} port speed [subreddit name]")
  sys.exit(1)

if len(sys.argv) == 4:
  subreddit = sys.argv[3]

print(f"Arduino: {arduinoPort} at {arduinoBaud} for {subreddit}")



userAgent = "SubredditMon/0.0.1"
print("user agent:       {}".format(userAgent))

# setup our header info, which gives reddit a brief description of our app
headers = {'User-Agent': userAgent}
requestParameters = {}

#requestText = "http://localhost:8084/RedditStatsService/GetSubRedditMetric?subName={}".format(subreddit)
#requestText = "http://www.gm310509.com:8080/RedditStatsService/GetSubRedditMetric?subName={}".format(subreddit)
requestText = "http://3.24.91.80:8080/RedditStatsService/GetSubRedditMetric?subName={}".format(subreddit)
historyText = "&history=1"

print("Request:          {}".format(requestText))

todayDate = None
prevDataGetTime = 0
arduinoOnline = False
inBuf = ""

with serial.Serial(arduinoPort, arduinoBaud, timeout=1) as arduino:
  while True:
    # check if there is any data to be read from the Arduino.
    ch = arduino.read()
    if (len(ch) > 0):
      try:
        ch = ch.decode('utf-8')
        #print("Got a {}, inbuf: {}".format(ch, inBuf))
        if (ch == '\n'):
          print(inBuf)
          if (inBuf == "Ready"):
            arduinoOnline = True
            sendArduinoData(arduino,
                activeUserCount, subscribers,
                calcSubscriberDifference(subscribers, yesterdaySubscribers),
                calcSubscriberDifference(subscribers, lastSevenDaysSubscribers),
                calcSubscriberDifference(subscribers, lastThirtyDaysSubscribers),
                calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers),
                nextTarget,estimatedTargetDate,
                subreddit)
                

          inBuf = ""
        elif ch != '\r':
          inBuf += ch
      except UnicodeDecodeError as e:
        print(f"UnicodeDecodeError: {e}")

    if time.time() - prevDataGetTime < 60:
      continue                     # Not yet time to get new data, so skip to the end of the loop.

    # it is time to refresh our statistics.
    # remember the time now for our next check.
    prevDataGetTime = time.time()

    # Get the refreshed stats.
    try:
      res = requests.get(requestText + historyText, headers=headers, params=requestParameters)
      historyText = ""

    # If the status code isn't 200 (OK), then print an error and skip the rest of the loop.
      if (res.status_code != 200):
        print(f"At {now} Response code: {res.status_code}")
        continue
      print("Response:")
      print (res)

      jsonData = res.json()

      # for debugging, print and store the response details.
      #  jsonStr = json.dumps(jsonData)
      #  #print("\n\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n")
      #  #print(jsonStr)
      #  print("\nfrom {}".format(requestText))
      #  responseFilename = "responseStats.json"
      #  with open(responseFilename, "w") as f:
      #    f.write(jsonStr)
      #    f.write("\n")

      # Get todays date and time.
      now = datetime.datetime.now()
      prevTodayDate = todayDate
      todayDate = now.date()

      subscribers = jsonData["subscribers"]
      activeUserCount = jsonData["activeUsers"]
      history = jsonData["subscriberHistory"]
      print(type(history))
      print(history)

      for dateStr in history:
        histDate = datetime.datetime.strptime(dateStr, "%Y-%m-%d").date()
        cnt = history[dateStr]
        memberHistory[histDate] = cnt

#      print("member history data:")
#      for date,cnt in memberHistory.items():
#        print(f"{date},{cnt}")
#      print(f"Total: {len(memberHistory)} days of history.")

      # Save the count of subscribers into the history.
      memberHistory[todayDate] = subscribers

      # recalculate our KPI dates if the date changed (or first time through).
      dateChanged = todayDate != prevTodayDate
      if dateChanged or prevTodayDate is None:
        yesterdayDate = todayDate - datetime.timedelta(days = 1)
        baseDate = todayDate            # or yesterday date?
        lastWeekDate =  baseDate - datetime.timedelta(days = 7)
        lastThirtyDaysDate =  baseDate - datetime.timedelta(days = 30)
        lastNinetyDaysDate =  baseDate - datetime.timedelta(days = 90)

      # Calculate the various metrics of new members.
      yesterdaySubscribers = getSubscriberCount(yesterdayDate)
      baseSubscribers = subscribers         # or yesterdaySubscribers?
      lastSevenDaysSubscribers = getSubscriberCount(lastWeekDate)
      lastThirtyDaysSubscribers = getSubscriberCount(lastThirtyDaysDate)
      lastNinetyDaysSubscribers = getSubscriberCount(lastNinetyDaysDate)
      
      # TODO: Consider weighting these so that the 30 days rate has more influence
      #       then the 7 days rate and the 90 days rate has even more influence than
      #       the other two rates.
      dailyRate = 0
      rateCount = 0
      if lastSevenDaysSubscribers is not None:
        dailyRate = dailyRate + calcSubscriberDifference(baseSubscribers, lastSevenDaysSubscribers) / 7
#         print(f"Seven day rate: {dailyRate}, lastSevenDays: {lastWeekDate}: {lastSevenDaysSubscribers}")
        rateCount = rateCount + 1
      if lastThirtyDaysSubscribers is not None:
        dailyRate = dailyRate + calcSubscriberDifference(baseSubscribers, lastThirtyDaysSubscribers) / 30
        rateCount = rateCount + 1
      if lastNinetyDaysSubscribers is not None:
        dailyRate = dailyRate + calcSubscriberDifference(baseSubscribers, lastNinetyDaysSubscribers) / 90
        rateCount = rateCount + 1

      estimatedDaysTo500K = None
      estimatedTargetDate = None

      subscribersMagnitude = int(math.log10(subscribers))
      nextTarget = (int(subscribers / pow(10, subscribersMagnitude)) + 1) * pow(10, subscribersMagnitude)

      if dailyRate > 0:
        dailyRate = dailyRate / rateCount
        estimatedDaysTo500K = round((nextTarget - baseSubscribers) / dailyRate, 2)
        estimatedTargetDate = todayDate + datetime.timedelta(days = round(estimatedDaysTo500K))

      print(f"dailyRate = {dailyRate}")
      print(f"at: {now}, active: {activeUserCount}, subscribers: {subscribers:,}, NEW: since yesterday: {calcSubscriberDifference(subscribers, yesterdaySubscribers)}, last 7 days {calcSubscriberDifference(subscribers, lastSevenDaysSubscribers)}, last 30 days {calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers)}, last 90 days: {calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers)}, {nextTarget:,} subscribers estimate: {estimatedTargetDate}, {estimatedDaysTo500K} days, daily rate: {round(dailyRate,2)}")

      if arduinoOnline:
        sendArduinoData(arduino,
            activeUserCount, subscribers,
            calcSubscriberDifference(subscribers, yesterdaySubscribers),
            calcSubscriberDifference(subscribers, lastSevenDaysSubscribers),
            calcSubscriberDifference(subscribers, lastThirtyDaysSubscribers),
            calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers),
            nextTarget,estimatedTargetDate,
            subreddit)

    except requests.exceptions.ConnectionError as e:
      print("Error submitting request: {requestText}")
      print("error: {e}")
      print("code: {e.errno}")
      print("text: {e.strerror}")
