CC=gcc
TARGET=checkapp
$(TARGET).exe:$(TARGET).c  $(TARGET).h
	$(CC) -o $(TARGET).exe $(TARGET).c -I. -L. sqlite3.dll sds.dll cset.dll cygwin1.dll -g -mwindows
sds:
	$(CC) -o sds.dll -fpic -shared sds.c
cset:
	$(CC) -o cset.dll -fpic -shared charsetchange.c -I.
cdll:$(TARGET).c  $(TARGET).h
	$(CC) -o $(TARGET).dll $(TARGET).c -fpic -shared -I. -L. sqlite3.dll sds.dll cset.dll -g -mwindows
clean:
	rm $(TARGET).exe