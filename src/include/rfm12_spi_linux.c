#ifdef __PLATFORM_LINUX__
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <byteswap.h>
#include <unistd.h>

static const char *device = "/dev/spidev0.1";
static int fd;
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 250000;

static void pabort(const char *s)
{
	perror(s);
	abort();
}


static void spi_init(void) {
	int ret = 0;

	fd = open(device, O_RDWR);

	if (fd < 0) {
			pabort("driver not loaded? can't open device");
	}
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
		if (ret == -1)
			pabort("can't set spi mode");

		ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
		if (ret == -1)
			pabort("can't get spi mode");

		/*
		 * bits per word
		 */
		ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
		if (ret == -1)
			pabort("can't set bits per word");

		ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
		if (ret == -1)
			pabort("can't get bits per word");

		/*
		 * max speed hz
		 */
		ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
		if (ret == -1)
			pabort("can't set max speed hz");

		ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
		if (ret == -1)
			pabort("can't get max speed hz");
}

static uint16_t rfm12_read(uint16_t address) {
	int ret;

	uint8_t rx[2];
	uint8_t tx[2];

	tx[0] = (uint8_t) (address >> 8);
	tx[1] = (uint8_t) (address & 0xFF);

	rx[0] = 0;
	rx[1] = 0;

	struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx,
			.rx_buf = (unsigned long)rx,
			.len = 2,
			.delay_usecs = 0,
			.speed_hz = speed,
			.bits_per_word = bits,
		};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	if (ret < 1)
			pabort("can't send spi message");


	return ((uint16_t)rx[0] << 8) + rx[1];
}

static void rfm12_data(uint16_t d) {
	uint8_t buffer[2];
	buffer[0] = (uint8_t) (d >> 8);
	buffer[1] = (uint8_t) (d & 0xff);

	write(fd, buffer, sizeof(buffer));
}

static uint8_t rfm12_read_int_flags_inline(void) {
	uint16_t ret = rfm12_read(0x0000);

	return ret >> 8;
}

#endif
