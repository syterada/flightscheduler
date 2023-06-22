/**
 * Assignment #3: Strings, structs, pointers, command-line arguments.
 *  Let's use our knowledge to write a simple flight management system!
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

// Limit constants
#define MAX_CITY_NAME_LEN 20
#define MAX_FLIGHTS_PER_CITY 5
#define MAX_DEFAULT_SCHEDULES 50

// Time definitions
#define TIME_MIN 0
#define TIME_MAX ((60 * 24) - 1)
#define TIME_NULL -1

/******************************************************************************
 * Structure and Type definitions                                             *
 ******************************************************************************/
typedef int time_t; // integers used for time values
typedef char city_t[MAX_CITY_NAME_LEN + 1];
; // null terminate fixed length city

// Structure to hold all the information for a single flight
//   A city's schedule has an array of these
struct flight
{
  int time;      // departure time of the flight
  int available; // number of seats currently available on the flight
  int capacity;  // maximum seat capacity of the flight
};

// Structure for an individual flight schedule
// The main data structure of the program is an Array of these structures
// Each structure will be placed on one of two linked lists:
//                free or active
// Initially the active list will be empty and all the schedules
// will be on the free list.  Adding a schedule is finding the first
// free schedule on the free list, removing it from the free list,
// setting its destination city and putting it on the active list
struct flight_schedule
{
  city_t destination;                          // destination city name
  struct flight flights[MAX_FLIGHTS_PER_CITY]; // array of flights to the city
  struct flight_schedule *next;                // link list next pointer
  struct flight_schedule *prev;                // link list prev pointer
};

/******************************************************************************
 * Global / External variables                                                *
 ******************************************************************************/
// This program uses two global linked lists of Schedules.  See comments
// of struct flight_schedule above for details
struct flight_schedule *flight_schedules_free = NULL;
struct flight_schedule *flight_schedules_active = NULL;

/******************************************************************************
 * Function Prototypes                                                        *
 ******************************************************************************/
// Misc utility io functions
int city_read(city_t city);
bool time_get(time_t *time_ptr);
bool flight_capacity_get(int *capacity_ptr);
void print_command_help(void);

// Core functions of the program
void flight_schedule_initialize(struct flight_schedule array[], int n);
struct flight_schedule *flight_schedule_find(city_t city);
struct flight_schedule *flight_schedule_allocate(void);
void flight_schedule_free(struct flight_schedule *fs);
void flight_schedule_add(city_t city);
void flight_schedule_listAll(void);
void flight_schedule_list(city_t city);
void flight_schedule_add_flight(city_t city);
void flight_schedule_remove_flight(city_t city);
void flight_schedule_schedule_seat(city_t city);
void flight_schedule_unschedule_seat(city_t city);
void flight_schedule_remove(city_t city);

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs);
int flight_compare_time(const void *a, const void *b);

int main(int argc, char *argv[])
{
  long n = MAX_DEFAULT_SCHEDULES;
  char command;
  city_t city;

  if (argc > 1)
  {
    // If the program was passed an argument then try and convert the first
    // argument in the a number that will override the default max number
    // of schedule we will support
    char *end;
    n = strtol(argv[1], &end, 10); // CPAMA p 787
    if (n == 0)
    {
      printf("ERROR: Bad number of default max scedules specified.\n");
      exit(EXIT_FAILURE);
    }
  }

  // C99 lets us allocate an array of a fixed length as a local
  // variable.  Since we are doing this in main and a call to main will be
  // active for the entire time of the program's execution this
  // array will be alive for the entire time -- its memory and values
  // will be stable for the entire program execution.
  struct flight_schedule flight_schedules[n];

  // Initialize our global lists of free and active schedules using
  // the elements of the flight_schedules array
  flight_schedule_initialize(flight_schedules, n);

  // DEFENSIVE PROGRAMMING:  Write code that avoids bad things from happening.
  //  When possible, if we know that some particular thing should have happened
  //  we think of that as an assertion and write code to test them.
  // Use the assert function (CPAMA p749) to be sure the initilization has set
  // the free list to a non-null value and the the active list is a null value.
  assert(flight_schedules_free != NULL && flight_schedules_active == NULL);

  // Print the instruction in the beginning
  print_command_help();

  // Command processing loop
  while (scanf(" %c", &command) == 1)
  {
    switch (command)
    {
    case 'A':
      //  Add an active flight schedule for a new city eg "A Toronto\n"
      city_read(city);
      flight_schedule_add(city);

      break;
    case 'L':
      // List all active flight schedules eg. "L\n"
      flight_schedule_listAll();
      break;
    case 'l':
      // List the flights for a particular city eg. "l\n"
      city_read(city);
      flight_schedule_list(city);
      break;
    case 'a':
      // Adds a flight for a particular city "a Toronto\n
      //                                      360 100\n"
      city_read(city);
      flight_schedule_add_flight(city);
      break;
    case 'r':
      // Remove a flight for a particular city "r Toronto\n
      //                                        360\n"
      city_read(city);
      flight_schedule_remove_flight(city);
      break;
    case 's':
      // schedule a seat on a flight for a particular city "s Toronto\n
      //                                                    300\n"
      city_read(city);
      flight_schedule_schedule_seat(city);
      break;
    case 'u':
      // unschedule a seat on a flight for a particular city "u Toronto\n
      //                                                      360\n"
      city_read(city);
      flight_schedule_unschedule_seat(city);
      break;
    case 'R':
      // remove the schedule for a particular city "R Toronto\n"
      city_read(city);
      flight_schedule_remove(city);
      break;
    case 'h':
      print_command_help();
      break;
    case 'q':
      goto done;
    default:
      printf("Bad command. Use h to see help.\n");
    }
  }
done:
  return EXIT_SUCCESS;
}

