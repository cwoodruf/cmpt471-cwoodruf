#!/bin/bash
/usr/bin/killall sr
srpid=`/bin/pidof sr`
if [ "$srpid" == "" ] 
then
	echo starting sr
	./sr_start.sh
	/bin/sleep 5
	echo double ping ARP test
	/bin/ping -c 3 171.67.245.103
	/bin/ping -c 3 171.67.245.103
fi

echo starting background pings \(stress test\)
/bin/ping -q 171.67.245.96 &
/bin/ping -q 171.67.245.100 &
/bin/ping -q 171.67.245.101 &
/bin/ping -q 171.67.245.102 &
/bin/ping -q 171.67.245.103 &

echo tracepaths
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
