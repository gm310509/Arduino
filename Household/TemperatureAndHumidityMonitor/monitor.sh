#!/bin/bash

####### Raspberry PI copy of script
SLEEP_TIME=120
ERR=0
CNT=0
LOG_FILE=/tmp/monitor.txt
STOP_FILE=/tmp/monitorStop.txt

if [ -e $STOP_FILE ]
then
  rm $STOP_FILE
fi

> $LOG_FILE

while `true`
do
  MSG=`date "+%Y-%m-%d %H:%M:%S.%N"`
  for HST in S0 S1 S2
  do
      RAW_DATA=`curl -s --connect-timeout 2 $HST:4000`
      STAT=$?
      SENSOR_DATA=`echo "$RAW_DATA" | tr -d '\r\n'`
      if [ $STAT -ne 0 ]
      then
          ERR=$((ERR+1))
      fi
      MSG="$MSG~$HST~$SENSOR_DATA"
  done
  
  CNT=$((CNT+1))
  echo "${MSG}CNT:$CNT,ERR:$ERR"
  echo "${MSG}CNT:$CNT,ERR:$ERR" >> $LOG_FILE
  if [ -e $STOP_FILE ]
  then
    exit 0
  fi

  sleep $SLEEP_TIME

done
