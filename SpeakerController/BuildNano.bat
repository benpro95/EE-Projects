SET ARDUINO_CLI=C:\Program Files\Arduino\arduino-cli.exe
SET ARDUINO_LIB=Z:\Projects\libraries

SET SKETCH_PATH=.
SET SKETCH_NAME=SpeakerController.ino

rmdir /s /q "%LocalAppData%\Arduino15\build"
mkdir "%LocalAppData%\Arduino15\build"
"%ARDUINO_CLI%" -v --libraries %ARDUINO_LIB% compile --fqbn arduino:avr:nano:cpu=atmega328old %SKETCH_PATH%\%SKETCH_NAME% --build-path "%LocalAppData%\Arduino15\build"
"%ARDUINO_CLI%" -v upload -p COM6 --fqbn arduino:avr:nano:cpu=atmega328old --input-dir "%LocalAppData%\Arduino15\build"
pause
exit