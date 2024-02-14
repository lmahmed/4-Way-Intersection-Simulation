// CIS 450 Program 3 - Traffic Simulation
// Professor Shenquan Wang
// Authors: Latif Ahmed & Sukhmandeep Dhillon
// Spin and GetTime function from common.h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

const int count_car = 8;

// Enum to represent traffic light states
enum traffic_light 
{
    green, 
    yellow,
    red,
};
enum traffic_light north_south_light;
enum traffic_light west_east_light;

// Simulation Times in seconds
const double TIME_SIMULATION = 30;
const double TIME_GREEN = 18;
const double TIME_YELLOW = 2;
const double TIME_RED = 20;
const double TIME_LEFT = 3;
const double TIME_STRAIGHT = 2;
const double TIME_RIGHT = 1;


double start_time; // for a relative starting time

// vars for back-to-back if all conditions are met
int count_south_left = 0;  // meaning vehicles that are coming from the south that are making a left turn.
int count_south_straight = 0;
int count_south_right = 0;

int count_west_left = 0;
int count_west_straight = 0;
int count_west_right = 0;

int count_east_left = 0;
int count_east_straight = 0;
int count_east_right = 0;

int count_north_left = 0;
int count_north_straight = 0;
int count_north_right = 0;

// Head-of-line (hol) locks
sem_t north_side_hol;
sem_t south_side_hol;
sem_t east_side_hol;
sem_t west_side_hol;

// Incoming traffic locks
sem_t north_side_inc;
sem_t south_side_inc;
sem_t east_side_inc;
sem_t west_side_inc;

// Middle of intersection locks
sem_t north_west_loc;
sem_t north_east_loc;
sem_t south_west_loc;
sem_t south_east_loc;

typedef struct _directions 
{
    char dir_original;
    char dir_target;
} directions;

typedef struct _car
{
    int cid;
    double arrival_time;
    directions directions; 
} car;


// from common.h
double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double)t.tv_sec + (double)t.tv_usec/1e6;
}

// from common.h
void Spin(double howlong) {
    double t = GetTime();
    while ((GetTime() - t) < howlong)
	; // do nothing in loop
}

