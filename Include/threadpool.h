#ifndef _threadpool_h
#define _threadpool_h

typedef struct threadpool_t threadpool_t;

/**
 * @brief a 创建线程池
 * @param thr_num  最小线程数
 * @param max_thr_num  最大线程数
 * @param queue_max_size   任务队列最大数量.
 * @return 返回一个线程池指针, 失败返回NULL
 */
threadpool_t *threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size);

/**
 * @brief 添加一个新任务到线程池中
 * @param pool     线程池地址
 * @param function 任务将要执行的函数指针
 * @param argument 任务参数.
 * @return 返回0, 失败返回 -1
 */
int threadpool_add(threadpool_t *pool, void*(*function)(void *arg), void *arg);

/**
 * @brief 停止并销毁线程池
 * @param pool  需要销毁的线程池
 * @return 返回0, 失败返回-1
 */
int threadpool_destroy(threadpool_t *pool);

 /**
 * @brief 获取当前线程数
 * @param 线程池
 * @return 当前线程数
 */
int threadpool_all_threadnum(threadpool_t *pool);

 /**
 * @brief 获取当前忙线程数
 * @param 线程池
 * @return 获取当前忙线程数
 */
int threadpool_busy_threadnum(threadpool_t *pool);

#endif
