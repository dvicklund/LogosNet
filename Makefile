make:
	gcc -o prog3_server prog3_server.c 
	gcc -o prog3_observer prog3_observer.c
	gcc -o prog3_participant prog3_participant.c

clean:
	rm -f prog3_participant prog3_observer prog3_server