void ArriveIntersection(car *car)
{
    printf("Time  %.1f: Car %d (%c %c) arriving\n", GetTime() - start_time, car->cid, car->directions.dir_original, car->directions.dir_target);

    char travel_direction; // relative to original direction < = left turn, > = right turn, ^ = straight
    switch (car->directions.dir_original)
    {
        case '^': 
            sem_wait(&south_side_hol);
            switch (car->directions.dir_target)
            {
                case '^':
                    travel_direction = '^';
                    break;
                case '<':
                    travel_direction = '<';
                    break;
                case '>':
                    travel_direction = '>';
                    break;
            }
            switch (travel_direction)
            {
                case '^':
                    if (count_south_straight == 0 || north_south_light != green)
                    {
                        int result = sem_trywait(&south_east_loc);
                        int result1 = sem_trywait(&north_side_inc);
                        while (north_south_light != green || !(result == 0 && result1 == 0))
                        {
                            if (result == 0)
                            {
                                sem_post(&south_east_loc);
                            }
                            if (result1 == 0)
                            {
                                sem_post(&north_side_inc);
                            }
                            result = sem_trywait(&south_east_loc);
                            result1 = sem_trywait(&north_side_inc);
                        }
                    }

                    count_south_straight++;
                    
                    break;
                case '<':
                    if (count_south_left == 0 || north_south_light != green)
                    {
                        int result = sem_trywait(&north_west_loc);
                        int result1 = sem_trywait(&west_side_inc);
                        while (north_south_light != green || !(result == 0 && result1 == 0))
                        {
                            if (result == 0)
                            {
                                sem_post(&north_west_loc);
                            }
                            if (result1 == 0)
                            {
                                sem_post(&west_side_inc);
                            }
                            result = sem_trywait(&north_west_loc);
                            result1 = sem_trywait(&west_side_inc);
                        }
                    }

                    count_south_left++;

                    break;
                case '>':
                    if (count_south_right == 0 || north_south_light == red) 
                    {
                        int result = sem_trywait(&east_side_inc);
                        while (!(result == 0)) 
                        {
                            result = sem_trywait(&east_side_inc);
                        }
                    }
                    count_south_right++;
                    break;
            }
            break;
        case '<':
            sem_wait(&east_side_hol);
            switch (car->directions.dir_target)
                {
                    case '^':
                        travel_direction = '>';
                        break;
                    case '<':
                        travel_direction = '^';
                        break;
                    case 'v':
                        travel_direction = '<';
                        break;
                }
            switch (travel_direction)
            {
                case '^':
                    if (count_east_straight == 0 || west_east_light != green)
                    {
                        int result = sem_trywait(&north_east_loc);
                        int result1 = sem_trywait(&west_side_inc);
                        while (west_east_light != green || !(result == 0 && result1 == 0))
                        {
                            if (result == 0)
                            {
                                sem_post(&north_east_loc);
                            }
                            if (result1 == 0)
                            {
                                sem_post(&west_side_inc);
                            }
                            result = sem_trywait(&north_east_loc);
                            result1 = sem_trywait(&west_side_inc);
                        }
                    }
                    count_east_straight++;

                    break;
                case '<':
                    if (count_east_left == 0 || west_east_light != green)
                    {
                        int result = sem_trywait(&south_west_loc);
                        int result1 = sem_trywait(&south_side_inc);
                        while (west_east_light != green || !(result == 0 && result1 == 0))
                        {
                            if (result == 0)
                            {
                                sem_post(&south_west_loc);
                            }
                            if (result1 == 0)
                            {
                                sem_post(&south_side_inc);
                            }
                            result = sem_trywait(&south_west_loc);
                            result1 = sem_trywait(&south_side_inc);
                        }
                    }

                    count_east_left++;
                    break;
                case '>':
                    if (count_east_right == 0 || west_east_light == red) 
                    {
                        int result = sem_trywait(&north_side_inc);
                        while (!(result == 0)) 
                        {
                            result = sem_trywait(&north_side_inc);
                        }
                    }
                    count_east_right++;
                    break;
            }
            break;
        case '>':
            sem_wait(&west_side_hol);
            switch (car->directions.dir_target)
                {
                    case '^':
                        travel_direction = '<';
                        break;
                    case '>':
                        travel_direction = '^';
                        break;
                    case 'v':
                        travel_direction = '>';
                        break;
                }
            switch (travel_direction)
            {
                case '^':
                    if (count_west_straight == 0 || west_east_light != green)
                    {
                        int result = sem_trywait(&south_west_loc);
                        int result1 = sem_trywait(&east_side_inc);
                        while (west_east_light != green || !(result == 0 && result1 == 0))
                        {
                            if (result == 0)
                            {
                                sem_post(&south_west_loc);
                            }
                            if (result1 == 0)
                            {
                                sem_post(&east_side_inc);
                            }
                            result = sem_trywait(&south_west_loc);
                            result1 = sem_trywait(&east_side_inc);
                        }
                    }
                    count_west_straight++;
                    break;
                case '<':
                    if (count_west_left == 0 || west_east_light != green)
                    {
                        int result = sem_trywait(&north_east_loc);
                        int result1 = sem_trywait(&north_side_inc);
                        while (west_east_light != green || !(result == 0 && result1 == 0))
                        {
                            if (result == 0)
                            {
                                sem_post(&north_east_loc);
                            }
                            if (result1 == 0)
                            {
                                sem_post(&north_side_inc);
                            }
                            result = sem_trywait(&north_east_loc);
                            result1 = sem_trywait(&north_side_inc);
                        }
                    }

                    count_west_left++;
                    break;
                case '>':
                    if (count_west_right == 0 || west_east_light == red) 
                    {
                        int result = sem_trywait(&south_side_inc);
                        while (!(result == 0)) 
                        {
                            result = sem_trywait(&south_side_inc);
                        }
                    }
                    count_west_right++;
                    break;
            }
            break;
        case 'v':
            sem_wait(&north_side_hol);
            switch (car->directions.dir_target)
                {
                    case '>':
                        travel_direction = '<';
                        break;
                    case 'v':
                        travel_direction = '^';
                        break;
                    case '<':
                        travel_direction = '>';
                        break;
                }
            switch (travel_direction)
            {
                case '^':
                    if (count_north_straight == 0 || north_south_light != green)
                    {
                        int result = sem_trywait(&north_west_loc);
                        int result1 = sem_trywait(&south_side_inc);
                        while (north_south_light != green || !(result == 0 && result1 == 0))
                        {
                            if (result == 0)
                            {
                                sem_post(&north_west_loc);
                            }
                            if (result1 == 0)
                            {
                                sem_post(&south_side_inc);
                            }
                            result = sem_trywait(&north_west_loc);
                            result1 = sem_trywait(&south_side_inc);
                        }
                    }

                    count_north_straight++;
                    break;
                case '<':
                    if (count_north_left == 0 || north_south_light != green)
                    {
                        int result = sem_trywait(&south_east_loc);
                        int result1 = sem_trywait(&east_side_inc);
                        while (north_south_light != green || !(result == 0 && result1 == 0))
                        {
                            if (result == 0)
                            {
                                sem_post(&south_east_loc);
                            }
                            if (result1 == 0)
                            {
                                sem_post(&east_side_inc);
                            }
                            result = sem_trywait(&south_east_loc);
                            result1 = sem_trywait(&east_side_inc);
                        }
                    }

                    count_north_left++;
                    break;
                case '>':
                    if (count_north_right == 0 || north_south_light == red) 
                    {
                        int result = sem_trywait(&west_side_inc);
                        while (!(result == 0)) 
                        {
                            result = sem_trywait(&west_side_inc);
                        }
                    }
                    count_north_right++;
                    break;
            }
            break; 
    }
}

