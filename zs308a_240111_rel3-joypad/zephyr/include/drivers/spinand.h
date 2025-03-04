#ifndef __SPINAND__H__
#define __SPINAND__H__

int spinand_storage_ioctl(const struct device *dev, uint8_t cmd, void *buff);

#ifdef CONFIG_SPINAND_TEST_FRAMEWORKS
int spinand_scan_delaychain(const struct device *dev);
#endif

#endif /* __SPINAND__H__ */
