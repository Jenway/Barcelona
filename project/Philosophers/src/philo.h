#ifndef PHILO_H
#define PHILO_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
// --- Constants ---
#define MAX_PHILOS 200
#define MAX_EVENTS 1024
#define INITIAL_BACKOFF_US 500

// --- Event Queue Types ---
typedef enum {
    EVENT_EAT,
    EVENT_DIED,
    EVENT_FINISHED
} philo_event_type;

typedef struct {
    int philo_id;
    philo_event_type type;
} philo_event;

typedef struct {
    philo_event buf[MAX_EVENTS];
    int head;
    int tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} event_queue_t;

// --- Forward Declarations ---
typedef struct s_philo t_philo;
typedef struct s_sim t_sim;

// --- Monitor for Fork Management ---
typedef struct s_fork_monitor {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int is_taken[MAX_PHILOS];
    int num_forks;
} t_fork_monitor;

// --- Strategy Pattern ---
typedef struct s_philo_strategy {
    int (*init)(t_sim* sim);
    void (*cleanup)(t_sim* sim);
    void (*take_forks)(t_philo* philo);
    void (*put_forks)(t_philo* philo);
} t_philo_strategy;

// --- Main Simulation Struct ---
typedef struct s_sim {
    int num_philos;
    long time_to_die;
    long time_to_eat;
    long time_to_sleep;
    int num_must_eat;

    long start_time;
    t_philo* philos;
    t_philo_strategy strategy;
    event_queue_t event_queue;
    long next_deadline_ms[MAX_PHILOS];

    pthread_mutex_t print_mutex;
    pthread_mutex_t sim_stop_mutex;
    volatile bool stop_simulation;

    // --- Strategy-specific data ---
    // For v1
    pthread_mutex_t* fork_mutexes;
    // For v2/v3
    t_fork_monitor fork_monitor;
    // For v3
    sem_t limiter_sem;

} t_sim;

// --- Philosopher Struct ---
struct s_philo {
    int id;
    pthread_t thread;
    pthread_mutex_t data_mutex;
    long last_meal_time;
    int meal_count;
    t_sim* sim;

    // For v1
    pthread_mutex_t* left_fork;
    pthread_mutex_t* right_fork;

    // For exponential backoff in v3
    int backoff_us;
};

// --- Function Prototypes ---

// utils.c
long get_time_in_ms(void);
void print_status(t_philo* philo, const char* status);
t_sim* parse_args(int argc, char** argv);
void queue_init(event_queue_t* q);
void enqueue(event_queue_t* q, philo_event ev);
philo_event dequeue(event_queue_t* q);

// routines.c
void* philo_routine(void* arg);
void* monitor_routine(void* arg);
void* watchdog_routine(void* arg);

// strategies
t_philo_strategy get_strategy_v1(void);
t_philo_strategy get_strategy_v2(void);
t_philo_strategy get_strategy_v3(void);
t_philo_strategy get_strategy_v4(void);

#endif
