//------------------------------------------------------------------------------
// Headers that are required for printf and mmap
//------------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <pthread.h>

//------------------------------------------------------------------------------
// GPIO ACCES
//------------------------------------------------------------------------------

#define BCM2835_PERIPH_BASE     0x20000000
#define BCM2835_GPIO_BASE       ( BCM2835_PERIPH_BASE + 0x200000 )

#define GPIO_LED0   4
#define GPIO_LED1   17
#define GPIO_BP     18

#define GPIO_FSEL_INPUT  0
#define GPIO_FSEL_OUTPUT 1

struct gpio_s
{
    uint32_t gpfsel[7];
    uint32_t gpset[3];
    uint32_t gpclr[3];
    uint32_t gplev[3];
    uint32_t gpeds[3];
    uint32_t gpren[3];
    uint32_t gpfen[3];
    uint32_t gphen[3];
    uint32_t gplen[3];
    uint32_t gparen[3];
    uint32_t gpafen[3];
    uint32_t gppud[1];
    uint32_t gppudclk[3];
    uint32_t test[1];
};

struct gpio_s *gpio_regs_virt; 


static void 
gpio_fsel(uint32_t pin, uint32_t fun)
{
    uint32_t reg = pin / 10;
    uint32_t bit = (pin % 10) * 3;
    uint32_t mask = 0b111 << bit;
    gpio_regs_virt->gpfsel[reg] = (gpio_regs_virt->gpfsel[reg] & ~mask) | ((fun << bit) & mask);
}

static void 
gpio_write (uint32_t pin, uint32_t val)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;
    if (val == 1) // set
        gpio_regs_virt->gpset[reg] = (1 << bit);
    else // clear
        gpio_regs_virt->gpclr[reg] = (1 << bit);
}

static uint32_t
gpio_read (uint32_t pin)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;
    return (gpio_regs_virt->gplev[reg] << bit) && 1;
}

//------------------------------------------------------------------------------
// Access to memory-mapped I/O
//------------------------------------------------------------------------------

#define RPI_PAGE_SIZE           4096
#define RPI_BLOCK_SIZE          4096

static int mmap_fd;

static int
gpio_mmap ( void ** ptr )
{
    void * mmap_result;

    mmap_fd = open ( "/dev/mem", O_RDWR | O_SYNC );

    if ( mmap_fd < 0 ) {
        return -1;
    }

    mmap_result = mmap (
        NULL
      , RPI_BLOCK_SIZE
      , PROT_READ | PROT_WRITE
      , MAP_SHARED
      , mmap_fd
      , BCM2835_GPIO_BASE );

    if ( mmap_result == MAP_FAILED ) {
        close ( mmap_fd );
        return -1;
    }

    *ptr = mmap_result;

    return 0;
}

void
gpio_munmap ( void * ptr )
{
    munmap ( ptr, RPI_BLOCK_SIZE );
}

//------------------------------------------------------------------------------
// Main Programm
//------------------------------------------------------------------------------

void
delay ( unsigned int milisec )
{
    struct timespec ts, dummy;
    ts.tv_sec  = ( time_t ) milisec / 1000;
    ts.tv_nsec = ( long ) ( milisec % 1000 ) * 1000000;
    nanosleep ( &ts, &dummy );
}

int
bp_thread(void* period_arg) {
    // button
    int period = *((int *) period_arg);
    int bounce_period = 100;
    int old_val = 1; // default button released
    int val = 1;
    int bp_on = 0;
    // led
    gpio_write(GPIO_LED0, 1);

    // programm loop
    while(1){
        old_val = val;
	    val = gpio_read(GPIO_BP);
        if (old_val != val){
            if (val == 0){
                printf("Button pressed\n");
                gpio_write(GPIO_LED0, 1);
                bp_on = 1;
            }
            else {
                printf("Button released\n");
                gpio_write(GPIO_LED0, 0);
                bp_on = 0;
            }
        }

        // waiting bounce period button before take new measure
        // waiting the original period if bounce_period is too short 
        if (bp_on && period > bounce_period) delay(bounce_period);
        else delay(period);
    }

    return 0;
}

int
bp_threadv2(void* period_arg) {
    // button
    int period = *((int *) period_arg);
    int old_val = 1; // default button released
    int val = 1;
    int led_val = 0;

    // led
    gpio_write(GPIO_LED0, 1);

    // programm loop
    while(1){
        old_val = val;
	    val = gpio_read(GPIO_BP);
        if (old_val != val){
            if (val == 0){
                printf("Bouton enfoncé\n");
                gpio_write(GPIO_LED0, 1);
                delay(period*2); // délai supplémentaire, on attend 2*plus longtemps que le délai de base
            }
            else printf("Bouton relaché\n");
        }
        delay(period);
    }

    return 0;
}

int
main ( int argc, char **argv )
{
    // Get args
    // ---------------------------------------------

    int period, half_period;

    period = 20; /* default = 50Hz, 20 ms */
    if ( argc > 1 ) {
        period = atoi ( argv[1] );
    }
    uint32_t volatile * gpio_base = 0;

    // map GPIO registers
    // ---------------------------------------------

    if ( gpio_mmap ( (void **)&gpio_regs_virt ) < 0 ) {
        printf ( "-- error: cannot setup mapped GPIO.\n" );
        exit ( 1 );
    }

    // Setup GPIO of LED0 to output
    // ---------------------------------------------
    
    gpio_fsel(GPIO_LED0, GPIO_FSEL_OUTPUT);
    gpio_fsel(GPIO_BP, GPIO_FSEL_INPUT);

    // Reading button's value
    // ---------------------------------------------

    pthread_t thd;
    if (pthread_create(&thd, NULL, bp_thread, (void*) &period) < 0){
        printf("-- error: cannot create thread.\n");
        exit(1);
    }

    pthread_join(thd, NULL);

    gpio_munmap(gpio_regs_virt);

    return 0;
}
