mkdir goodies/ 
cd goodies/


Q3BASE_DIR=../build/release-linux-x86_64/baseq3

wget https://www.moddb.com/downloads/mirror/121410/102/0b34606b2283bffc7f31ab0d767730c0 
wget http://ioquake3.org/files/xcsv_hires.zip
unzip 0b34606b2283bffc7f31ab0d767730c0 
unzip xcsv_hires.zip
cp baseq3/pak9hqq36.pk3 $Q3BASE_DIR
cp xcsv_bq3hi-res.pk3 $Q3BASE_DIR 
