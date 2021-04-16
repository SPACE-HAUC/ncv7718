#define NCV7718_PRIVATE
#include "ncv7718.h"
#undef NCV7718_PRIVATE
#include <stdio.h>
#include <string.h>

#define eprintf(str, ...)                               \
    fprintf(stderr, "%s: " str "\n", __func__, ##__VA_ARGS__); \
    fflush(stderr);

int ncv7718_init(ncv7718 *dev, int bus, int cs, int gpiocs)
{
    if (dev == NULL)
    {
        eprintf("Fatal error, memory for struct not allocated");
        return NCV7718_FAILURE;
    }
    if (dev->bus == NULL)
    {
        eprintf("Fatal error, %p is null in %p for device, exiting", dev->bus, dev);
        return NCV7718_FAILURE;
    }
    memset(dev->bus, 0x0, sizeof(spibus));
    dev->bus->bus = bus;
    dev->bus->cs = cs;
    if (gpiocs > 0)
    {
        dev->bus->cs_gpio = gpiocs;
        dev->bus->cs_internal = CS_EXTERNAL;
    }
    else
    {
        dev->bus->cs_gpio = -1;
        dev->bus->cs_internal = CS_INTERNAL;
    }
    dev->bus->lsb = 0;
    dev->bus->mode = SPI_MODE_1;
    dev->bus->bits = 8;
    dev->bus->speed = 0;
    dev->bus->sleeplen = 0;
    dev->bus->internal_rotation = true;
    int status = spibus_init(dev->bus);
    if (status < 0)
    {
        eprintf("Error %d initializing SPI Bus", status);
        return NCV7718_FAILURE;
    }
    return ncv7718_por(dev);
}

void ncv7718_destroy(ncv7718 *dev)
{
    if (dev == NULL)
    {
        eprintf("Could not close device at %p", dev);
        return;
    }
    dev->out_conf = 0x0; // turn off all switches
    dev->out_en = 0x0;   // turn off all enables
    if (ncv7718_por(dev) < 0)
    {
        eprintf("Could not turn off outputs");
    }
    spibus_destroy(dev->bus);
    return;
}

int ncv7718_set_output(ncv7718 *dev, int axis, int direction)
{
    if (axis < 0 || axis > 3)
    {
        eprintf("Invalid axis limit: %d out of range", axis);
        return NCV7718_FAILURE;
    }
    axis <<= 1; // double it

    dev->out_en |= ((uint8_t)(0x3 << axis)); // enable channels
    dev->out_conf &= ~((uint8_t)(0x3 << axis)); // unset output mode
    if (direction == 1)
        dev->out_conf |= (uint8_t)(0x1 << axis); // 1 on, 2 off etc
    else if (direction == -1)
        dev->out_conf |= (uint8_t)(0x2 << axis); // 1 off, 2 on etc
    else if (direction != 0)
    {
        eprintf("Invalid direction: %d", direction);
        return NCV7718_FAILURE;
    }
    return NCV7718_SUCCESS;
}

NCV7718_RETCODE ncv7718_por(ncv7718 *dev)
{
    ncv7718_cmd cmd[1];
    memset(cmd, 0x0, sizeof(ncv7718_cmd));
    cmd->srr = 1;
    if (spibus_xfer(dev->bus, &(cmd->data), sizeof(ncv7718_cmd)) < 0)
    {
        eprintf("Could not send command 0x%04X\n", cmd->data);
        return NCV7718_FAILURE;
    }
    return NCV7718_SUCCESS;
}

NCV7718_RETCODE ncv7718_exec_output(ncv7718 *dev)
{
    ncv7718_cmd cmd[1];
    ncv7718_data data[1];
    memset(cmd, 0x0, sizeof(ncv7718_cmd));
    memset(data, 0x0, sizeof(ncv7718_data));
    // write configuration
    cmd->hben = dev->out_en;
    cmd->hbcnf = dev->out_conf;
    // transfer once
    if (spibus_xfer(dev->bus, &(cmd->data), sizeof(ncv7718_cmd)) < 0)
    {
        eprintf("Could not send command 0x%04X\n", cmd->data);
        return NCV7718_FAILURE;
    }
    if (spibus_xfer_full(dev->bus, &(data->data), sizeof(ncv7718_data), &(cmd->data), sizeof(ncv7718_cmd)) < 0)
    {
        eprintf("Could not confirm command 0x%04X\n", cmd->data);
        return NCV7718_FAILURE;
    }
    // check for errors
    if (data->hbcr != cmd->hbcnf)
    {
        eprintf("HB Configuration does not match: Out -> 0x%04X | In -> 0x%04X", cmd->data, data->data);
        return NCV7718_FAILURE;
    }
    // check for overload
    if (data->tw)
    {
        eprintf("Thermal warning!");
        return NCV7718_THERMAL_WARNING;
    }
    if (data->psf)
    {
        eprintf("Power supply failure!");
        return NCV7718_PSU_FAIL;
    }
    if (data->ocs)
    {
        eprintf("Over current shutoff");
        return NCV7718_OC_SHUTOFF;
    }
    if (data->uld)
    {
        eprintf("Under load detection");
        return NCV7718_UNDER_LOAD;
    }
#ifdef NCV7718_DEBUG
    eprintf("Command: 0x%04x | Result: 0x%04x", cmd->data, data->data);
#endif
    return NCV7718_SUCCESS;
}

#ifdef NCV7718_UNIT_TEST
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
volatile sig_atomic_t done = 0;
void sighandler(int sig)
{
    done = 1;
}
int main(int argc, char *argv[])
{
    if (!(argc == 3 || argc == 4))
    {
        printf("Usage: ncv7718 <SPI Bus> <SPI CS> [<Optional GPIO CS>]\n\n");
        return 0;
    }
    int bus = atoi(argv[1]);
    int cs = atoi(argv[2]);
    int gpiocs = -1;
    if (argc == 4)
        gpiocs = atoi(argv[3]);
    signal(SIGINT, &sighandler);
    ncv7718 hb[1];
    if (ncv7718_init(hb, bus, cs, gpiocs) != NCV7718_SUCCESS)
    {
        eprintf("Init failed!");
        return 0;
    }
    while(!done)
    {
        ncv7718_set_output(hb, 0, 1);
        ncv7718_exec_output(hb);
        sleep(1);
        ncv7718_set_output(hb, 0, 0);
        ncv7718_exec_output(hb);
        sleep(1);
    }
    ncv7718_destroy(hb);
    return 0;
}
#endif
