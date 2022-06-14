build:
	gcc -Ilib/ main.c server.c lib/thpool.c -o server.out
#-std=c99 -Wall -g 
