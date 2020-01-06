#!/bin/sh

sts_base_url="http://www.co.mi.i.nagoya-u.ac.jp/~yagiura/scp/stcp/"

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
data.1215
data.2187
"

for file in ${files}
do
	if ! [ -f ${file} ]; then
		file_gz="${file}.gz"
		# download
		wget "${sts_base_url}${file_gz}"
		# extract
		gzip -d "${file_gz}"
	else
		echo "${file}"
	fi
done
