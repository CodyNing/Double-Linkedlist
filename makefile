all:
	gcc -Werror -Wall -g -o main *.c *.h

clean:
	rm main