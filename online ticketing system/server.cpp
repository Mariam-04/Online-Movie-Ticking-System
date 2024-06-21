///SERVER
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <semaphore.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

#define MOVIE_INFO_FILE "movieInfo.txt"
#define BOOKING_INFO_FILE "BookingInfo.txt"

// Structure for movie information
struct Movie {
  int number;
  string name;
  int availableSeats;
};

// Function to read movie information from file
vector<Movie> readMoviesInfo() {
  vector<Movie> movies;
  ifstream file(MOVIE_INFO_FILE);
  if (!file) {
    cerr << "Error: Unable to open " << MOVIE_INFO_FILE << endl;
    exit(EXIT_FAILURE);
  }

  Movie movie;
  while (file >> movie.number >> movie.name >> movie.availableSeats) {
    movies.push_back(movie);
  }
  file.close();
  return movies;
}

// Function to update movie information in file
void updateMoviesInfo(const vector<Movie> &movies) {
  ofstream file(MOVIE_INFO_FILE);
  if (!file) {
    cerr << "Error: Unable to open " << MOVIE_INFO_FILE << endl;
    exit(EXIT_FAILURE);
  }

  for (const Movie &movie : movies) {
    file << movie.number << " " << movie.name << " " << movie.availableSeats
         << endl;
  }
  file.close();
}

// Function to book tickets
void bookTickets(int movieNumber, int numSeats, int customerNumber,
                 sem_t *serverSemaphore) {
  // Open client semaphore
  sem_t *clientSemaphore = sem_open("/client_semaphore", O_CREAT, 0644, 1);
  if (clientSemaphore == SEM_FAILED) {
    cerr << "Error: Semaphore initialization failed.\n";
    exit(EXIT_FAILURE);
  }

  // Wait for server semaphore
  sem_wait(serverSemaphore);

  // Read movie info
  vector<Movie> movies = readMoviesInfo();

  // Find the selected movie
  Movie *selectedMovie = nullptr;
  for (Movie &movie : movies) {
    if (movie.number == movieNumber) {
      selectedMovie = &movie;
      break;
    }
  }

  // Check if movie found
  if (selectedMovie) {
    // Check if seats available
    if (selectedMovie->availableSeats >= numSeats) {
      // Update available seats
      selectedMovie->availableSeats -= numSeats;
      updateMoviesInfo(movies);

      // Save booking info
      ofstream bookingFile(BOOKING_INFO_FILE, ios::app);
      if (!bookingFile) {
        cerr << "Error: Unable to open " << BOOKING_INFO_FILE << endl;
        exit(EXIT_FAILURE);
      }
      bookingFile << customerNumber << " " << movieNumber << " " << numSeats
                  << endl;
      bookingFile.close();

      cout << "Tickets booked successfully!" << endl;
    } else {
      cout << "Sorry, insufficient seats available for the selected movie."
           << endl;
    }
  } else {
    cout << "Invalid movie number." << endl;
  }

  // Post server semaphore
  sem_post(serverSemaphore);

  // Close client semaphore
  sem_close(clientSemaphore);
  sem_unlink("/client_semaphore");
}

// Function to display available movies
void displayMovies(const vector<Movie> &movies) {
  cout << "Available Movies:" << endl;
  for (const Movie &movie : movies) {
    cout << "Movie Number: " << movie.number << ", Name: " << movie.name
         << ", Available Seats: " << movie.availableSeats << endl;
  }
}

int main() {
  // Open server semaphore
  sem_t *serverSemaphore = sem_open("/server_semaphore", O_CREAT, 0644, 1);
  if (serverSemaphore == SEM_FAILED) {
    cerr << "Error: Semaphore initialization failed.\n";
    exit(EXIT_FAILURE);
  }

  // Display available movies
  vector<Movie> movies = readMoviesInfo();
  displayMovies(movies);

  // Get user input
  int movieNumber, numSeats, customerNumber;
  cout << "Enter the movie number, number of seats, and customer number "
          "(space-separated): ";
  cin >> movieNumber >> numSeats >> customerNumber;

  // Book tickets
  bookTickets(movieNumber, numSeats, customerNumber, serverSemaphore);

  // Close server semaphore
  sem_close(serverSemaphore);
  sem_unlink("/server_semaphore");

  return 0;
}