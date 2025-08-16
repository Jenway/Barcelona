// src/strategy_v4.c
#include "philo.h"
#include <stddef.h>
#include <sys/time.h>

#define MAX_QUEUE 200
#define CRITICAL_THRESHOLD_US 200000 // 0.2 秒，可根据 time_to_die 调整

typedef struct {
    int left;
    int right;
    t_fork_monitor* fm;
} t_fork_ctx;

// FIFO 队列
typedef struct {
    int queue[MAX_QUEUE];
    int head;
    int tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} fifo_queue_t;

static fifo_queue_t g_fifo_queue;

static void fifo_queue_init(void)
{
    g_fifo_queue.head = g_fifo_queue.tail = 0;
    pthread_mutex_init(&g_fifo_queue.mutex, NULL);
    pthread_cond_init(&g_fifo_queue.cond, NULL);
}

static void fifo_queue_destroy(void)
{
    pthread_mutex_destroy(&g_fifo_queue.mutex);
    pthread_cond_destroy(&g_fifo_queue.cond);
}

static void fifo_enqueue(int id)
{
    g_fifo_queue.queue[g_fifo_queue.tail % MAX_QUEUE] = id;
    g_fifo_queue.tail++;
}

// 将哲学家插入队列头，用于优先级提升
static void fifo_enqueue_head(int id)
{
    g_fifo_queue.head--;
    g_fifo_queue.queue[g_fifo_queue.head % MAX_QUEUE] = id;
}

static int fifo_head(void)
{
    return g_fifo_queue.queue[g_fifo_queue.head % MAX_QUEUE];
}

static void fifo_dequeue(void)
{
    g_fifo_queue.head++;
}

// 获取当前时间（微秒）
static long long now_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

static void strategy_v4_take(t_philo* philo)
{
    t_sim* sim = philo->sim;
    t_fork_monitor* fm = &sim->fork_monitor;
    int left = philo->id - 1;
    int right = philo->id % sim->num_philos;

    // 动态优先级：快要饿死的哲学家提升队列头
    long long last_meal_us;
    pthread_mutex_lock(&philo->data_mutex);
    last_meal_us = philo->last_meal_time;
    pthread_mutex_unlock(&philo->data_mutex);

    long long time_to_die_us = (long long)sim->time_to_die * 1000;
    long long remaining_us = time_to_die_us - (now_us() - last_meal_us);

    pthread_mutex_lock(&g_fifo_queue.mutex);
    if (remaining_us < CRITICAL_THRESHOLD_US) {
        fifo_enqueue_head(philo->id);
    } else {
        fifo_enqueue(philo->id);
    }

    while (!sim->stop_simulation && fifo_head() != philo->id) {
        pthread_cond_wait(&g_fifo_queue.cond, &g_fifo_queue.mutex);
    }
    pthread_mutex_unlock(&g_fifo_queue.mutex);

    // 尝试拿叉子
    pthread_mutex_lock(&fm->mutex);
    while (!sim->stop_simulation && (fm->is_taken[left] || fm->is_taken[right])) {
        pthread_cond_wait(&fm->cond, &fm->mutex);
    }
    fm->is_taken[left] = 1;
    fm->is_taken[right] = 1;
    pthread_mutex_unlock(&fm->mutex);

    print_status(philo, "has taken a fork");
    print_status(philo, "has taken a fork");
}

static void strategy_v4_put(t_philo* philo)
{
    t_sim* sim = philo->sim;
    t_fork_monitor* fm = &sim->fork_monitor;
    int left = philo->id - 1;
    int right = philo->id % sim->num_philos;

    // 释放叉子
    pthread_mutex_lock(&fm->mutex);
    fm->is_taken[left] = 0;
    fm->is_taken[right] = 0;
    pthread_cond_broadcast(&fm->cond);
    pthread_mutex_unlock(&fm->mutex);

    // 离开队列，通知下一个哲学家
    pthread_mutex_lock(&g_fifo_queue.mutex);
    fifo_dequeue();
    pthread_cond_broadcast(&g_fifo_queue.cond);
    pthread_mutex_unlock(&g_fifo_queue.mutex);
}

static int strategy_v4_init(t_sim* sim)
{
    pthread_mutex_init(&sim->fork_monitor.mutex, NULL);
    sim->fork_monitor.num_forks = sim->num_philos;
    for (int i = 0; i < sim->num_philos; i++) {
        sim->fork_monitor.is_taken[i] = 0;
    }

    fifo_queue_init();
    return 0;
}

static void strategy_v4_cleanup(t_sim* sim)
{
    pthread_mutex_destroy(&sim->fork_monitor.mutex);
    fifo_queue_destroy();
}

t_philo_strategy get_strategy_v4(void)
{
    t_philo_strategy strategy;
    strategy.init = strategy_v4_init;
    strategy.cleanup = strategy_v4_cleanup;
    strategy.take_forks = strategy_v4_take;
    strategy.put_forks = strategy_v4_put;
    return strategy;
}
