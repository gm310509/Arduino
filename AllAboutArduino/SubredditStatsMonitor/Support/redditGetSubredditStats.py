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

# Declare an empty dictionary to hold the historical
# subscription numbers
memberHistory = {}


# Return the number of subscribers for a specified
# date if it exists in the history.
def getSubscriberCount(date):
  if date in memberHistory:
    return memberHistory[date]
  else:
    return None

# Calculate the difference between two susbscriber counts
# and allow for the possibility that the previous one
# might not exist.
def calcSubscriberDifference(current, previous):
  if previous is not None:
    return current - previous
  else:
    return "--"


# Send the current data to the attached device via
# the serial device provided by the arduino variable.
def sendArduinoData(arduino,
    activeUserCount, subscribers,
    newSinceYesterday,
    newThisWeek,
    newThisThirtyDays,
    newThisNinetyDays,
    nextTarget, estimatedTargetDate,
    subreddit):

  # work out the targetDate allowing for the possibility that the
  # supplied value might be none.
  targetDate = estimatedTargetDate if estimatedTargetDate is not None else "--"
  msg = f"{activeUserCount},{subscribers},{calcSubscriberDifference(subscribers, yesterdaySubscribers)},{calcSubscriberDifference(subscribers, lastSevenDaysSubscribers)},{calcSubscriberDifference(subscribers, lastThirtyDaysSubscribers)},{calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers)},{nextTarget},{targetDate},{subreddit}"
#   print(msg)
  print("Sending command {}".format(msg))
  arduino.write("{}\n".format(msg).encode())
  arduino.flush()         # ensure the data is sent to the attached device.



# Start of main program.
# output a startup message.
print("Subreddit monitor")
print("by: gm310509")
print("    2022-08-24")
print(f"Version: {VERSION}\n")


# define some global variables.
subreddit = "Arduino"       # Subreddit to monitor - default is r/Arduino
arduinoPort = None
arduinoBaud = None

# Check command line arguments
if len(sys.argv) >= 3:      # Ensure that we have at least the port and baud.
  arduinoPort = sys.argv[1]
  arduinoBaud = sys.argv[2]
else:
  print(f"{os.path.basename(sys.argv[0])} port speed [subreddit name]")
  sys.exit(1)

# Check for an optional third parameter - the name of an alternative subreddit to monitor.
if len(sys.argv) == 4:
  subreddit = sys.argv[3]

# echo the operational parameters.
print(f"Arduino: {arduinoPort} at {arduinoBaud} for {subreddit}")


# Define a user agent so that reddit doesn't "block us".
userAgent = "SubredditMon/0.0.1"
print("user agent:       {}".format(userAgent))

# setup our header info, which gives reddit a brief description of our app
headers = {'User-Agent': userAgent}
requestParameters = {}

#requestText = "http://localhost:8084/RedditStatsService/GetSubRedditMetric?subName={}".format(subreddit)       # for debugging/testing only.
requestText = "http://www.gm310509.com:8080/RedditStatsService/GetSubRedditMetric?subName={}".format(subreddit)
historyText = "&history=1"              # Our first call will request history. This string is tacked onto the end of the requestText.

print("Request:          {}".format(requestText))

todayDate = None              # Initially set to None to force calculation of "change of date" values
prevDataGetTime = 0           # Used to check when it is time to refresh the data.
arduinoOnline = False         # Tracks if the Arduino is online.
inBuf = ""                    # A buffer to receive characters from the Arduino.

