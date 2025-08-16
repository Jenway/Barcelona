#include "philo.h"

// 破坏“循环等待”：资源有序分配法

// Strategy V1: Breaks the "Circular Wait" condition by enforcing a strict
// order in which forks (resources) are acquired. Each philosopher must
// pick up the fork with the lower ID first.

static void strategy_v1_take(t_philo* philo)
{
    pthread_mutex_lock(philo->left_fork);
    print_status(philo, "has taken a fork");
    pthread_mutex_lock(philo->right_fork);
    print_status(philo, "has taken a fork");
}

static void strategy_v1_put(t_philo* philo)
{
    // The order of unlocking doesn't matter for correctness,
    // but it's good practice to unlock in the reverse order of locking.
    pthread_mutex_unlock(philo->right_fork);
    pthread_mutex_unlock(philo->left_fork);
}

static int strategy_v1_init(t_sim* sim)
{
    sim->fork_mutexes = malloc(sizeof(pthread_mutex_t) * sim->num_philos);
    if (!sim->fork_mutexes) {
        perror("Failed to allocate memory for fork_mutexes");
        return 1;
    }

    for (int i = 0; i < sim->num_philos; i++) {
        pthread_mutex_init(&sim->fork_mutexes[i], NULL);
    }

    for (int i = 0; i < sim->num_philos; i++) {
        t_philo* philo = &sim->philos[i];
        int left_fork_idx = i;
        int right_fork_idx = (i + 1) % sim->num_philos;

        // This is the key to resource ordering: always lock the smaller
        // index mutex first.
        if (left_fork_idx < right_fork_idx) {
            philo->left_fork = &sim->fork_mutexes[left_fork_idx];
            philo->right_fork = &sim->fork_mutexes[right_fork_idx];
        } else {
            // The last philosopher has forks (N-1) and 0. Swap them.
            philo->left_fork = &sim->fork_mutexes[right_fork_idx];
            philo->right_fork = &sim->fork_mutexes[left_fork_idx];
        }
    }
    return 0; // Success
}

static void strategy_v1_cleanup(t_sim* sim)
{
    for (int i = 0; i < sim->num_philos; i++) {
        pthread_mutex_destroy(&sim->fork_mutexes[i]);
    }
    free(sim->fork_mutexes);
    sim->fork_mutexes = NULL;
}

t_philo_strategy get_strategy_v1(void)
{
    t_philo_strategy strategy;
    strategy.init = strategy_v1_init;
    strategy.cleanup = strategy_v1_cleanup;
    strategy.take_forks = strategy_v1_take;
    strategy.put_forks = strategy_v1_put;
    return strategy;
}
