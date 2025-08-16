#include "philo.h"

/**
 * @brief Initializes common simulation components like mutexes and philosophers.
 * @param sim The simulation state.
 * @return 0 on success, 1 on failure.
 */
static int init_common(t_sim* sim)
{
    sim->philos = malloc(sizeof(t_philo) * sim->num_philos);
    if (!sim->philos) {
        perror("Failed to allocate memory for philosophers");
        return 1;
    }

    pthread_mutex_init(&sim->print_mutex, NULL);
    pthread_mutex_init(&sim->sim_stop_mutex, NULL);
    sim->stop_simulation = false;

    queue_init(&sim->event_queue);

    for (int i = 0; i < sim->num_philos; i++) {
        t_philo* philo = &sim->philos[i];
        philo->id = i + 1;
        philo->sim = sim;
        philo->last_meal_time = sim->start_time;
        philo->meal_count = 0;
        sim->next_deadline_ms[i] = sim->start_time + sim->time_to_die;
        pthread_mutex_init(&philo->data_mutex, NULL);
    }
    return 0;
}

/**
 * @brief Cleans up common simulation components.
 * @param sim The simulation state.
 */
static void cleanup_common(t_sim* sim)
{
    pthread_mutex_destroy(&sim->print_mutex);
    pthread_mutex_destroy(&sim->sim_stop_mutex);
    pthread_mutex_destroy(&sim->event_queue.mutex);
    pthread_cond_destroy(&sim->event_queue.cond);

    for (int i = 0; i < sim->num_philos; i++) {
        pthread_mutex_destroy(&sim->philos[i].data_mutex);
    }
    free(sim->philos);
    sim->philos = NULL;
}

int main(int argc, char** argv)
{
    t_sim* sim = parse_args(argc, argv);
    if (!sim)
        return 1;

    t_philo_strategy strategy;

    if (strcmp(argv[5], "v1") == 0) {
        strategy = get_strategy_v1();
    } else if (strcmp(argv[5], "v2") == 0) {
        strategy = get_strategy_v2();
    } else if (strcmp(argv[5], "v3") == 0) {
        strategy = get_strategy_v3();
    } else if (strcmp(argv[5], "v4") == 0) {
        strategy = get_strategy_v4();
    } else {
        fprintf(stderr, "Error: Unknown strategy '%s'. Use 'v1', 'v2' or 'v3'.\n", argv[5]);
        free(sim);
        return 1;
    }
    sim->strategy = strategy;

    // Initialization
    if (init_common(sim) != 0) {
        free(sim);
        return 1;
    }
    if (sim->strategy.init(sim) != 0) {
        cleanup_common(sim);
        free(sim);
        return 1;
    }

    // Create threads
    pthread_t monitor_thread, watchdog_thread;
    for (int i = 0; i < sim->num_philos; i++) {
        pthread_create(&sim->philos[i].thread, NULL, philo_routine, &sim->philos[i]);
    }
    pthread_create(&monitor_thread, NULL, monitor_routine, sim);
    pthread_create(&watchdog_thread, NULL, watchdog_routine, sim);

    // Wait for threads to finish
    for (int i = 0; i < sim->num_philos; i++) {
        pthread_join(sim->philos[i].thread, NULL);
    }
    pthread_join(monitor_thread, NULL);
    pthread_join(watchdog_thread, NULL);

    // Cleanup
    sim->strategy.cleanup(sim);
    cleanup_common(sim);
    free(sim);

    return 0;
}