# Main loop. Open the Arduino. Use a read timeout of 1 second to allow multi-tasking.
with serial.Serial(arduinoPort, arduinoBaud, timeout=1) as arduino:
  while True:
    # check if there is any data to be read from the Arduino.
    ch = arduino.read()
    if (len(ch) > 0):
      try:
        ch = ch.decode('utf-8')       # Decode the data - the try/catch allows for
                                      # invalid characters that seem to come from
                                      # the Arduino reset.
        #print("Got a {}, inbuf: {}".format(ch, inBuf))
        if (ch == '\n'):              # End of line on input, so we can process it.
          print(inBuf)                # print the input received so we can read it.
          if (inBuf == "Ready"):      # Check if the input is Ready (which the Arduino sends on startup).
            arduinoOnline = True      # Mark that the Arduino is not online.
            sendArduinoData(arduino,
                activeUserCount, subscribers,
                calcSubscriberDifference(subscribers, yesterdaySubscribers),
                calcSubscriberDifference(subscribers, lastSevenDaysSubscribers),
                calcSubscriberDifference(subscribers, lastThirtyDaysSubscribers),
                calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers),
                nextTarget,estimatedTargetDate,
                subreddit)            # Now that the Arduino is online, send it the first data parcel.
                

          inBuf = ""                  # Now that we have processed the input, reset the buffer for new input.
        elif ch != '\r':
          inBuf += ch                 # Not an end of line character, just tack the character to the end of
                                      # the buffer.
      except UnicodeDecodeError as e: # catch (and ignore) unicode conversion errors.
        print(f"UnicodeDecodeError: {e}")

    # Is it time to update our data from the online service?
    if time.time() - prevDataGetTime < 60:
      continue                        # Not yet time to get new data, so skip to the end of the loop.

    # it is time to refresh our statistics.
    # remember the time now for our next check.
    prevDataGetTime = time.time()

    # Get the refreshed stats.
    try:
      res = requests.get(requestText + historyText, headers=headers, params=requestParameters)

    # If the status code isn't 200 (OK), then print an error and skip the rest of the loop.
      if (res.status_code != 200):
        print(f"At {now} Response code: {res.status_code}")
        continue                      # didn't get a good response, so don't do anything more with it.

      print("Response:")
      print (res)

      jsonData = res.json()

      historyText = ""                # Now that we have requested the history, and successfully
                                      # interpreted is as JSON, we don't need to keep getting
                                      # the history, so drop the history part of the request.

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
      prevTodayDate = todayDate       # Record the last known todays date to see if the date has changed.
      todayDate = now.date()          # get todays date.

      # Extract the current values from our data.
      subscribers = jsonData["subscribers"]
      activeUserCount = jsonData["activeUsers"]
      history = jsonData["subscriberHistory"]
      #print(type(history))
      #print(history)

      # Store any history data received from the service into our history dictionary.
      for dateStr in history:
        histDate = datetime.datetime.strptime(dateStr, "%Y-%m-%d").date()
        cnt = history[dateStr]
        memberHistory[histDate] = cnt

#      print("member history data:")
#      for date,cnt in memberHistory.items():
#        print(f"{date},{cnt}")
#      print(f"Total: {len(memberHistory)} days of history.")

      # Save the count of subscribers into the history for todays date.
      memberHistory[todayDate] = subscribers

      # recalculate our KPI dates if the date changed (or first time through).
      dateChanged = todayDate != prevTodayDate
      if dateChanged or prevTodayDate is None:
        yesterdayDate = todayDate - datetime.timedelta(days = 1)
        baseDate = todayDate            # or should we use yesterday date? If we
                                        # did, the display will be much more static.
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

      # Calculate a target value based upon the "scale" of the current subscriber count.
      # for example if the current count is 123,456 then this should result in 200,000.
      # alternatively if the current count is 1,234, then this should result in 2,000.
      # and so on.
      # the target is held in the nextTarget variable.
      subscribersMagnitude = int(math.log10(subscribers))
      nextTarget = (int(subscribers / pow(10, subscribersMagnitude)) + 1) * pow(10, subscribersMagnitude)

      # if we have some history and thus have a dailyRate, calculate the estimated target date values.
      if dailyRate > 0:
        dailyRate = dailyRate / rateCount
        estimatedDaysTo500K = round((nextTarget - baseSubscribers) / dailyRate, 2)
        estimatedTargetDate = todayDate + datetime.timedelta(days = round(estimatedDaysTo500K))

      # Output what we have calculated
      print(f"dailyRate = {dailyRate}")
      print(f"at: {now}, active: {activeUserCount}, subscribers: {subscribers:,}, NEW: since yesterday: {calcSubscriberDifference(subscribers, yesterdaySubscribers)}, last 7 days {calcSubscriberDifference(subscribers, lastSevenDaysSubscribers)}, last 30 days {calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers)}, last 90 days: {calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers)}, {nextTarget:,} subscribers estimate: {estimatedTargetDate}, {estimatedDaysTo500K} days, daily rate: {round(dailyRate,2)}")

      # And if the arduino is active, send the data to it for display.
      if arduinoOnline:
        sendArduinoData(arduino,
            activeUserCount, subscribers,
            calcSubscriberDifference(subscribers, yesterdaySubscribers),
            calcSubscriberDifference(subscribers, lastSevenDaysSubscribers),
            calcSubscriberDifference(subscribers, lastThirtyDaysSubscribers),
            calcSubscriberDifference(subscribers, lastNinetyDaysSubscribers),
            nextTarget,estimatedTargetDate,
            subreddit)

    # Handle any communication/connection errors.
    except requests.exceptions.ConnectionError as e:
      print("Error submitting request: {requestText}")
      print("error: {e}")
      print("code: {e.errno}")
      print("text: {e.strerror}")
