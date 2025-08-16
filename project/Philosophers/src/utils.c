#include "philo.h"

/**
 * @brief Gets the current time in milliseconds since the Epoch.
 * @return The current time in milliseconds.
 */
long get_time_in_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long)(tv.tv_sec * 1000LL + tv.tv_usec / 1000);
}

/**
 * @brief Prints a status message for a philosopher in a thread-safe manner.
 * @param philo The philosopher.
 * @param status The status message to print.
 */
void print_status(t_philo* philo, const char* status)
{
    pthread_mutex_lock(&philo->sim->sim_stop_mutex);
    if (philo->sim->stop_simulation) {
        pthread_mutex_unlock(&philo->sim->sim_stop_mutex);
        return;
    }
    pthread_mutex_lock(&philo->sim->print_mutex);
    printf("%ld %d %s\n", get_time_in_ms() - philo->sim->start_time, philo->id, status);
    pthread_mutex_unlock(&philo->sim->print_mutex);
    pthread_mutex_unlock(&philo->sim->sim_stop_mutex);
}

/**
 * @brief Initializes an event queue.
 * @param q The event queue to initialize.
 */
void queue_init(event_queue_t* q)
{
    q->head = 0;
    q->tail = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

static int queue_empty(event_queue_t* q)
{
    return q->head == q->tail;
}

static int queue_full(event_queue_t* q)
{
    return ((q->tail + 1) % MAX_EVENTS) == q->head;
}

/**
 * @brief Adds an event to the queue. Called by philosophers.
 * @param q The event queue.
 * @param ev The event to add.
 */
void enqueue(event_queue_t* q, philo_event ev)
{
    pthread_mutex_lock(&q->mutex);
    // For simplicity, we block if the queue is full.
    while (queue_full(q)) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    q->buf[q->tail] = ev;
    q->tail = (q->tail + 1) % MAX_EVENTS;
    pthread_cond_signal(&q->cond); // Signal that an item is available
    pthread_mutex_unlock(&q->mutex);
}

/**
 * @brief Removes and returns an event from the queue. Called by the monitor.
 * @param q The event queue.
 * @return The dequeued event.
 */
philo_event dequeue(event_queue_t* q)
{
    pthread_mutex_lock(&q->mutex);
    while (queue_empty(q)) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    philo_event ev = q->buf[q->head];
    q->head = (q->head + 1) % MAX_EVENTS;
    pthread_cond_signal(&q->cond); // Signal that space is available
    pthread_mutex_unlock(&q->mutex);
    return ev;
}

/**
 * @brief Parses command-line arguments to initialize the simulation struct.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return A pointer to a newly allocated t_sim struct, or NULL on error.
 */
t_sim* parse_args(int argc, char** argv)
{
    // Now requires a 6th or 7th argument for strategy
    if (argc < 6 || argc > 7) {
        fprintf(stderr, "Usage: %s num_philos time_to_die time_to_eat time_to_sleep <strategy> [num_must_eat]\n", argv[0]);
        fprintf(stderr, "Available strategies: v1, v2, v3\n");
        return NULL;
    }
    t_sim* sim = malloc(sizeof(t_sim));
    if (!sim) {
        perror("Failed to allocate memory for simulation");
        return NULL;
    }
    memset(sim, 0, sizeof(t_sim));

    sim->num_philos = atoi(argv[1]);
    if (sim->num_philos <= 0 || sim->num_philos > MAX_PHILOS) {
        fprintf(stderr, "num_philos must be between 1 and %d\n", MAX_PHILOS);
        free(sim);
        return NULL;
    }
    sim->time_to_die = atol(argv[2]);
    sim->time_to_eat = atol(argv[3]);
    sim->time_to_sleep = atol(argv[4]);
    sim->num_must_eat = (argc == 7) ? atoi(argv[6]) : -1;
    sim->start_time = get_time_in_ms();
    return sim;
}
