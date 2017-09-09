/*
Copyright (c) 2012-2015 Ben Croston

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "c_gpio.h"


#define SUNXI_GPIO_BASE  (0x01C20000)
#define SUNXI_GPIO_BASE_OFFSET 0x800 // GPIO really starts at offset SUNXI_GPIO_BASE + 0x800

#define BCM2708_PERI_BASE_DEFAULT   0x20000000
#define BCM2709_PERI_BASE_DEFAULT   0x3f000000
#define GPIO_BASE_OFFSET            0x200000
#define FSEL_OFFSET                 0   // 0x0000
#define SET_OFFSET                  7   // 0x001c / 4
#define CLR_OFFSET                  10  // 0x0028 / 4
#define PINLEVEL_OFFSET             13  // 0x0034 / 4
#define EVENT_DETECT_OFFSET         16  // 0x0040 / 4
#define RISING_ED_OFFSET            19  // 0x004c / 4
#define FALLING_ED_OFFSET           22  // 0x0058 / 4
#define HIGH_DETECT_OFFSET          25  // 0x0064 / 4
#define LOW_DETECT_OFFSET           28  // 0x0070 / 4
#define PULLUPDN_OFFSET             37  // 0x0094 / 4
#define PULLUPDNCLK_OFFSET          38  // 0x0098 / 4

#define PAGE_SIZE  (4*1024)
#define BLOCK_SIZE (4*1024)

#define MAP_SIZE (4096*2)
#define MAP_MASK (MAP_SIZE - 1)

static volatile uint32_t *gpio_map;

/* Nanopi mask pins available by banks */
static const int nanopi_PIN_MASK[9][32] = //[BANK]  [INDEX]
{
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,}, //PA
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,}, //PB
    { 0, 1, 2, 3, -1, -1, -1, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,}, //PC
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,}, //PD
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,}, //PE
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,}, //PF
    {-1, -1, -1, -1, -1, -1, 6, 7, 8, 9, -1, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,}, //PG
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,}, //PH
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,}, //PI
};



/** Read a word from GPIO base + 'addr'. */
static uint32_t readl(uint32_t addr) {
    uint32_t mmap_base = addr & MAP_MASK;
    uint32_t mmap_seek = (mmap_base + SUNXI_GPIO_BASE_OFFSET);
    return gpio_map[mmap_seek >> 2];

}

/** Write a word to GPIO base + 'addr'. */
static void writel(uint32_t val, uint32_t addr) {
    uint32_t mmap_base = addr & MAP_MASK;
    uint32_t mmap_seek = (mmap_base + SUNXI_GPIO_BASE_OFFSET);
    gpio_map[mmap_seek >> 2] = val;
}

void short_wait(void)
{
    int i;

    for (i=0; i<150; i++) {    // wait 150 cycles
        asm volatile("nop");
    }
}

int setup(void)
{
    int mem_fd;
    uint32_t gpio_base;
#if 0
    uint8_t *gpio_mem;
    uint32_t peri_base;
    unsigned char buf[4];
    FILE *fp;
    char buffer[1024];
    char hardware[1024];
    int found = 0;
#endif
    // try /dev/gpiomem first - this does not require root privs
    if ((mem_fd = open("/dev/gpiomem", O_RDWR|O_SYNC)) > 0)
    {
        gpio_map = (uint32_t *)mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, 0);
        if (gpio_map == ((void*)-1)) {
            return SETUP_MMAP_FAIL;
        } else {
            return SETUP_OK;
        }
    }

    // revert to /dev/mem method - requires root
#if 0
    // determine peri_base
    if ((fp = fopen("/proc/device-tree/soc/ranges", "rb")) != NULL) {
        // get peri base from device tree
        fseek(fp, 4, SEEK_SET);
        if (fread(buf, 1, sizeof buf, fp) == sizeof buf) {
            peri_base = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] << 0;
        }
        fclose(fp);
    } else {
        // guess peri base based on /proc/cpuinfo hardware field
        if ((fp = fopen("/proc/cpuinfo", "r")) == NULL)
            return SETUP_CPUINFO_FAIL;

        while(!feof(fp) && !found) {
            fgets(buffer, sizeof(buffer), fp);
            sscanf(buffer, "Hardware	: %s", hardware);
            if (strcmp(hardware, "BCM2708") == 0 || strcmp(hardware, "BCM2835") == 0) {
                // pi 1 hardware
                peri_base = BCM2708_PERI_BASE_DEFAULT;
                found = 1;
            } else if (strcmp(hardware, "BCM2709") == 0 || strcmp(hardware, "BCM2836") == 0) {
                // pi 2 hardware
                peri_base = BCM2709_PERI_BASE_DEFAULT;
                found = 1;
            }
        }
        fclose(fp);
        if (!found)
            return SETUP_NOT_RPI_FAIL;
    }

    gpio_base = peri_base + GPIO_BASE_OFFSET;
#else
    gpio_base = SUNXI_GPIO_BASE;
#endif
    // mmap the GPIO memory registers
     if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
         return SETUP_DEVMEM_FAIL;

    gpio_map = (uint32_t *)mmap( NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, gpio_base);

    if (gpio_map == ((void*) -1))
        return SETUP_MMAP_FAIL;

    return SETUP_OK;
}


