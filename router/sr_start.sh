#!/bin/sh
pid=`/bin/pidof sr`
if [ $pid > 0 ] 
then
	echo "sr already running on $pid"
	return;
fi
outlog=~/Desktop/`/bin/date +%Y%m%d`
echo logging to $outlog.*
./sr -l $outlog.log > $outlog.txt 2>&1 &

echo sr process id `/bin/pidof sr`
