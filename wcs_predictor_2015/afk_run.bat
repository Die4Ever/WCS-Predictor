echo "begin afk wcs predictor 2015"
echo off
:loop
echo "5 hours left"
timeout /t 3600 /nobreak
echo "4 hours left"
timeout /t 3600 /nobreak
echo "3 hours left"
timeout /t 3600 /nobreak
echo "2 hours left"
timeout /t 3600 /nobreak
echo "1 hour left"
echo off
timeout /t 3600 /nobreak
set "test="
tasklist | find /c "wcs_predictor_2015" > temp.txt
set /p test= <temp.txt
echo %test%
if [%test%] == [0] (
	goto start
) ELSE (
	echo "waiting an extra hour for currently running WCS Predictor"
	timeout /t 3600 /nobreak
)
GOTO loop

:start
..\x64\Release\wcs_predictor_2015.exe afkmode
GOTO loop