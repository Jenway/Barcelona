#include "philo.h"

// --- Static Helper Functions for Routines ---

static void notify_monitor(t_philo* philo, philo_event_type type)
{
    enqueue(&philo->sim->event_queue, (philo_event) { philo->id, type });
}

static void philo_act_think(t_philo* philo)
{
    print_status(philo, "is thinking");
}

static void philo_act_eat(t_philo* philo)
{
    print_status(philo, "is eating");

    pthread_mutex_lock(&philo->data_mutex);
    philo->last_meal_time = get_time_in_ms();
    philo->meal_count++;
    // Update the deadline for the watchdog
    philo->sim->next_deadline_ms[philo->id - 1] = philo->last_meal_time + philo->sim->time_to_die;
    pthread_mutex_unlock(&philo->data_mutex);

    notify_monitor(philo, EVENT_EAT);

    if (philo->sim->num_must_eat != -1 && philo->meal_count >= philo->sim->num_must_eat) {
        notify_monitor(philo, EVENT_FINISHED);
    }

    usleep(philo->sim->time_to_eat * 1000);
}

static void philo_act_sleep(t_philo* philo)
{
    print_status(philo, "is sleeping");
    usleep(philo->sim->time_to_sleep * 1000);
}

// --- Thread Routines ---

void* philo_routine(void* arg)
{
    t_philo* philo = (t_philo*)arg;
    t_sim* sim = philo->sim;

    // Stagger start times to avoid initial resource contention
    if (philo->id % 2 == 0) {
        usleep(sim->time_to_eat * 500);
    }

    while (1) {
        pthread_mutex_lock(&sim->sim_stop_mutex);
        if (sim->stop_simulation) {
            pthread_mutex_unlock(&sim->sim_stop_mutex);
            break;
        }
        pthread_mutex_unlock(&sim->sim_stop_mutex);

        philo_act_think(philo);

        sim->strategy.take_forks(philo);

        // Re-check stop condition after acquiring forks, as it might have taken time
        pthread_mutex_lock(&sim->sim_stop_mutex);
        if (sim->stop_simulation) {
            pthread_mutex_unlock(&sim->sim_stop_mutex);
            sim->strategy.put_forks(philo);
            break;
        }
        pthread_mutex_unlock(&sim->sim_stop_mutex);

        philo_act_eat(philo);
        sim->strategy.put_forks(philo);

        philo_act_sleep(philo);
    }
    return NULL;
}

void* watchdog_routine(void* arg)
{
    t_sim* sim = (t_sim*)arg;

    while (1) {
        pthread_mutex_lock(&sim->sim_stop_mutex);
        if (sim->stop_simulation) {
            pthread_mutex_unlock(&sim->sim_stop_mutex);
            break;
        }
        pthread_mutex_unlock(&sim->sim_stop_mutex);

        long current_time = get_time_in_ms();
        for (int i = 0; i < sim->num_philos; i++) {
            pthread_mutex_lock(&sim->philos[i].data_mutex);
            long deadline = sim->next_deadline_ms[i];
            int meal_count = sim->philos[i].meal_count;
            pthread_mutex_unlock(&sim->philos[i].data_mutex);

            // If the philosopher has finished eating, they can't die.
            if (sim->num_must_eat != -1 && meal_count >= sim->num_must_eat) {
                continue;
            }

            if (current_time > deadline) {
                notify_monitor(sim->philos + i, EVENT_DIED);
                return NULL;
            }
        }
        usleep(1000);
    }
    return NULL;
}

void* monitor_routine(void* arg)
{
    t_sim* sim = (t_sim*)arg;
    int finished_count = 0;

    while (1) {
        philo_event ev = dequeue(&sim->event_queue);

        pthread_mutex_lock(&sim->sim_stop_mutex);
        if (sim->stop_simulation) {
            pthread_mutex_unlock(&sim->sim_stop_mutex);
            // Drain queue but do nothing if simulation is already stopping
            continue;
        }
        pthread_mutex_unlock(&sim->sim_stop_mutex);

        switch (ev.type) {
        case EVENT_EAT:
            // This event is primarily for statistics if needed.
            break;
        case EVENT_DIED:
            pthread_mutex_lock(&sim->sim_stop_mutex);
            sim->stop_simulation = true;
            pthread_mutex_unlock(&sim->sim_stop_mutex);

            pthread_mutex_lock(&sim->print_mutex);
            printf("%ld %d died\n", get_time_in_ms() - sim->start_time, ev.philo_id);
            pthread_mutex_unlock(&sim->print_mutex);
            return NULL;
        case EVENT_FINISHED:
            finished_count++;
            if (finished_count >= sim->num_philos) {
                pthread_mutex_lock(&sim->sim_stop_mutex);
                sim->stop_simulation = true;
                pthread_mutex_unlock(&sim->sim_stop_mutex);

                pthread_mutex_lock(&sim->print_mutex);
                printf("All philosophers have eaten %d times.\n", sim->num_must_eat);
                pthread_mutex_unlock(&sim->print_mutex);
                return NULL;
            }
            break;
        }
    }
    return NULL;
}
