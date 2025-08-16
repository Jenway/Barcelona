#include "philo.h"

// 破坏“占有并等待”：一次性申请

// Strategy V2: Breaks the "Hold and Wait" condition by acquiring both forks
// at once or waiting until both are available. This is done using a monitor.

static void strategy_v2_take(t_philo* philo)
{
    t_fork_monitor* fm = &philo->sim->fork_monitor;
    int left = philo->id - 1;
    int right = philo->id % philo->sim->num_philos;

    pthread_mutex_lock(&fm->mutex);
    // Wait until both forks are free
    while (fm->is_taken[left] || fm->is_taken[right]) {
        // Before waiting, check if simulation has stopped
        if (philo->sim->stop_simulation) {
            pthread_mutex_unlock(&fm->mutex);
            return;
        }
        pthread_cond_wait(&fm->cond, &fm->mutex);
    }
    fm->is_taken[left] = 1;
    fm->is_taken[right] = 1;
    pthread_mutex_unlock(&fm->mutex);

    print_status(philo, "has taken a fork");
    print_status(philo, "has taken a fork");
}

static void strategy_v2_put(t_philo* philo)
{
    t_fork_monitor* fm = &philo->sim->fork_monitor;
    int left = philo->id - 1;
    int right = philo->id % philo->sim->num_philos;

    pthread_mutex_lock(&fm->mutex);
    fm->is_taken[left] = 0;
    fm->is_taken[right] = 0;
    // Wake up all waiting philosophers as multiple might be able to proceed
    pthread_cond_broadcast(&fm->cond);
    pthread_mutex_unlock(&fm->mutex);
}

static int strategy_v2_init(t_sim* sim)
{
    // Initialize the fork monitor
    pthread_mutex_init(&sim->fork_monitor.mutex, NULL);
    pthread_cond_init(&sim->fork_monitor.cond, NULL);
    sim->fork_monitor.num_forks = sim->num_philos;
    for (int i = 0; i < sim->num_philos; i++) {
        sim->fork_monitor.is_taken[i] = 0;
    }
    return 0; // Success
}

static void strategy_v2_cleanup(t_sim* sim)
{
    pthread_mutex_destroy(&sim->fork_monitor.mutex);
    pthread_cond_destroy(&sim->fork_monitor.cond);
}

// Public function to get the strategy implementation
t_philo_strategy get_strategy_v2(void)
{
    t_philo_strategy strategy;
    strategy.init = strategy_v2_init;
    strategy.cleanup = strategy_v2_cleanup;
    strategy.take_forks = strategy_v2_take;
    strategy.put_forks = strategy_v2_put;
    return strategy;
}
