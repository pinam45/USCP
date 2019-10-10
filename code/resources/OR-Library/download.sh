#!/bin/sh

orlibrary_base_url="http://people.brunel.ac.uk/~mastjjb/jeb/orlib/files/"

files_txt="
scp41
scp42
scp43
scp44
scp45
scp46
scp47
scp48
scp49
scp410
scp51
scp52
scp53
scp54
scp55
scp56
scp57
scp58
scp59
scp510
scp61
scp62
scp63
scp64
scp65
scpa1
scpa2
scpa3
scpa4
scpa5
scpb1
scpb2
scpb3
scpb4
scpb5
scpc1
scpc2
scpc3
scpc4
scpc5
scpd1
scpd2
scpd3
scpd4
scpd5
scpe1
scpe2
scpe3
scpe4
scpe5
scpnre1
scpnre2
scpnre3
scpnre4
scpnre5
scpnrf1
scpnrf2
scpnrf3
scpnrf4
scpnrf5
scpnrg1
scpnrg2
scpnrg3
scpnrg4
scpnrg5
scpnrh1
scpnrh2
scpnrh3
scpnrh4
scpnrh5
scpclr10
scpclr11
scpclr12
scpclr13
scpcyc06
scpcyc07
scpcyc08
scpcyc09
scpcyc10
scpcyc11
rail507
rail516
rail582
"

files_gz="
rail2536
rail2586
rail4284
rail4872
"

for file in ${files_txt}
do
	file_txt="${file}.txt"
	if ! [ -f ${file_txt} ]; then
		# download
		wget "${orlibrary_base_url}${file_txt}"
	else
		echo "${file_txt}"
	fi
done

for file in ${files_gz}
do
	file_txt="${file}.txt"
	if ! [ -f ${file_txt} ]; then
		file_gz="${file}.gz"
		# download
		wget "${orlibrary_base_url}${file_gz}"
		# extract
		gzip -d "${file_gz}"
		# rename
		mv "${file}" "${file_txt}"
	else
		echo "${file_txt}"
	fi
done
