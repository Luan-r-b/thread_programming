gcc -o bank_program main.c bank.c client.c server.c worker.c request_queue.c -pthread
./bank_program 10 20 20 100 50 5
