#!/bin/sh

sts_base_url="http://mauricio.resende.info/data/"
sts_achive="steiner-triple-covering.tar.gz"

files="
data.9
data.15
data.27
data.45
data.81
data.135
data.243
data.405
data.729
"

for file in $files
do
	if ! [ -f $file ]; then
		# download archive
		wget "$sts_base_url$sts_achive"
		# extract
		tar -x -z -f $sts_achive --strip-components 1
		# remove archive
		rm $sts_achive
	else
		echo "$file"
	fi
done