void  CrossIntersection(car *car)
{
    printf("Time  %.1f: Car %d (%c %c)          crossing\n", GetTime() - start_time, car->cid, car->directions.dir_original, car->directions.dir_target);

    char travel_direction; // relative to original direction < = left turn, > = right turn, ^ = straight
    switch (car->directions.dir_original)
    {
        case '^':
            sem_post(&south_side_hol);
            switch (car->directions.dir_target)
            {
                case '^':
                    travel_direction = '^';
                    break;
                case '<':
                    travel_direction = '<';
                    break;
                case '>':
                    travel_direction = '>';
                    break;
            }
            break;
        case '<':
            sem_post(&east_side_hol);
            switch (car->directions.dir_target)
                {
                    case '^':
                        travel_direction = '>';
                        break;
                    case '<':
                        travel_direction = '^';
                        break;
                    case 'v':
                        travel_direction = '<';
                        break;
                }
            break;
        case '>':
            sem_post(&west_side_hol);
            switch (car->directions.dir_target)
                {
                    case '^':
                        travel_direction = '<';
                        break;
                    case '>':
                        travel_direction = '^';
                        break;
                    case 'v':
                        travel_direction = '>';
                        break;
                }
            break;
        case 'v':
            sem_post(&north_side_hol);
            switch (car->directions.dir_target)
                {
                    case '>':
                        travel_direction = '<';
                        break;
                    case 'v':
                        travel_direction = '^';
                        break;
                    case '<':
                        travel_direction = '>';
                        break;
                }
            break; 
    }
    switch (travel_direction)
    {
        case '<':
            Spin(TIME_LEFT);
            break;
        case '>':
            Spin(TIME_RIGHT);
            break;
        case '^':
            Spin(TIME_STRAIGHT);
            break;
    }
}

