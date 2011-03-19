#!/bin/bash
ipbase=171.67.245
/usr/bin/killall sr
make clean
make

echo starting sr
./sr_start.sh
/bin/sleep 5
echo double ping ARP test
/bin/ping -c 3 171.67.245.103
/bin/ping -c 3 171.67.245.103

for ip in 96 100 101 102 103
do
	/bin/ping -c 5 $ipbase.$ip 
done

for ip in 101 103  
do 
	/usr/bin/tracepath -n $ipbase.$ip
	/usr/bin/firefox http://$ipbase.$ip/big.jpg &
done

