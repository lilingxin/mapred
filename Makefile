map:main.o buf.o os.o thread.o
	gcc -g -O0 -o mapred *.o -pthread -levent

%.o:%.c
	gcc -g -O0 -std=gnu99 -c $< -o $@ 

.PHONY:clean
clean:
	rm -rf mapred
	rm -rf *.o
