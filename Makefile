map:mapred.o buffer.o mapred_files.o 
	gcc -g -O0 -o mapred *.o

%.o:%.c
	gcc -g -O0 -std=gnu99 -c $< -o $@ 
%.o:./os/%.c	
	gcc -g -O0 -std=gnu99 -c $< -o $@

.PHONY:clean
clean:
	@rm -rf mapred
	@rm -rf *.o
