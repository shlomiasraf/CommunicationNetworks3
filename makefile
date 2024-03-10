CC= gcc
FLAGS= -Wall -g 

all: TCP_Sender TCP_Receiver
	
Sender.o: Sender.c
	$(CC) $(FLAGS) -c Sender.c

Receiver.o: Receiver.c
	$(CC) $(FLAGS) -c Receiver.c
TCP_Sender: Sender.o
	$(CC) $(FLAGS) -o TCP_Sender Sender.o

TCP_Receiver: Receiver.o
	$(CC) $(FLAGS) -o TCP_Receiver Receiver.o

clean:
	rm -f *.o TCP_Sender TCP_Receiver