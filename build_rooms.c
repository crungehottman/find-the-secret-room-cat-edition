/* Author: Caleigh Runge-Hottman
 * CS 344 Program 2
 *
 * Filename: rungehoc.buildrooms.c
 * Description: creates a series of files that hold descriptions of the in-game
 *              rooms  and how the rooms are connected
 *
 * To run: 
 *         $ gcc -o rungehoc.buildrooms rungehoc.buildrooms.c
 *         $ rungehoc.buildrooms
 */

#include <string.h>
#include <stdio.h> // need for file manipulation
#include <stdlib.h> // need to create randomness and file stuff
#include <sys/types.h> // need for pid
#include <unistd.h> // need for pid
#include <sys/stat.h> // do i still need this???

int main() {
   ////////////////////////////////////////////////////////////////////////////
   //                          create directory
   ////////////////////////////////////////////////////////////////////////////
   // build up the rooms directory name string
   // this is used as the dir to store the room files
   // code adopted from Prof. Brewster's Piazza post @178
   ////////////////////////////////////////////////////////////////////////////
   pid_t myPID = getpid();
   const char* myONID = "rungehoc";
   char myRoomsDirName[64];
   memset(myRoomsDirName, '\0', sizeof(myRoomsDirName));
   sprintf(myRoomsDirName, "%s.rooms.%d", myONID, myPID);

   // create directory called rungehoc.rooms.<PROCESSID> 
   int result = mkdir(myRoomsDirName, 0755); 

   ////////////////////////////////////////////////////////////////////////////
   //                           room variables
   ////////////////////////////////////////////////////////////////////////////
   // Each room has...
   //   - a random room name
   //   - a random room type
   //   - at least 3 outgoing connections
   //   - at most 6 outgoing connections 
   //   - the # of outgoing connections is random!
   ////////////////////////////////////////////////////////////////////////////
   time_t t; // for srand
   srand((unsigned) time(&t)); // random seed
   int rand_num = 0; // the rand # we use to create rooms, names, and type
   int rand_pairing = 0; // the index of the random pairing for connections

   char file_name[50]; // stores the file name 
   FILE *fileptr; // file pointer

   int i = 0; // for loops and such
   int j = 0; // for loops and such
   int total = 0; // for total num connections
   int num_connections = 0;
   int rooms_created = 0; // holds # rooms created so far
   int connections[7][7]; // holds each rooms connections
   char *room_list[7]; // holds the 7 random room names for this round
   int room_type[3] = {0, 1, 2}; // 0 = START_ROOM, 1 = END_ROOM, 2 = MID_ROOM
   int available_rooms[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
   // create room names based off of my cat's favorite areas
   char *room_names[10] = {
      "sink",
      "hallway",
      "litterbox",
      "couch",
      "shelf",
      "foodbowl",
      "bathtub",
      "countertop",
      "pizzabox",
      "windowsill"
   };

   // init all connections to 0
   for (i = 0; i < 7; i++) {
      for (j = 0; j < 7; j++) {
         connections[i][j] = 0;
      }
   }

   // clear the room list
   for (i = 0; i < 7; i++) {
      room_list[i] = malloc(50 * sizeof(char));
      memset(room_list[i], '\0', sizeof(room_list[i]));
   }
 
   ////////////////////////////////////////////////////////////////////////////
   //                     create 7 rooms
   // creates the room files, chooses room name, and appends ROOM NAME
   ////////////////////////////////////////////////////////////////////////////

   // loop until we have created 7 rooms
   while (rooms_created < 7) {
      // Choose a random room number between 0-10 (we have 10 available rooms)
      rand_num = rand() % 10;
      // reset/memset filename to avoid errors
      memset(file_name, '\0', sizeof(file_name));

      // Rooms cannot be used twice!
      // available rooms holds 1-10, as a room ID. if the room is taken, it
      // becomes a 0. if it is available, index that room name.
      while (available_rooms[rand_num] == 1) {
         // build file name to write to
         sprintf(file_name, "%s/%s", myRoomsDirName, room_names[rand_num]);
         fileptr = fopen(file_name, "a");
         fprintf(fileptr, "ROOM NAME: %s\n", room_names[rand_num]);
         fclose(fileptr);
         // add the room name to the room_list and update rooms_created
         room_list[rooms_created] = room_names[rand_num];
         rooms_created++;
         // that room is now unavailable
         available_rooms[rand_num] = 0;
      }
   }


   ////////////////////////////////////////////////////////////////////////////
   //                     Create room connections
   //
   // The connections from one room to the others should be randomly assigned
   // (i.e., which rooms connect to each other is random)
   //
   // Note: I created this section with a LOT of help from the piazza post:
   //
   // "To build up the graph from an empty state, continuously add connections 
   // (both ways) between a randomly selected pair of rooms to the graph 
   // (checking beforehand that both connections can be added). Each time you 
   // add a pair of connections, check the constraints of the graph: Does each 
   // room have at least 3 out-bound connections? If so, yer done, graph built!
   // If not, add another pair!" - Professor Brewster @186 on Piazza
   //  each room gets its connections one room at a time. 
   //
   ////////////////////////////////////////////////////////////////////////////

   // loop through each room
   for (i = 0; i < 7; i++) {
      // get a random number of connections between 3 and 6
      rand_num = (rand() % 4) + 3;  

      // reset total because we're looking in a different room
      num_connections = 0; 
      // find the total number of connections so far
      for (j = 0; j < 7; j++) {
         num_connections += connections[i][j];
      }
      // randomly select a pair for this room to create a connection with
      while ( num_connections < rand_num ) {
         rand_pairing = rand() % 7;
         // a room cannot pair with itself or a room it has already paired with!
         while (rand_pairing == i || connections[i][rand_pairing] == 1) {
            rand_pairing = rand() % 7;
         }
         // if the connection hasn't been made,  mark the connection as true 
         // note: because the graph is undirected, a connection from 
         // a -> b is also a connection from b -> a
         connections[i][rand_pairing] = 1;
         connections[rand_pairing][i] = 1;
         // the total number of connections has increased
         num_connections++;
      }
   }

   // loop through each room
   for (i = 0; i < 7; i++) {
      // write the connections
      // reset num connections because we're in a new room
      num_connections = 1;
      for (j = 0; j < 7; j++) {
         if (connections[i][j] == 1) {
            // change the file name
            // reset/memset filename to avoid errors
            memset(file_name, '\0', sizeof(file_name));
            // build up the file name (file names will be names of rooms)
            sprintf(file_name, "%s/%s", myRoomsDirName, room_list[i]);
            FILE *fileptr;
            fileptr = fopen(file_name, "a");
            fprintf(fileptr, "CONNECTION %d: %s\n", num_connections, room_list[j]);
            fclose(fileptr);
            num_connections++; // store the number of connections
         }
      }

      // write type of room
      sprintf(file_name, "%s/%s", myRoomsDirName, room_list[i]);
      fileptr = fopen(file_name, "a");

      // randomly assign the room types!
      if (i == 0) { // the first index will be the start room
         fprintf(fileptr, "ROOM TYPE: START_ROOM\n");
      }
      else if (i == 6) { // the final index will be the end room
         fprintf(fileptr, "ROOM TYPE: END_ROOM\n");
      }
      else { // all others will be the mid rooms
         fprintf(fileptr, "ROOM TYPE: MID_ROOM\n");
      }
      fclose(fileptr);
   }

   return 0;
}
