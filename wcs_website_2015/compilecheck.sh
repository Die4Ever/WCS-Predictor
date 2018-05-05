#!/bin/bash
for f in ./*.h ; do
	[ "$f" -nt lastcompile.txt ] && recompile="1" && echo "$f updated"
	done
for f in ./*.cpp ; do
	[ "$f" -nt lastcompile.txt ] && recompile="1" && echo "$f updated"
	done
[ "$recompile" != 1 ] && exit
sudo -u fourever scl enable devtoolset-1.1 './compile.sh'

echo 'making sure services are started'
service mysql start
service httpd start
sleep 3
echo 'services started!'
echo ''
echo ''
echo ''
echo ''