void ExitIntersection(car *car)
{
    printf("Time  %.1f: Car %d (%c %c)                   exiting\n", GetTime() - start_time, car->cid, car->directions.dir_original, car->directions.dir_target);

    char travel_direction;

    switch (car->directions.dir_original)
    {
        case '^':
            switch (car->directions.dir_target)
            {
                case '^':
                    travel_direction = '^';
                    break;
                case '<':
                    travel_direction = '<';
                    break;
                case '>':
                    travel_direction = '>';
                    break;
            }
            switch (travel_direction)
            {
                case '^':
                    count_south_straight--;
                    if (count_south_straight == 0)
                    {
                        sem_post(&south_east_loc);
                        sem_post(&north_side_inc);
                    }
                    break;
                case '<':
                    count_south_left--;
                    if (count_south_left == 0)
                    {
                        sem_post(&north_west_loc);
                        sem_post(&west_side_inc);
                    }
                    break;
                case '>':
                    count_south_right--;
                    if (count_south_right == 0)
                    {
                        sem_post(&east_side_inc);
                    }
                    break;
            }
            break;
        case '<':
            switch (car->directions.dir_target)
                {
                    case '^':
                        travel_direction = '>';
                        break;
                    case '<':
                        travel_direction = '^';
                        break;
                    case 'v':
                        travel_direction = '<';
                        break;
                }
            switch (travel_direction)
            {
                case '^':
                    count_east_straight--;
                    if (count_east_straight == 0)
                    {
                        sem_post(&north_east_loc);
                        sem_post(&west_side_inc);
                    }
                    break;
                case '<':
                    count_east_left--;
                    if (count_east_left == 0)
                    {
                        sem_post(&south_west_loc);
                        sem_post(&south_side_inc);
                    }
                    break;
                case '>':
                    count_east_right--;
                    if (count_east_right == 0)
                    {
                        sem_post(&north_side_inc);
                    }
                    break;
            }
            break;
        case '>':
            switch (car->directions.dir_target)
                {
                    case '^':
                        travel_direction = '<';
                        break;
                    case '>':
                        travel_direction = '^';
                        break;
                    case 'v':
                        travel_direction = '>';
                        break;
                }
            switch (travel_direction)
            {
                case '^':
                    count_west_straight--;
                    if (count_west_straight == 0)
                    {
                        sem_post(&south_west_loc);
                        sem_post(&east_side_inc);
                    }
                    break;
                case '<':
                    count_west_left--;
                    if (count_west_left == 0)
                    {
                        sem_post(&north_east_loc);
                        sem_post(&north_side_inc);
                    }
                    break;
                case '>':
                    count_west_right--;
                    if (count_west_right == 0)
                    {
                        sem_post(&south_side_inc);
                    }
                    break;
            }
            break;
        case 'v':
            switch (car->directions.dir_target)
                {
                    case '>':
                        travel_direction = '<';
                        break;
                    case 'v':
                        travel_direction = '^';
                        break;
                    case '<':
                        travel_direction = '>';
                        break;
                }
            switch (travel_direction)
            {
                case '^':
                    count_north_straight--;
                    if (count_north_straight == 0)
                    {
                        sem_post(&north_west_loc);
                        sem_post(&south_side_inc);
                    }
                    break;
                case '<':
                    count_north_left--;
                    if (count_north_left == 0)
                    {
                        sem_post(&south_east_loc);
                        sem_post(&east_side_inc);
                    }
                    break;
                case '>':
                    count_north_right--;
                    if (count_north_right == 0)
                    {
                        sem_post(&west_side_inc);
                    }
                    break;
            }
            break; 
    }
}

void *Car(void *car) 
{
    ArriveIntersection(car);
    CrossIntersection(car);
    ExitIntersection(car);
}

