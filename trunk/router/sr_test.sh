#!/bin/sh
outlog=~/Desktop/`/bin/date +%Y%m%d`
echo saving output logs to $outlog'.*'

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
