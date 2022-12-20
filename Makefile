build: archiver.c
	gcc archiver.c -lm -o archiver

clean:
	rm -f archiver
