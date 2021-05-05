@ECHO OFF
DEL link.txt > NUL
:LOOP
FOR /F "tokens=1*" %%a IN ("%*") DO (
  ECHO %%a>> link.txt
  IF NOT x%%b==x CALL :LOOP %%b
)