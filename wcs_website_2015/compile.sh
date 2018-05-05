date > lastcompile.txt
date
echo 'Compiling WCS Predictor 2015 website'
sleep 3
python ~/Dropbox/dropbox.py stop
which g++
g++ --version
time g++ main.cpp -D _DEBUG -O3 --param ggc-min-expand=12 --param ggc-min-heapsize=200 -std=c++11 -I ~/Dropbox/MyHeaders/ -lpthread -lmysqlclient -lcrypto -Wall -o index.exe
#time g++ main.cpp -D _DEBUG -g --param ggc-min-expand=12 --param ggc-min-heapsize=200 -std=c++11 -I ~/Dropbox/MyHeaders/ -lpthread -lmysqlclient -lcrypto -Wall -o index.exe
cp ../../wcs_2014_predictor/public_html/index.exe ../../wcs_2014_predictor/public_html/index2015.old.exe
rm ../../wcs_2014_predictor/public_html/index.exe
cp index.exe ../../wcs_2014_predictor/public_html/index.exe
chmod 755 ../../wcs_2014_predictor/public_html/index.exe
ls -la ../../wcs_2014_predictor/public_html/index.exe
date
python ~/Dropbox/dropbox.py start
sleep 10