void set_pullupdn(int gpio, int pud)
{
#if 0
    int clk_offset = PULLUPDNCLK_OFFSET + (gpio/32);
    int shift = (gpio%32);

    if (pud == PUD_DOWN)
        *(gpio_map+PULLUPDN_OFFSET) = (*(gpio_map+PULLUPDN_OFFSET) & ~3) | PUD_DOWN;
    else if (pud == PUD_UP)
        *(gpio_map+PULLUPDN_OFFSET) = (*(gpio_map+PULLUPDN_OFFSET) & ~3) | PUD_UP;
    else  // pud == PUD_OFF
        *(gpio_map+PULLUPDN_OFFSET) &= ~3;

    short_wait();
    *(gpio_map+clk_offset) = 1 << shift;
    short_wait();
    *(gpio_map+PULLUPDN_OFFSET) &= ~3;
    *(gpio_map+clk_offset) = 0;
#else
    uint32_t regval = 0;
    int bank = gpio >> 5;
    int index = gpio & 0x1F;
    int sub = index >> 4;
    int sub_index = index - 16 * sub;
    int offset = bank * 36 + 0x1c + sub * 4; // +0x10 -> pullUpDn reg
    if (nanopi_PIN_MASK[bank][index] != -1)
    {
        regval = readl(offset);
        if(pud == PUD_OFF)
        {
            regval &= ~(3 << (sub_index << 1));
        }
        else
        if( pud == PUD_UP )
        {
            regval &= ~(3 << (sub_index << 1));
            regval |= (1 << (sub_index << 1));
        }
        else
        if( pud == PUD_DOWN )
        {
            regval &= ~(3 << (sub_index << 1));
            regval |= (2 << (sub_index << 1));
        }
        else
        {
            printf("set_pullupdn: pud number error %d\n", pud);
        }
        writel(regval, offset);
    }
    else
    {
        printf("set_pullupdn: pin number error %d\n", gpio);
    }
#endif
}


void setup_gpio(int gpio, int direction, int pud)
{
#if 0
    int offset = FSEL_OFFSET + (gpio/10);
    int shift = (gpio%10)*3;

    set_pullupdn(gpio, pud);
    if (direction == OUTPUT)
        *(gpio_map+offset) = (*(gpio_map+offset) & ~(7<<shift)) | (1<<shift);
    else  // direction == INPUT
        *(gpio_map+offset) = (*(gpio_map+offset) & ~(7<<shift));
#else
    int regval = 0;
    int bank = gpio >> 5;
    int index = gpio & 0x1F;
    int offset = bank * 36 + ((index >> 3) << 2);
    int val_offset = ((index - ((index >> 3) << 3)) << 2);
    if (nanopi_PIN_MASK[bank][index] != -1)
    {
        regval = readl(offset);
        if(direction == OUTPUT)
        {
            regval &= ~(7 << val_offset);
            regval |= (1 << val_offset);
        }
        else
        if( direction == INPUT )
        {
            regval &= ~(7 << val_offset);
        }
        else
        {
            printf("setup_gpio: direction number error %d\n", direction);
        }
        writel(regval, offset);
    }
    else
    {
        printf("setup_gpio: pin number error %d\n", gpio);
    }
#endif
}


// Contribution by Eric Ptak <trouch@trouch.com>
int gpio_function(int gpio)
{
#if 0
    int offset = FSEL_OFFSET + (gpio/10);
    int shift = (gpio%10)*3;
    int value = *(gpio_map+offset);
    value >>= shift;
    value &= 7;
    return value; // 0=input, 1=output, 4=alt0
#else
    int regval = 0;
    int bank = gpio >> 5;
    int index = gpio & 0x1F ;
    int offset = bank * 36 + ((index >> 3) << 2);
    int val_offset = ((index - ((index >> 3) << 3)) << 2);
    if (nanopi_PIN_MASK[bank][index] != -1)
    {
        regval = (readl(offset) >> val_offset) & 0x07;
    }
    else
    {
        printf("gpio_function: pin number error %d\n", gpio);
    }
    return regval;
#endif
}


void output_gpio(int gpio, int value)
{
#if 0
    int offset, shift;

    if (value) // value == HIGH
        offset = SET_OFFSET + (gpio/32);
    else       // value == LOW
       offset = CLR_OFFSET + (gpio/32);

    shift = (gpio%32);

    *(gpio_map+offset) = 1 << shift;
#else
    int regval = 0;
    int bank = gpio >> 5;
    int offset = bank * 36 + 0x10; // +0x10 -> data reg
    int index = gpio & 0x1F ;
    int mask = 1 << index;
    if (nanopi_PIN_MASK[bank][index] != -1)
    {
        regval = readl(offset);
        regval = value ? (regval | mask) : (regval & ~mask);
        writel(regval, offset);
    }
    else
    {
        printf("output_gpio: pin number error %d\n", gpio);
    }
#endif
}

int input_gpio(int gpio)
{
#if 0
   int offset, value, mask;

   offset = PINLEVEL_OFFSET + (gpio/32);
   mask = (1 << gpio%32);
   value = *(gpio_map+offset) & mask;
   return value;
#else
    int regval = 0;
    int bank = gpio >> 5;
    int offset = bank * 36 + 0x10; // +0x10 -> data reg
    int index = gpio & 0x1F ;
    int mask = 1 << index;
    if (nanopi_PIN_MASK[bank][index] != -1) {
        regval = readl(offset) & mask;
    } else {
        printf("input_gpio: pin number error %d\n", gpio);
    }
    return regval;
#endif
}

void cleanup(void)
{
    munmap((void *)gpio_map, BLOCK_SIZE);
}
