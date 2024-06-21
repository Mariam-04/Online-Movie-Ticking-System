//CLENT
#include <fcntl.h>
#include <iostream>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

int c_main() {
  // Open client semaphore
  sem_t *clientSemaphore = sem_open("/client_semaphore", O_CREAT, 0644, 1);
  if (clientSemaphore == SEM_FAILED) {
    cerr << "Error: Semaphore initialization failed.\n";
    exit(EXIT_FAILURE);
  }

  // Wait for server semaphore
  sem_wait(clientSemaphore);

  // Get user input for movie number, number of seats, and customer number
  int movieNumber, numSeats, customerNumber;
  cout << "Enter the movie number, number of seats, and customer number "
          "(space-separated): ";
  cin >> movieNumber >> numSeats >> customerNumber;

  // Post server semaphore
  sem_post(clientSemaphore);

  // Close client semaphore
  sem_close(clientSemaphore);
  sem_unlink("/client_semaphore");

  return 0;
}