void InitializeCars(car car_arr[])
{
    // Start Input 1
    car_arr[0].cid = 0;
    car_arr[0].arrival_time = 1.0;
    car_arr[0].directions.dir_original = '^';
    car_arr[0].directions.dir_target = '^';

    car_arr[1].cid = 1;
    car_arr[1].arrival_time = 1.9;
    car_arr[1].directions.dir_original = '^';
    car_arr[1].directions.dir_target = '^';

    car_arr[2].cid = 2;
    car_arr[2].arrival_time = 3.2;
    car_arr[2].directions.dir_original = '^';
    car_arr[2].directions.dir_target = '<';

    car_arr[3].cid = 3;
    car_arr[3].arrival_time = 3.4;
    car_arr[3].directions.dir_original = 'v';
    car_arr[3].directions.dir_target = 'v';

    car_arr[4].cid = 4;
    car_arr[4].arrival_time = 4.1;
    car_arr[4].directions.dir_original = 'v';
    car_arr[4].directions.dir_target = '>';

    car_arr[5].cid = 5;
    car_arr[5].arrival_time = 4.3;
    car_arr[5].directions.dir_original = '^';
    car_arr[5].directions.dir_target = '^';

    car_arr[6].cid = 6;
    car_arr[6].arrival_time = 5.6;
    car_arr[6].directions.dir_original = '>';
    car_arr[6].directions.dir_target = '^';

    car_arr[7].cid = 7;
    car_arr[7].arrival_time = 5.8;
    car_arr[7].directions.dir_original = '<';
    car_arr[7].directions.dir_target = '^';
    
    // End Input 1

    // Start Input 2
    // car_arr[0].cid = 0;
    // car_arr[0].arrival_time = 1.0;
    // car_arr[0].directions.dir_original = '^'; 
    // car_arr[0].directions.dir_target = '<';

    // car_arr[1].cid = 1;
    // car_arr[1].arrival_time = 1.5;
    // car_arr[1].directions.dir_original = 'v';
    // car_arr[1].directions.dir_target = '>';

    // End Input 2

}

void *TrafficLight()
{
    bool north_south_green; // true represents north-south green/yellow cycle, false represents east-west green/yellow cycle
    while (true)
    {
        north_south_green = north_south_light == green; 
        
        Spin(TIME_GREEN);
        if (north_south_green)
        {
            north_south_light = yellow;
        }
        else
        {
            west_east_light = yellow;
        } 
        
        Spin(TIME_YELLOW);
        if (north_south_green)
        {
            north_south_light = red;
            west_east_light = green; 
        } 
        else
        {
            west_east_light = red;
            north_south_light = green;
        }
    }
}

int main(void) 
{
    pthread_t car_thread[count_car];
    car car_arr[count_car]; // must be sorted by arrival time, least to greatest
    InitializeCars(car_arr); // must initialize count_car cars

    // Initalize traffic lights
    north_south_light = green;
    west_east_light = red;
    // Initalize semaphores
    sem_init(&north_side_hol, 0, 1);
    sem_init(&south_side_hol, 0, 1);
    sem_init(&east_side_hol, 0, 1);
    sem_init(&west_side_hol, 0, 1);

    sem_init(&north_side_inc, 0, 1);
    sem_init(&south_side_inc, 0, 1);
    sem_init(&east_side_inc, 0, 1);
    sem_init(&west_side_inc, 0, 1);

    sem_init(&north_west_loc, 0, 1);
    sem_init(&north_east_loc, 0, 1);
    sem_init(&south_west_loc, 0, 1);
    sem_init(&south_east_loc, 0, 1);

    start_time = GetTime(); // Set relative starting time
    printf("Time  0.0: Simulation Start\n");

    pthread_t traffic_light_thread;
    pthread_create(&traffic_light_thread, NULL, TrafficLight, NULL);

    // Test initialization of cars
    // printf("%d %f %c %c \n",  car_arr[7].cid,  car_arr[7].arrival_time,  car_arr[7].directions.dir_original,  car_arr[7].directions.dir_target);
    
    // printf("%f \n", start_time);

    double elapsed_time = GetTime() - start_time;
    int car_iterator = 0;
    while (elapsed_time < TIME_SIMULATION)
    {
        while (car_iterator < count_car && elapsed_time > car_arr[car_iterator].arrival_time)
        {
            pthread_create(&car_thread[car_iterator], NULL, Car, &car_arr[car_iterator]);
            car_iterator++;
        }
        elapsed_time = GetTime() - start_time;
    }
        printf("Time  %.1f: Simulation End\n", elapsed_time);
   
    return 0;
}