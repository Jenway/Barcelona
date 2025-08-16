// philo_bonus.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>

#define SEM_FORKS "/sem_forks"
#define SEM_PRINT "/sem_print"

typedef struct s_args {
    int id;
    int time_to_die;
    int time_to_eat;
    int time_to_sleep;
    int must_eat;
    long start_time;
    sem_t *sem_forks;
    sem_t *sem_print;
} t_args;

long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void print_action(t_args *args, char *msg) {
    sem_wait(args->sem_print);
    long now = get_time_ms() - args->start_time;
    printf("%ld %d %s\n", now, args->id, msg);
    sem_post(args->sem_print);
}

void precise_sleep(long ms) {
    long start = get_time_ms();
    while (get_time_ms() - start < ms)
        usleep(100);
}

void *monitor_routine(void *arg) {
    t_args *args = (t_args *)arg;
    long last_meal = get_time_ms();
    while (1) {
        if (get_time_ms() - last_meal > args->time_to_die) {
            sem_wait(args->sem_print);
            printf("%ld %d died\n", get_time_ms() - args->start_time, args->id);
            exit(1);
        }
        usleep(1000);
    }
    return NULL;
}

void philosopher(t_args *args) {
    pthread_t monitor;
    pthread_create(&monitor, NULL, monitor_routine, args);
    pthread_detach(monitor);
    int eat_count = 0;

    while (args->must_eat == -1 || eat_count < args->must_eat) {
        print_action(args, "is thinking");
        sem_wait(args->sem_forks);
        sem_wait(args->sem_forks);
        print_action(args, "has taken a fork");
        print_action(args, "has taken a fork");
        print_action(args, "is eating");
        precise_sleep(args->time_to_eat);
        sem_post(args->sem_forks);
        sem_post(args->sem_forks);
        eat_count++;
        print_action(args, "is sleeping");
        precise_sleep(args->time_to_sleep);
    }
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 5 || argc > 6) {
        printf("Usage: %s num_philos time_to_die time_to_eat time_to_sleep [must_eat]\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int t_die = atoi(argv[2]);
    int t_eat = atoi(argv[3]);
    int t_sleep = atoi(argv[4]);
    int must_eat = (argc == 6) ? atoi(argv[5]) : -1;

    sem_unlink(SEM_FORKS);
    sem_unlink(SEM_PRINT);
    sem_t *sem_forks = sem_open(SEM_FORKS, O_CREAT, 0644, n);
    sem_t *sem_print = sem_open(SEM_PRINT, O_CREAT, 0644, 1);

    pid_t *pids = malloc(sizeof(pid_t) * n);
    long start = get_time_ms();

    for (int i = 0; i < n; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            t_args args = {
                .id = i + 1,
                .time_to_die = t_die,
                .time_to_eat = t_eat,
                .time_to_sleep = t_sleep,
                .must_eat = must_eat,
                .start_time = start,
                .sem_forks = sem_forks,
                .sem_print = sem_print
            };
            philosopher(&args);
        }
    }

    int status;
    waitpid(-1, &status, 0);
    for (int i = 0; i < n; i++) {
        kill(pids[i], SIGTERM);
    }
    sem_close(sem_forks);
    sem_close(sem_print);
    sem_unlink(SEM_FORKS);
    sem_unlink(SEM_PRINT);
    free(pids);
    return 0;
}