/**********************************************************************
 * city_read: Takes in and processes a given city following a command *
 *********************************************************************/
int city_read(city_t city)
{
  int ch, i = 0;

  // skip leading non letter characters
  while (true)
  {
    ch = getchar();
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
    {
      city[i++] = ch;
      break;
    }
  }
  while ((ch = getchar()) != '\n')
  {
    if (i < MAX_CITY_NAME_LEN)
    {
      city[i++] = ch;
    }
  }
  city[i] = '\0';
  return i;
}

/****************************************************************
 * Message functions so that your messages match what we expect *
 ****************************************************************/
void msg_city_bad(char *city)
{
  printf("No schedule for %s\n", city);
}

void msg_city_exists(char *city)
{
  printf("There is a schedule of %s already.\n", city);
}

void msg_schedule_no_free(void)
{
  printf("Sorry no more free schedules.\n");
}

void msg_city_flights(char *city)
{
  printf("The flights for %s are:", city);
}

void msg_flight_info(int time, int avail, int capacity)
{
  printf(" (%d, %d, %d)", time, avail, capacity);
}

void msg_city_max_flights_reached(char *city)
{
  printf("Sorry we cannot add more flights on this city.\n");
}

void msg_flight_bad_time(void)
{
  printf("Sorry there's no flight scheduled on this time.\n");
}

void msg_flight_no_seats(void)
{
  printf("Sorry there's no more seats available!\n");
}

void msg_flight_all_seats_empty(void)
{
  printf("All the seats on this flights are empty!\n");
}

void msg_time_bad()
{
  printf("Invalid time value\n");
}

void msg_capacity_bad()
{
  printf("Invalid capacity value\n");
}

void print_command_help()
{
  printf("Here are the possible commands:\n"
         "A <city name>     - Add an active empty flight schedule for\n"
         "                    <city name>\n"
         "L                 - List cities which have an active schedule\n"
         "l <city name>     - List the flights for <city name>\n"
         "a <city name>\n"
         "<time> <capacity> - Add a flight for <city name> @ <time> time\n"
         "                    with <capacity> seats\n"
         "r <city name>\n"
         "<time>            - Remove a flight form <city name> whose time is\n"
         "                    <time>\n"
         "s <city name>\n"
         "<time>            - Attempt to schedule seat on flight to \n"
         "                    <city name> at <time> or next closest time on\n"
         "                    which their is an available seat\n"
         "u <city name>\n"
         "<time>            - unschedule a seat from flight to <city name>\n"
         "                    at <time>\n"
         "R <city name>     - Remove schedule for <city name>\n"
         "h                 - print this help message\n"
         "q                 - quit\n");
}

/****************************************************************
 * Resets a flight schedule                                     *
 ****************************************************************/
void flight_schedule_reset(struct flight_schedule *fs)
{
  fs->destination[0] = 0;
  for (int i = 0; i < MAX_FLIGHTS_PER_CITY; i++)
  {
    fs->flights[i].time = TIME_NULL;
    fs->flights[i].available = 0;
    fs->flights[i].capacity = 0;
  }
  fs->next = NULL;
  fs->prev = NULL;
}

/******************************************************************
* Initializes the flight_schedule array that will hold any flight *
* schedules created by the user. This is called in main for you.  *
 *****************************************************************/

void flight_schedule_initialize(struct flight_schedule array[], int n)
{
  flight_schedules_active = NULL;
  flight_schedules_free = NULL;

  // takes care of empty array case
  if (n == 0)
    return;

  // Loop through the Array connecting them
  // as a linear doubly linked list
  array[0].prev = NULL;
  for (int i = 0; i < n - 1; i++)
  {
    flight_schedule_reset(&array[i]);
    array[i].next = &array[i + 1];
    array[i + 1].prev = &array[i];
  }
  // Takes care of last node.
  // Its prev pointer will be set in the loop,
  // we just have to set its next to NULL.
  array[n - 1].next = NULL;
  flight_schedules_free = &array[0];
}

