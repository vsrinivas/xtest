csan: csan.c main.c
	$(CC) -g -DTEST2 -ffreestanding -fsanitize=thread -nostdlib -nostdinc -c -o csan.o csan.c
	$(CC) -g -c -o main.o main.c
	$(CC) -g -pthread -o csan csan.o main.o

tsan: tsan.c main.c
	$(CC) -g -DTEST1 -ffreestanding -fsanitize=thread -nostdlib -nostdinc -c -o tsan.o tsan.c
	$(CC) -g -c -o main.o main.c
	$(CC) -g -pthread -o tsan tsan.o main.o

.PHONY: check
check:

.PHONY: clean
clean:
	rm -f tsan csan csan.o tsan.o main.o
