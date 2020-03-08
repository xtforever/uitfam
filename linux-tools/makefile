CC=gcc
ALL: drop-dtr xcom
xcom: safe_write.o

install:
	install xcom drop-dtr $(HOME)/.local/bin

clean:
	-${RM} xcom drop-dtr *.o
 
