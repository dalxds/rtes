// LIBRARIES
#include <pthread.h>

// STRUCTS
typedef struct rwlock_tag {
    pthread_mutex_t     mutex;
    pthread_cond_t      read;           /* wait for read */
    pthread_cond_t      write;          /* wait for write */
    int                 valid;          /* set when valid */
    int                 r_active;       /* readers active */
    int                 w_active;       /* writer active */
    int                 r_wait;         /* readers waiting */
    int                 w_wait;         /* writers waiting */
} rwlock_t;

// GLOBAL VARIABLES & CONSTANTS
#define RWLOCK_VALID    0xfacade


// Support static initialization of barriers
#define RWL_INITIALIZER \
    {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, \
    PTHREAD_COND_INITIALIZER, RWLOCK_VALID, 0, 0, 0, 0}

/********************************************//**
 *  MAIN API FUNCTIONS
 ***********************************************/
/// @brief  Initialize an RW Lock
/// @param  rwl An rwl lock handler (rwlock_t)
/// @return 0   Success
/// @return !0  Error No
extern int rwl_init (rwlock_t *rwlock);

/// @brief  Destroy an RW Lock
/// @param  rwl An rwl lock handler (rwlock_t)
/// @return 0   Success
/// @return !0  Error No
extern int rwl_destroy (rwlock_t *rwlock);

/// @brief  Read Lock an RW Lock
/// @param  rwl An rwl lock handler (rwlock_t)
/// @return 0   Success
/// @return !0  Error No
extern int rwl_readlock (rwlock_t *rwlock);

/// @brief  Read Unlock an RW Lock
/// @param  rwl An rwl lock handler (rwlock_t)
/// @return 0   Success
/// @return !0  Error No
extern int rwl_readunlock (rwlock_t *rwlock);

/// @brief  Read Unlock an RW Lock
/// @param  rwl An rwl lock handler (rwlock_t)
/// @return 0   Success
/// @return !0  Error No
extern int rwl_writelock (rwlock_t *rwlock);

/// @brief  Write Unlock an RW Lock
/// @param  rwl An rwl lock handler (rwlock_t)
/// @return 0   Success
/// @return !0  Error No
extern int rwl_writeunlock (rwlock_t *rwlock);

/********************************************//**
 *  ERROR HANDLERS
 ***********************************************/
#define err_abort(code,text) do { \
    fprintf (stderr, "%s at \"%s\":%d: %s\n", \
        text, __FILE__, __LINE__, strerror (code)); \
    abort (); \
    } while (0)
#define errno_abort(text) do { \
    fprintf (stderr, "%s at \"%s\":%d: %s\n", \
        text, __FILE__, __LINE__, strerror (errno)); \
    abort (); \
    } while (0)