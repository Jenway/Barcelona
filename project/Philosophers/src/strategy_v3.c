#include "philo.h"

// 超时放弃

// Strategy V3: Breaks "Circular Wait" by limiting the number of philosophers
// who can try to pick up forks simultaneously to N-1. It also uses a timeout
// and backoff mechanism to prevent livelock.

typedef struct {
    int left;
    int right;
    t_fork_monitor* fm;
} t_fork_ctx;

static int can_take_forks(void* arg)
{
    t_fork_ctx* ctx = (t_fork_ctx*)arg;
    return !ctx->fm->is_taken[ctx->left] && !ctx->fm->is_taken[ctx->right];
}

static void do_take_forks(void* arg)
{
    t_fork_ctx* ctx = (t_fork_ctx*)arg;
    ctx->fm->is_taken[ctx->left] = 1;
    ctx->fm->is_taken[ctx->right] = 1;
}

static void do_put_forks(void* arg)
{
    t_fork_ctx* ctx = (t_fork_ctx*)arg;
    ctx->fm->is_taken[ctx->left] = 0;
    ctx->fm->is_taken[ctx->right] = 0;
}

// This helper function remains the same as in the original V3
static int try_execute_with_timeout(
    pthread_mutex_t* mtx,
    int (*predicate)(void*),
    void (*action)(void*),
    void* arg,
    int timeout_ms)
{
    struct timespec start_ts, now_ts;
    clock_gettime(CLOCK_MONOTONIC, &start_ts);

    while (1) {
        pthread_mutex_lock(mtx);
        if (predicate(arg)) {
            if (action)
                action(arg);
            pthread_mutex_unlock(mtx);
            return 1; // Success
        }
        pthread_mutex_unlock(mtx);

        clock_gettime(CLOCK_MONOTONIC, &now_ts);
        long elapsed_ms = (now_ts.tv_sec - start_ts.tv_sec) * 1000 + (now_ts.tv_nsec - start_ts.tv_nsec) / 1000000;
        if (elapsed_ms >= timeout_ms) {
            return 0; // Timeout
        }
        usleep(200); // Small sleep to prevent busy-waiting
    }
}

static void strategy_v3_take(t_philo* philo)
{
    t_sim* sim = philo->sim;
    t_fork_monitor* fm = &sim->fork_monitor;
    int left = philo->id - 1;
    int right = philo->id % sim->num_philos;
    t_fork_ctx ctx = { left, right, fm };

    const int TRY_TIMEOUT_MS = 100;
    const int MAX_BACKOFF_US = (sim->time_to_die * 1000) / 4;

    while (!sim->stop_simulation) {
        // 1. Try to acquire a slot from the semaphore (non-blocking)
        if (sem_trywait(&sim->limiter_sem) == 0) {
            // Success: we have a slot. Now try to get the forks.
            if (try_execute_with_timeout(&fm->mutex, can_take_forks, do_take_forks, &ctx, TRY_TIMEOUT_MS)) {
                print_status(philo, "has taken a fork");
                print_status(philo, "has taken a fork");
                return; // Success, we have the slot and the forks
            } else {
                // Timed out getting forks: release the semaphore slot before cooling off
                sem_post(&sim->limiter_sem);
            }
        }

        // --- Randomized Exponential Backoff ---
        // If sem_trywait failed, or if we timed out getting forks, we cool off.

        // Calculate sleep time with jitter (randomness) to avoid synchronized retries
        int backoff = philo->backoff_us > 0 ? philo->backoff_us : 1;
        int sleep_us = backoff + (rand() % backoff);
        usleep((useconds_t)sleep_us);

        // Increase backoff time for the next attempt (exponential)
        pthread_mutex_lock(&philo->data_mutex);
        philo->backoff_us *= 2;
        if (philo->backoff_us > MAX_BACKOFF_US) {
            philo->backoff_us = MAX_BACKOFF_US;
        }
        pthread_mutex_unlock(&philo->data_mutex);
    }
}

static void strategy_v3_put(t_philo* philo)
{
    t_sim* sim = philo->sim;
    t_fork_monitor* fm = &sim->fork_monitor;
    int left = philo->id - 1;
    int right = philo->id % sim->num_philos;
    t_fork_ctx ctx = { left, right, fm };

    // Release forks
    pthread_mutex_lock(&fm->mutex);
    do_put_forks(&ctx);
    pthread_mutex_unlock(&fm->mutex);

    // Release the semaphore slot
    sem_post(&sim->limiter_sem);
}

static int strategy_v3_init(t_sim* sim)
{
    // Init fork monitor
    pthread_mutex_init(&sim->fork_monitor.mutex, NULL);
    sim->fork_monitor.num_forks = sim->num_philos;
    for (int i = 0; i < sim->num_philos; i++) {
        sim->fork_monitor.is_taken[i] = 0;
    }

    // Init semaphore limiter
    if (sem_init(&sim->limiter_sem, 0, sim->num_philos - 1) != 0) {
        perror("sem_init failed");
        return 1;
    }

    srand((unsigned int)(time(NULL) ^ (getpid() << 16)));
    return 0;
}

static void strategy_v3_cleanup(t_sim* sim)
{
    pthread_mutex_destroy(&sim->fork_monitor.mutex);
    sem_destroy(&sim->limiter_sem);
}

t_philo_strategy get_strategy_v3(void)
{
    t_philo_strategy strategy;
    strategy.init = strategy_v3_init;
    strategy.cleanup = strategy_v3_cleanup;
    strategy.take_forks = strategy_v3_take;
    strategy.put_forks = strategy_v3_put;
    return strategy;
}
