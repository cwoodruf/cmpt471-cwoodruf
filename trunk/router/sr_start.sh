#!/bin/sh
outlog=~/Desktop/`/bin/date +%Y%m%d`
echo logging to $outlog'.*'
./sr -l $outlog.log > $outlog.txt 2>&1 &