/***********************************************************
 * time_get: read a time from the user
   Time in this program is a minute number 0-((24*60)-1)=1439
   -1 is used to indicate the NULL empty time 
   This function should read in a time value and check its 
   validity.  If it is not valid eg. not -1 or not 0-1439
   It should print "Invalid Time" and return false.
   othewise it should return the value in the integer pointed
   to by time_ptr.
 ***********************************************************/
bool time_get(int *time_ptr)
{
  if (scanf("%d", time_ptr) == 1)
  {
    return (TIME_NULL == *time_ptr ||
            (*time_ptr >= TIME_MIN && *time_ptr <= TIME_MAX));
  }
  msg_time_bad();
  return false;
}

/***********************************************************
 * flight_capacity_get: read the capacity of a flight from the user
   This function should read in a capacity value and check its 
   validity.  If it is not greater than 0, it should print 
   "Invalid capacity value" and return false. Othewise it should 
   return the value in the integer pointed to by cap_ptr.
 ***********************************************************/
bool flight_capacity_get(int *cap_ptr)
{
  if (scanf("%d", cap_ptr) == 1)
  {
    return *cap_ptr > 0;
  }
  msg_capacity_bad();
  return false;
}

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs)
{
  qsort(fs->flights, MAX_FLIGHTS_PER_CITY, sizeof(struct flight),
        flight_compare_time);
}

int flight_compare_time(const void *a, const void *b)
{
  const struct flight *af = a;
  const struct flight *bf = b;

  return (af->time - bf->time);
}

struct flight_schedule *flight_schedule_find(city_t city){//active only

  struct flight_schedule *trav = flight_schedules_active; //pointer to traverse array

  while(trav){//sees if the city is the one we're looking for

    if(strcmp(city, trav->destination) == 0){//we found the city
      return trav;
    }

    trav = trav->next;
  }

  return trav;
}

struct flight_schedule *flight_schedule_allocate(void){
  struct flight_schedule *fltptr = flight_schedules_free; //pointer to first element of free array

  if(flight_schedules_free == NULL){//if we've reached the limit for flights
    return NULL;
  }
  if(fltptr->next == NULL){
    flight_schedules_free = NULL;
    fltptr->next = flight_schedules_active;
    flight_schedules_active->prev = fltptr;
    flight_schedules_active = fltptr;
  }else if(flight_schedules_active == NULL){//if there's nothing in the active yet
    flight_schedules_free = flight_schedules_free->next;
    fltptr->next = NULL; 
    flight_schedules_free->prev = NULL; //this and above line breaks the pointer off the array
    flight_schedules_active = fltptr;

  }else{//if there is something in the active
    flight_schedules_free = flight_schedules_free->next;
    fltptr->next = NULL; 
    flight_schedules_free->prev = NULL; //this and above line breaks the pointer off the array
    flight_schedules_active->prev = fltptr; //makes the first element of the active point to this pointer
    fltptr->next = flight_schedules_active; // fully attaches it to the list
    flight_schedules_active = flight_schedules_active->prev; //moves the pointer of the active list back to new beginning}
  }
  return fltptr;
}

void flight_schedule_free(struct flight_schedule *fs){

  if(fs->prev == NULL && fs->next == NULL){
    flight_schedules_active = NULL;
  
  }else if(fs->prev == NULL){//this means fs is at the front
    flight_schedules_active = flight_schedules_active->next;
    flight_schedules_active->prev = NULL;
    fs->next = NULL;
    flight_schedule_reset(fs);

  }else if(fs->next == NULL){//this means fs is at the end
    fs->prev->next = NULL;
    fs->prev = NULL;
    flight_schedule_reset(fs);

  }else{//fs is somewhere in the middle
    fs->prev->next = fs->next;
    fs->next->prev = fs->prev;
    fs->next = NULL;
    fs->prev = NULL;
    flight_schedule_reset(fs);
  }

  if(flight_schedules_free == NULL){
    flight_schedules_free = fs;
    return;
  }

  /*now move fs to beginning of the free list*/
  flight_schedules_free->prev = fs;
  fs->next = flight_schedules_free;
  flight_schedules_free = fs;
}

void flight_schedule_add(city_t city){ 

  if(flight_schedule_find(city)){
    msg_city_exists(city);
    return;
  }

  struct flight_schedule *to_add = flight_schedule_allocate();

  if(to_add == NULL){//if we cannot add any more flights
    msg_schedule_no_free();
    return;
  }

  strcpy(to_add->destination, city);

  return;
}

