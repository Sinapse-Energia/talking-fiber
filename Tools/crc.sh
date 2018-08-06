cp ./Debug/EC-M2M.binary ./Debug/EC-M2M-CRC.binary
CRC=$(crc32 ./Debug/EC-M2M-CRC.binary)
echo CRC32 is $CRC
perl -e 'print pack "H*", "'$CRC'"' >> ./Debug/EC-M2M-CRC.binary