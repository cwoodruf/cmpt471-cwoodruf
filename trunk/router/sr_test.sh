#!/bin/bash
srpid=`/bin/pidof sr`
if [ "$srpid" == "" ] 
then
	echo starting sr
	./sr_start.sh
	/bin/sleep 5
fi

echo starting pings
/bin/ping -q 171.67.245.96 &
/bin/ping -q 171.67.245.100 &
/bin/ping -q 171.67.245.101 &
/bin/ping -q 171.67.245.102 &
/bin/ping -q 171.67.245.103 &

echo traceroutes
/usr/bin/tracepath -n 171.67.245.96
/usr/bin/tracepath -n 171.67.245.100
/usr/bin/tracepath -n 171.67.245.101
/usr/bin/tracepath -n 171.67.245.102
/usr/bin/tracepath -n 171.67.245.103

echo downloading big.jpg from both interfaces
while true 
do
/usr/bin/wget --timeout=5 --no-verbose -O ~/Desktop/big_101.jpg http://171.67.245.101/big.jpg 
/usr/bin/wget --timeout=5 --no-verbose -O ~/Desktop/big_103.jpg http://171.67.245.103/big.jpg 
done