void flight_schedule_listAll(void){

  struct flight_schedule *trav = flight_schedules_active; //pointer to traverse array

  while(trav){//prints then moves to next element
    printf("%s\n", trav->destination);
    trav = trav->next;
  }

  return;
}

void flight_schedule_list(city_t city){
  struct flight_schedule *city_found = flight_schedule_find(city); // tells whether city exists

  if(city_found == NULL){//if the city does not exist
    msg_city_bad(city);
    return;
  }

  msg_city_flights(city);
  flight_schedule_sort_flights_by_time(city_found);
  int i;
  for(i = 0; i < MAX_FLIGHTS_PER_CITY; i++){
    if(city_found->flights[i].time == TIME_NULL){
      continue;
    }

    msg_flight_info(city_found->flights[i].time, city_found->flights[i].available, 
    city_found->flights[i].capacity);
  }
  printf("\n");
  return;
}

void flight_schedule_add_flight(city_t city){

  struct flight_schedule *fltptr = flight_schedule_find(city);
  if(fltptr == NULL){
    msg_city_bad(city);
    return;
  }
  int x; //variable to store our time
  int y; //variable to store our capacity

  if(!time_get(&x)){//makes sure time and capacity are valid
    return;
  }else if(!flight_capacity_get(&y)){
    return;
  }

  int i;
  for(i = 0; i < MAX_FLIGHTS_PER_CITY; i++){//iterates through flights array 
    if(fltptr->flights[i].time == TIME_NULL){//looks for a free index
      fltptr->flights[i].time = x;
      fltptr->flights[i].capacity = y;
      fltptr->flights[i].available = y;
      break;
    }
  }

  if(i == MAX_FLIGHTS_PER_CITY){//if we couldn't find a free index
    msg_city_max_flights_reached(city);
  }

  return;
}

void flight_schedule_remove_flight(city_t city){

  struct flight_schedule *fltptr = flight_schedule_find(city);
  if(fltptr == NULL){
    msg_city_bad(city);
    return;
  }
  int x; //variable to store time input
  
  if(!time_get(&x)){
    return;
  }

  int i;
  for(i = 0; i < MAX_FLIGHTS_PER_CITY; i++){
    if(fltptr->flights[i].time == x){
      fltptr->flights[i].time = TIME_NULL;
      fltptr->flights[i].capacity = 0;
      fltptr->flights[i].available = 0;
      break;
    }
  }
  if(i == MAX_FLIGHTS_PER_CITY){
    msg_flight_bad_time();
  }
  return;
}

void flight_schedule_schedule_seat(city_t city){

  struct flight_schedule *fltptr = flight_schedule_find(city);
  if(fltptr == NULL){//if the city does not exist
    msg_city_bad(city);
    return;
  }
  int x;
  if(!time_get(&x)){
    return;
  }

  int closest_difference = TIME_MAX + 1; //keeps track of closest flightat TIME_MAX+1
                                         // so that all valid times are counted
  int difference = 0;
  int index_closest = -1;
  int i;
  for(i = 0; i < MAX_FLIGHTS_PER_CITY; i++){
    if(fltptr->flights[i].time == TIME_NULL){//if there isn't a flight here
      continue;
    }
    if(fltptr->flights[i].available == 0){//if the flight is already full
      continue;
    }

    difference = fltptr->flights[i].time - x;

    if(difference < 0){
      continue;
    }
    if(difference < closest_difference){
      closest_difference = difference;
      index_closest = i;
    }
  }

  if(index_closest == -1){
    msg_flight_no_seats();
    return;
  }
  if(fltptr->flights[index_closest].available == 0){
    msg_flight_all_seats_empty();
    return;
  }

  fltptr->flights[index_closest].available -= 1;
}

void flight_schedule_unschedule_seat(city_t city){

  struct flight_schedule *fltptr = flight_schedule_find(city);
  if(fltptr == NULL){//if the city does not exist
    msg_city_bad(city);
    return;
  }
  int x;
  int i;
  if(!time_get(&x)){
    return;
  }
  for(i = 0; i < MAX_FLIGHTS_PER_CITY; i++){
    if(fltptr->flights[i].time == x){
      if(fltptr->flights[i].capacity == fltptr->flights[i].available){
        msg_flight_all_seats_empty();
        return;
      }
      fltptr->flights[i].available +=1;
      break;
    }
  }
  if(i == MAX_FLIGHTS_PER_CITY){
    msg_flight_bad_time();
  }

  return;
}

void flight_schedule_remove(city_t city){

  struct flight_schedule *to_remove = flight_schedule_find(city);
  if(to_remove == NULL){//if the city does not exist
    msg_city_bad(city);
    return;
  }

  flight_schedule_free(to_remove);
}
