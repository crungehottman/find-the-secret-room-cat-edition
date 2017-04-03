/* Author: Caleigh Runge-Hottman
 */

 
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>

void *getcurrtime() {
   // store file info for currentTime.txt
   FILE *fp;
   char *time_file_name = malloc(10 * sizeof(char));

   // stuff for time thread
   time_t rawtime;
   struct tm * timeinfo;
   char time_str[100];
   time (&rawtime);
   timeinfo = localtime (&rawtime);
   // format the time into hour, minute, am/pm, weekday, month, date, year
   strftime (time_str, 100, "%I:%M%p, %A, %B %d, %Y", timeinfo);
   printf("\n%s\n\n", time_str);

   // open and write the formatted time string to file
   fp = fopen("currentTime.txt", "w+");
   fprintf(fp, "%s\n", time_str);
   fclose(fp); // close file
   return NULL;
}



int main() {

   // for loops and such
   int i = 0;
   int j = 0;

   // some constants to avoid string errors
   int FILE_NAME_SIZE = 255;
   int PATH_SIZE = 100;

   //////////////////////////////////////////////////////////////////
   // variables for storing room info
   //////////////////////////////////////////////////////////////////

   // holds the names of the 7 rooms
   char *room_names[7];

   // init the room names array
   for (i = 0; i < 7; i++) {
      room_names[i] = malloc(sizeof(char) * FILE_NAME_SIZE);
      memset(room_names[i], '\0', sizeof(room_names[i]));
   }

   // init connections matrix (holds connections between the rooms)
   char *connections[7][7];
   for (i = 0; i < 7; i++) {
      for (j = 0; j < 7; j++) {
         connections[i][j] = malloc(sizeof(char) * FILE_NAME_SIZE);
         memset(connections[i][j], '\0', sizeof(connections[i][j]));
      }
   }

   // holds the type of each room. 0: starting room, 1: mid, 3: end
   int room_types[7];

   // holds the start and end room indexes
   int start_room_idx;
   int end_room_idx;

   // holds the number of connections for each room
   int num_connects[7];

   // define file pointer, file name and dir name and path name
   FILE *fileptr;
   char *file_name = malloc(FILE_NAME_SIZE * sizeof(char));
   char *most_recent_dir = malloc(FILE_NAME_SIZE * sizeof(char));
   char *path = malloc(PATH_SIZE * sizeof(char));

   // memset most recent dir and filename
   memset(most_recent_dir, '\0', sizeof(most_recent_dir));
   memset(file_name, '\0', sizeof(file_name));
   memset(path, '\0', sizeof(path));

   // used to determine when user inputs time
   char timetitle[5] = " time";

   
   ///////////////////////////////////////////////////////////////////////
   // find most recent directory using STAT() so we know which directory
   // to search for files.
   ///////////////////////////////////////////////////////////////////////

   // get our current directory
   char *fd = "rungehoc.rooms."; // start of the directory name
   char currentDir[100]; // char arr to hold current directory
   memset(currentDir, '\0', sizeof(currentDir)); // clear 
   getcwd(currentDir, sizeof(currentDir)); // get current working directory 
//   printf("%s\n", currentDir);

   // create a directory entry object to get info about directories
   DIR *d;
   struct dirent *dp = malloc(sizeof(struct dirent));
   // create stat object to find most recent dir
   struct stat *buffer = malloc(sizeof(struct stat));
   time_t lastModified; // the files modification time
   d = opendir(currentDir); // open our directory to search for subdirs
   int greatest_time = 0; // holds greatest unix time
   if (d != NULL) {
      // read the contents of the cwd
      while (dp= readdir(d)) {
         // if rungehoc.rooms isn't in file, we don't care about its info
         if (strstr(dp->d_name,fd) != NULL){
            stat(dp->d_name, buffer); // get info
            lastModified = buffer->st_mtime; // in unix time!!!!!!!!!
            if (lastModified > greatest_time) {
               // unix time uses one long number representing sec from 1970...
               // the greater the number, the more recently it was modified
               greatest_time = lastModified;
               memset(most_recent_dir, '\0', sizeof(most_recent_dir));
               strcpy(most_recent_dir, dp->d_name); // store most recent dir
            }
         }
      }
   }
   closedir(d);


   ///////////////////////////////////////////////////////////////////
   // read files into variables
   ///////////////////////////////////////////////////////////////////

   // create a directory entry obj to get info about our most_recent_dir
   DIR *dir;
   struct dirent *ent;
   // open our most recent dir
   dir = opendir(most_recent_dir);
   if (dir) {
      // * append all the files and directories within directory
      i = 0; // keep track for array indexing
      while ((ent = readdir (dir)) != NULL) {
         if (ent->d_type == DT_REG) { // (ignores parent directory files like '.' and '..')
            // store each of the filenames in our most recent dir because
            // the filenames are our room names
            strncpy(room_names[i], ent->d_name, 254);
            (room_names[i])[254] = '\0';
            i++;
         }
      }
      closedir(dir);
   }
 

   /////////////////////////////////////////////////////////////
   // create path name in order to access file info
   ////////////////////////////////////////////////////////////
   for (i = 0; i < 7; i++) {
      memset(path, '\0', sizeof(path));
      strcat(path, "./");
      strcat(path, most_recent_dir);
      strcat(path, "/");
      // append room name because the file names are the room names
      strcat(path, room_names[i]);

      ////////////////////////////////////////////////////
      //  try opening file
      ////////////////////////////////////////////////////////
      FILE *room_file = fopen(path, "r");
      if (room_file == 0) {
         perror("Cannot open file");
      }
      else {
         ////////////////////////////////////////////////////////////
         // a bunch of variables for use with strtok() and strcmp
         ////////////////////////////////////////////////////////////
         char line [128];  // to store line
         memset(line, '\0', sizeof(line));
         char templine [128]; // to hold copy of line for strtok
         char *token;
         int ret; // holds result
         // for result matching purposes
         char room_type_str[10] = "ROOM TYPE";
         char room_name_str[10] = "ROOM NAME";
         char room_mid[10] = " MID_ROOM";
         char room_end[10] = " END_ROOM";
       
         // holds # of current room's connections 
         // reset it because it's a new room
         int num_connections = 0;

         while (fgets(line, sizeof line, room_file) != NULL) {
            memset(templine, '\0', sizeof(line));
//            fputs(line, stdout); // write the line
            strcpy(templine, line); // copy the line for strtok
            token = strtok(templine, ":");

            // check whether the token is the room name
            ret = strcmp(room_name_str, token);

            if (ret != 0) { // it's not the room name...
               // check whether it's the room type
               ret = strcmp(room_type_str, token);
               if (ret != 0) { 
                  // it's not the room type or room name...
                  // it must be a connection!
                  token = strtok(NULL, "\n"); // get the name of the connection

                  // append the name to the connections array
                  strcpy(connections[i][num_connections], token);
                  num_connections++; // we just added a new connection
               }

               else {  // it's the room type!
                  // get the room type
                  token = strtok(NULL, "\n");
                  ret = strcmp(room_mid, token);
                  // check whether it's a mid room
                  if (ret == 0) { // it's a mid room
                     room_types[i] = 1; // mid room key: 1;
                 }
                  else { // either an end or start room
                     ret = strcmp(room_end, token); // check whether it's end
                     if (ret == 0) { // it's the end room!
                        room_types[i] = 2; // end room key: 1
                        end_room_idx = i;
                     }
                     else { // it's the start room!
                        room_types[i] = 1; // start room key: 0
                        start_room_idx = i;
                     }
                  }
               }
            }
            else {
               // it is the room name. we already have this, so do nothing.
               continue;
            }
         }
         // done with that room_file, store its total number of connections
         num_connects[i] = num_connections; 
      } 
      fclose(room_file);
   }


   //////////////////////////////////////////////////////////////////////
   //    Play Game
   //////////////////////////////////////////////////////////////////////
 //  pthread_t mainthread;
//   pthread_t time_thread;
//   pthread_create(&time_thread, NULL, getcurrtime, NULL);

   // for reporting the number of steps
   int num_steps = 1; // the starting room counts as 1 step
   char *steps[100];
   for (i = 0; i < 100; i++) {
      steps[i] = malloc(50 * sizeof(char));
      memset(steps[i], '\0', sizeof(steps[i]));
   }

   // start with first room
   int curr_room = start_room_idx;
   steps[0] = room_names[curr_room];

   ////////////////////////////////////////////////////////////////////////////
   //  get user input
   ////////////////////////////////////////////////////////////////////////////

   // stores user input
   char *input = malloc(50 * sizeof(char));
   memset(input, '\0', sizeof(input));

   while(curr_room != end_room_idx) {
      int valid_input = 0; // check whether input was valid

      // print current location and its conneting rooms
      printf("CURRENT LOCATION: %s\n", room_names[curr_room]);
      // print possible connections
      printf("POSSIBLE  CONNECTIONS: ");
      for (i = 0; i < num_connects[curr_room]; i++) {
         if (i == 0) { // if first connection
            printf("%s,", connections[curr_room][i]);
         }
         else if (i == num_connects[curr_room] - 1) { // if last connection
            printf("%s.\n", connections[curr_room][i]);
         }
         else { // if any other connection in between
            printf(" %s, ", connections[curr_room][i]);
         }
      }

      // ask the user where to go & wait for input
      printf("WHERE TO? >");
      fgets(input, 25, stdin);

      // remove new line char from input
      char *pos;
      if ((pos=strchr(input, '\n')) != NULL) {
         *pos = '\0';
      }

      // fix a spacing issue with input...
      char* input2 = malloc(50 * sizeof(char));
      strcpy(input2, input);
      sprintf(input2, " %s", input);

      // check whether input is valid
      for (i = 0; i < num_connects[curr_room]; i++) {
         if (strcmp(input2, connections[curr_room][i]) == 0) {
            // the input was found in this room's connections
            valid_input = 1;
            // add this step to our list of steps and increment # of steps
            steps[num_steps] = connections[curr_room][i];
            num_steps++;

            // find the true index of the input room
            for (j = 0; j < 7; j++) {
               if (strcmp(input, room_names[j]) == 0) {
                  // get new index for the current room
                  curr_room = j;
                  continue;
               }
            }
         } // check whether user chose to get the current time
         else if (strcmp(input2, timetitle) == 0) {
            getcurrtime(); // get current time!
            valid_input = 1;
            break;
         }
      }
      if (valid_input == 0) { // invalid input
         printf("\n\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");  
      }
   }
   // if you've reached here, you've won!
   printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
   printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", num_steps);
   // print the individual steps (note: including the first step)
   for (i = 0; i < num_steps; i++) {
      printf("%s\n", steps[i]);
   }

//   pthread_detach(time_thread);
//   pthread_cancel(time_thread);
   return 0;
}
