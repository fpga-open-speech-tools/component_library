// SPDX-License-Identifier: GPL-2.0+
/** @file FE_pFIR_DPRAM.c
    
    This kernel driver controls a Dual Port RAM block that interfaces with a programmable FIR filter
    
    @author Joshua Harthan (based on code written by Tyler Davis)
    @date 2020
    @copyright 2020 FlatEarth Inc, Bozeman MT
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include "custom_functions.h" // fixed point operations

/* Header for successful compilation */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joshua Harthan");
MODULE_DESCRIPTION("Loadable kernel module for Dual Port RAM, interfaceable with a programmable FIR filter.");
MODULE_VERSION("1.0");

/* Structure necessary for fixed point number reperesentation */
struct fixed_num {
  int integer;
  int fraction;
  int fraction_len;
};

/* Register offsets of the DPRAM component */
#define REGISTER0_OFFSET 0x0
#define REGISTER1_OFFSET 0x1
#define REGISTER2_OFFSET 0x2
#define REGISTER3_OFFSET 0x3
#define REGISTER4_OFFSET 0x4
#define REGISTER5_OFFSET 0x5
#define REGISTER6_OFFSET 0x6
#define REGISTER7_OFFSET 0x7
#define REGISTER8_OFFSET 0x8
#define REGISTER9_OFFSET 0x9
#define REGISTER10_OFFSET 0xa
#define REGISTER11_OFFSET 0xb
#define REGISTER12_OFFSET 0xc
#define REGISTER13_OFFSET 0xd
#define REGISTER14_OFFSET 0xe
#define REGISTER15_OFFSET 0xf
#define REGISTER16_OFFSET 0x10
#define REGISTER17_OFFSET 0x11
#define REGISTER18_OFFSET 0x12
#define REGISTER19_OFFSET 0x13
#define REGISTER20_OFFSET 0x14
#define REGISTER21_OFFSET 0x15
#define REGISTER22_OFFSET 0x16
#define REGISTER23_OFFSET 0x17
#define REGISTER24_OFFSET 0x18
#define REGISTER25_OFFSET 0x19
#define REGISTER26_OFFSET 0x1a
#define REGISTER27_OFFSET 0x1b
#define REGISTER28_OFFSET 0x1c
#define REGISTER29_OFFSET 0x1d
#define REGISTER30_OFFSET 0x1e
#define REGISTER31_OFFSET 0x1f
#define REGISTER32_OFFSET 0x20
#define REGISTER33_OFFSET 0x21
#define REGISTER34_OFFSET 0x22
#define REGISTER35_OFFSET 0x23
#define REGISTER36_OFFSET 0x24
#define REGISTER37_OFFSET 0x25
#define REGISTER38_OFFSET 0x26
#define REGISTER39_OFFSET 0x27
#define REGISTER40_OFFSET 0x28
#define REGISTER41_OFFSET 0x29
#define REGISTER42_OFFSET 0x2a
#define REGISTER43_OFFSET 0x2b
#define REGISTER44_OFFSET 0x2c
#define REGISTER45_OFFSET 0x2d
#define REGISTER46_OFFSET 0x2e
#define REGISTER47_OFFSET 0x2f
#define REGISTER48_OFFSET 0x30
#define REGISTER49_OFFSET 0x31
#define REGISTER50_OFFSET 0x32
#define REGISTER51_OFFSET 0x33
#define REGISTER52_OFFSET 0x34
#define REGISTER53_OFFSET 0x35
#define REGISTER54_OFFSET 0x36
#define REGISTER55_OFFSET 0x37
#define REGISTER56_OFFSET 0x38
#define REGISTER57_OFFSET 0x39
#define REGISTER58_OFFSET 0x3a
#define REGISTER59_OFFSET 0x3b
#define REGISTER60_OFFSET 0x3c
#define REGISTER61_OFFSET 0x3d
#define REGISTER62_OFFSET 0x3e
#define REGISTER63_OFFSET 0x3f
#define REGISTER64_OFFSET 0x40
#define REGISTER65_OFFSET 0x41
#define REGISTER66_OFFSET 0x42
#define REGISTER67_OFFSET 0x43
#define REGISTER68_OFFSET 0x44
#define REGISTER69_OFFSET 0x45
#define REGISTER70_OFFSET 0x46
#define REGISTER71_OFFSET 0x47
#define REGISTER72_OFFSET 0x48
#define REGISTER73_OFFSET 0x49
#define REGISTER74_OFFSET 0x4a
#define REGISTER75_OFFSET 0x4b
#define REGISTER76_OFFSET 0x4c
#define REGISTER77_OFFSET 0x4d
#define REGISTER78_OFFSET 0x4e
#define REGISTER79_OFFSET 0x4f
#define REGISTER80_OFFSET 0x50
#define REGISTER81_OFFSET 0x51
#define REGISTER82_OFFSET 0x52
#define REGISTER83_OFFSET 0x53
#define REGISTER84_OFFSET 0x54
#define REGISTER85_OFFSET 0x55
#define REGISTER86_OFFSET 0x56
#define REGISTER87_OFFSET 0x57
#define REGISTER88_OFFSET 0x58
#define REGISTER89_OFFSET 0x59
#define REGISTER90_OFFSET 0x5a
#define REGISTER91_OFFSET 0x5b
#define REGISTER92_OFFSET 0x5c
#define REGISTER93_OFFSET 0x5d
#define REGISTER94_OFFSET 0x5e
#define REGISTER95_OFFSET 0x5f
#define REGISTER96_OFFSET 0x60
#define REGISTER97_OFFSET 0x61
#define REGISTER98_OFFSET 0x62
#define REGISTER99_OFFSET 0x63
#define REGISTER100_OFFSET 0x64
#define REGISTER101_OFFSET 0x65
#define REGISTER102_OFFSET 0x66
#define REGISTER103_OFFSET 0x67
#define REGISTER104_OFFSET 0x68
#define REGISTER105_OFFSET 0x69
#define REGISTER106_OFFSET 0x6a
#define REGISTER107_OFFSET 0x6b
#define REGISTER108_OFFSET 0x6c
#define REGISTER109_OFFSET 0x6d
#define REGISTER110_OFFSET 0x6e
#define REGISTER111_OFFSET 0x6f
#define REGISTER112_OFFSET 0x70
#define REGISTER113_OFFSET 0x71
#define REGISTER114_OFFSET 0x72
#define REGISTER115_OFFSET 0x73
#define REGISTER116_OFFSET 0x74
#define REGISTER117_OFFSET 0x75
#define REGISTER118_OFFSET 0x76
#define REGISTER119_OFFSET 0x77
#define REGISTER120_OFFSET 0x78
#define REGISTER121_OFFSET 0x79
#define REGISTER122_OFFSET 0x7a
#define REGISTER123_OFFSET 0x7b
#define REGISTER124_OFFSET 0x7c
#define REGISTER125_OFFSET 0x7d
#define REGISTER126_OFFSET 0x7e
#define REGISTER127_OFFSET 0x7f
#define REGISTER128_OFFSET 0x80
#define REGISTER129_OFFSET 0x81
#define REGISTER130_OFFSET 0x82
#define REGISTER131_OFFSET 0x83
#define REGISTER132_OFFSET 0x84
#define REGISTER133_OFFSET 0x85
#define REGISTER134_OFFSET 0x86
#define REGISTER135_OFFSET 0x87
#define REGISTER136_OFFSET 0x88
#define REGISTER137_OFFSET 0x89
#define REGISTER138_OFFSET 0x8a
#define REGISTER139_OFFSET 0x8b
#define REGISTER140_OFFSET 0x8c
#define REGISTER141_OFFSET 0x8d
#define REGISTER142_OFFSET 0x8e
#define REGISTER143_OFFSET 0x8f
#define REGISTER144_OFFSET 0x90
#define REGISTER145_OFFSET 0x91
#define REGISTER146_OFFSET 0x92
#define REGISTER147_OFFSET 0x93
#define REGISTER148_OFFSET 0x94
#define REGISTER149_OFFSET 0x95
#define REGISTER150_OFFSET 0x96
#define REGISTER151_OFFSET 0x97
#define REGISTER152_OFFSET 0x98
#define REGISTER153_OFFSET 0x99
#define REGISTER154_OFFSET 0x9a
#define REGISTER155_OFFSET 0x9b
#define REGISTER156_OFFSET 0x9c
#define REGISTER157_OFFSET 0x9d
#define REGISTER158_OFFSET 0x9e
#define REGISTER159_OFFSET 0x9f
#define REGISTER160_OFFSET 0xa0
#define REGISTER161_OFFSET 0xa1
#define REGISTER162_OFFSET 0xa2
#define REGISTER163_OFFSET 0xa3
#define REGISTER164_OFFSET 0xa4
#define REGISTER165_OFFSET 0xa5
#define REGISTER166_OFFSET 0xa6
#define REGISTER167_OFFSET 0xa7
#define REGISTER168_OFFSET 0xa8
#define REGISTER169_OFFSET 0xa9
#define REGISTER170_OFFSET 0xaa
#define REGISTER171_OFFSET 0xab
#define REGISTER172_OFFSET 0xac
#define REGISTER173_OFFSET 0xad
#define REGISTER174_OFFSET 0xae
#define REGISTER175_OFFSET 0xaf
#define REGISTER176_OFFSET 0xb0
#define REGISTER177_OFFSET 0xb1
#define REGISTER178_OFFSET 0xb2
#define REGISTER179_OFFSET 0xb3
#define REGISTER180_OFFSET 0xb4
#define REGISTER181_OFFSET 0xb5
#define REGISTER182_OFFSET 0xb6
#define REGISTER183_OFFSET 0xb7
#define REGISTER184_OFFSET 0xb8
#define REGISTER185_OFFSET 0xb9
#define REGISTER186_OFFSET 0xba
#define REGISTER187_OFFSET 0xbb
#define REGISTER188_OFFSET 0xbc
#define REGISTER189_OFFSET 0xbd
#define REGISTER190_OFFSET 0xbe
#define REGISTER191_OFFSET 0xbf
#define REGISTER192_OFFSET 0xc0
#define REGISTER193_OFFSET 0xc1
#define REGISTER194_OFFSET 0xc2
#define REGISTER195_OFFSET 0xc3
#define REGISTER196_OFFSET 0xc4
#define REGISTER197_OFFSET 0xc5
#define REGISTER198_OFFSET 0xc6
#define REGISTER199_OFFSET 0xc7
#define REGISTER200_OFFSET 0xc8
#define REGISTER201_OFFSET 0xc9
#define REGISTER202_OFFSET 0xca
#define REGISTER203_OFFSET 0xcb
#define REGISTER204_OFFSET 0xcc
#define REGISTER205_OFFSET 0xcd
#define REGISTER206_OFFSET 0xce
#define REGISTER207_OFFSET 0xcf
#define REGISTER208_OFFSET 0xd0
#define REGISTER209_OFFSET 0xd1
#define REGISTER210_OFFSET 0xd2
#define REGISTER211_OFFSET 0xd3
#define REGISTER212_OFFSET 0xd4
#define REGISTER213_OFFSET 0xd5
#define REGISTER214_OFFSET 0xd6
#define REGISTER215_OFFSET 0xd7
#define REGISTER216_OFFSET 0xd8
#define REGISTER217_OFFSET 0xd9
#define REGISTER218_OFFSET 0xda
#define REGISTER219_OFFSET 0xdb
#define REGISTER220_OFFSET 0xdc
#define REGISTER221_OFFSET 0xdd
#define REGISTER222_OFFSET 0xde
#define REGISTER223_OFFSET 0xdf
#define REGISTER224_OFFSET 0xe0
#define REGISTER225_OFFSET 0xe1
#define REGISTER226_OFFSET 0xe2
#define REGISTER227_OFFSET 0xe3
#define REGISTER228_OFFSET 0xe4
#define REGISTER229_OFFSET 0xe5
#define REGISTER230_OFFSET 0xe6
#define REGISTER231_OFFSET 0xe7
#define REGISTER232_OFFSET 0xe8
#define REGISTER233_OFFSET 0xe9
#define REGISTER234_OFFSET 0xea
#define REGISTER235_OFFSET 0xeb
#define REGISTER236_OFFSET 0xec
#define REGISTER237_OFFSET 0xed
#define REGISTER238_OFFSET 0xee
#define REGISTER239_OFFSET 0xef
#define REGISTER240_OFFSET 0xf0
#define REGISTER241_OFFSET 0xf1
#define REGISTER242_OFFSET 0xf2
#define REGISTER243_OFFSET 0xf3
#define REGISTER244_OFFSET 0xf4
#define REGISTER245_OFFSET 0xf5
#define REGISTER246_OFFSET 0xf6
#define REGISTER247_OFFSET 0xf7
#define REGISTER248_OFFSET 0xf8
#define REGISTER249_OFFSET 0xf9
#define REGISTER250_OFFSET 0xfa
#define REGISTER251_OFFSET 0xfb
#define REGISTER252_OFFSET 0xfc
#define REGISTER253_OFFSET 0xfd
#define REGISTER254_OFFSET 0xfe
#define REGISTER255_OFFSET 0xff
#define REGISTER256_OFFSET 0x100
#define REGISTER257_OFFSET 0x101
#define REGISTER258_OFFSET 0x102
#define REGISTER259_OFFSET 0x103
#define REGISTER260_OFFSET 0x104
#define REGISTER261_OFFSET 0x105
#define REGISTER262_OFFSET 0x106
#define REGISTER263_OFFSET 0x107
#define REGISTER264_OFFSET 0x108
#define REGISTER265_OFFSET 0x109
#define REGISTER266_OFFSET 0x10a
#define REGISTER267_OFFSET 0x10b
#define REGISTER268_OFFSET 0x10c
#define REGISTER269_OFFSET 0x10d
#define REGISTER270_OFFSET 0x10e
#define REGISTER271_OFFSET 0x10f
#define REGISTER272_OFFSET 0x110
#define REGISTER273_OFFSET 0x111
#define REGISTER274_OFFSET 0x112
#define REGISTER275_OFFSET 0x113
#define REGISTER276_OFFSET 0x114
#define REGISTER277_OFFSET 0x115
#define REGISTER278_OFFSET 0x116
#define REGISTER279_OFFSET 0x117
#define REGISTER280_OFFSET 0x118
#define REGISTER281_OFFSET 0x119
#define REGISTER282_OFFSET 0x11a
#define REGISTER283_OFFSET 0x11b
#define REGISTER284_OFFSET 0x11c
#define REGISTER285_OFFSET 0x11d
#define REGISTER286_OFFSET 0x11e
#define REGISTER287_OFFSET 0x11f
#define REGISTER288_OFFSET 0x120
#define REGISTER289_OFFSET 0x121
#define REGISTER290_OFFSET 0x122
#define REGISTER291_OFFSET 0x123
#define REGISTER292_OFFSET 0x124
#define REGISTER293_OFFSET 0x125
#define REGISTER294_OFFSET 0x126
#define REGISTER295_OFFSET 0x127
#define REGISTER296_OFFSET 0x128
#define REGISTER297_OFFSET 0x129
#define REGISTER298_OFFSET 0x12a
#define REGISTER299_OFFSET 0x12b
#define REGISTER300_OFFSET 0x12c
#define REGISTER301_OFFSET 0x12d
#define REGISTER302_OFFSET 0x12e
#define REGISTER303_OFFSET 0x12f
#define REGISTER304_OFFSET 0x130
#define REGISTER305_OFFSET 0x131
#define REGISTER306_OFFSET 0x132
#define REGISTER307_OFFSET 0x133
#define REGISTER308_OFFSET 0x134
#define REGISTER309_OFFSET 0x135
#define REGISTER310_OFFSET 0x136
#define REGISTER311_OFFSET 0x137
#define REGISTER312_OFFSET 0x138
#define REGISTER313_OFFSET 0x139
#define REGISTER314_OFFSET 0x13a
#define REGISTER315_OFFSET 0x13b
#define REGISTER316_OFFSET 0x13c
#define REGISTER317_OFFSET 0x13d
#define REGISTER318_OFFSET 0x13e
#define REGISTER319_OFFSET 0x13f
#define REGISTER320_OFFSET 0x140
#define REGISTER321_OFFSET 0x141
#define REGISTER322_OFFSET 0x142
#define REGISTER323_OFFSET 0x143
#define REGISTER324_OFFSET 0x144
#define REGISTER325_OFFSET 0x145
#define REGISTER326_OFFSET 0x146
#define REGISTER327_OFFSET 0x147
#define REGISTER328_OFFSET 0x148
#define REGISTER329_OFFSET 0x149
#define REGISTER330_OFFSET 0x14a
#define REGISTER331_OFFSET 0x14b
#define REGISTER332_OFFSET 0x14c
#define REGISTER333_OFFSET 0x14d
#define REGISTER334_OFFSET 0x14e
#define REGISTER335_OFFSET 0x14f
#define REGISTER336_OFFSET 0x150
#define REGISTER337_OFFSET 0x151
#define REGISTER338_OFFSET 0x152
#define REGISTER339_OFFSET 0x153
#define REGISTER340_OFFSET 0x154
#define REGISTER341_OFFSET 0x155
#define REGISTER342_OFFSET 0x156
#define REGISTER343_OFFSET 0x157
#define REGISTER344_OFFSET 0x158
#define REGISTER345_OFFSET 0x159
#define REGISTER346_OFFSET 0x15a
#define REGISTER347_OFFSET 0x15b
#define REGISTER348_OFFSET 0x15c
#define REGISTER349_OFFSET 0x15d
#define REGISTER350_OFFSET 0x15e
#define REGISTER351_OFFSET 0x15f
#define REGISTER352_OFFSET 0x160
#define REGISTER353_OFFSET 0x161
#define REGISTER354_OFFSET 0x162
#define REGISTER355_OFFSET 0x163
#define REGISTER356_OFFSET 0x164
#define REGISTER357_OFFSET 0x165
#define REGISTER358_OFFSET 0x166
#define REGISTER359_OFFSET 0x167
#define REGISTER360_OFFSET 0x168
#define REGISTER361_OFFSET 0x169
#define REGISTER362_OFFSET 0x16a
#define REGISTER363_OFFSET 0x16b
#define REGISTER364_OFFSET 0x16c
#define REGISTER365_OFFSET 0x16d
#define REGISTER366_OFFSET 0x16e
#define REGISTER367_OFFSET 0x16f
#define REGISTER368_OFFSET 0x170
#define REGISTER369_OFFSET 0x171
#define REGISTER370_OFFSET 0x172
#define REGISTER371_OFFSET 0x173
#define REGISTER372_OFFSET 0x174
#define REGISTER373_OFFSET 0x175
#define REGISTER374_OFFSET 0x176
#define REGISTER375_OFFSET 0x177
#define REGISTER376_OFFSET 0x178
#define REGISTER377_OFFSET 0x179
#define REGISTER378_OFFSET 0x17a
#define REGISTER379_OFFSET 0x17b
#define REGISTER380_OFFSET 0x17c
#define REGISTER381_OFFSET 0x17d
#define REGISTER382_OFFSET 0x17e
#define REGISTER383_OFFSET 0x17f
#define REGISTER384_OFFSET 0x180
#define REGISTER385_OFFSET 0x181
#define REGISTER386_OFFSET 0x182
#define REGISTER387_OFFSET 0x183
#define REGISTER388_OFFSET 0x184
#define REGISTER389_OFFSET 0x185
#define REGISTER390_OFFSET 0x186
#define REGISTER391_OFFSET 0x187
#define REGISTER392_OFFSET 0x188
#define REGISTER393_OFFSET 0x189
#define REGISTER394_OFFSET 0x18a
#define REGISTER395_OFFSET 0x18b
#define REGISTER396_OFFSET 0x18c
#define REGISTER397_OFFSET 0x18d
#define REGISTER398_OFFSET 0x18e
#define REGISTER399_OFFSET 0x18f
#define REGISTER400_OFFSET 0x190
#define REGISTER401_OFFSET 0x191
#define REGISTER402_OFFSET 0x192
#define REGISTER403_OFFSET 0x193
#define REGISTER404_OFFSET 0x194
#define REGISTER405_OFFSET 0x195
#define REGISTER406_OFFSET 0x196
#define REGISTER407_OFFSET 0x197
#define REGISTER408_OFFSET 0x198
#define REGISTER409_OFFSET 0x199
#define REGISTER410_OFFSET 0x19a
#define REGISTER411_OFFSET 0x19b
#define REGISTER412_OFFSET 0x19c
#define REGISTER413_OFFSET 0x19d
#define REGISTER414_OFFSET 0x19e
#define REGISTER415_OFFSET 0x19f
#define REGISTER416_OFFSET 0x1a0
#define REGISTER417_OFFSET 0x1a1
#define REGISTER418_OFFSET 0x1a2
#define REGISTER419_OFFSET 0x1a3
#define REGISTER420_OFFSET 0x1a4
#define REGISTER421_OFFSET 0x1a5
#define REGISTER422_OFFSET 0x1a6
#define REGISTER423_OFFSET 0x1a7
#define REGISTER424_OFFSET 0x1a8
#define REGISTER425_OFFSET 0x1a9
#define REGISTER426_OFFSET 0x1aa
#define REGISTER427_OFFSET 0x1ab
#define REGISTER428_OFFSET 0x1ac
#define REGISTER429_OFFSET 0x1ad
#define REGISTER430_OFFSET 0x1ae
#define REGISTER431_OFFSET 0x1af
#define REGISTER432_OFFSET 0x1b0
#define REGISTER433_OFFSET 0x1b1
#define REGISTER434_OFFSET 0x1b2
#define REGISTER435_OFFSET 0x1b3
#define REGISTER436_OFFSET 0x1b4
#define REGISTER437_OFFSET 0x1b5
#define REGISTER438_OFFSET 0x1b6
#define REGISTER439_OFFSET 0x1b7
#define REGISTER440_OFFSET 0x1b8
#define REGISTER441_OFFSET 0x1b9
#define REGISTER442_OFFSET 0x1ba
#define REGISTER443_OFFSET 0x1bb
#define REGISTER444_OFFSET 0x1bc
#define REGISTER445_OFFSET 0x1bd
#define REGISTER446_OFFSET 0x1be
#define REGISTER447_OFFSET 0x1bf
#define REGISTER448_OFFSET 0x1c0
#define REGISTER449_OFFSET 0x1c1
#define REGISTER450_OFFSET 0x1c2
#define REGISTER451_OFFSET 0x1c3
#define REGISTER452_OFFSET 0x1c4
#define REGISTER453_OFFSET 0x1c5
#define REGISTER454_OFFSET 0x1c6
#define REGISTER455_OFFSET 0x1c7
#define REGISTER456_OFFSET 0x1c8
#define REGISTER457_OFFSET 0x1c9
#define REGISTER458_OFFSET 0x1ca
#define REGISTER459_OFFSET 0x1cb
#define REGISTER460_OFFSET 0x1cc
#define REGISTER461_OFFSET 0x1cd
#define REGISTER462_OFFSET 0x1ce
#define REGISTER463_OFFSET 0x1cf
#define REGISTER464_OFFSET 0x1d0
#define REGISTER465_OFFSET 0x1d1
#define REGISTER466_OFFSET 0x1d2
#define REGISTER467_OFFSET 0x1d3
#define REGISTER468_OFFSET 0x1d4
#define REGISTER469_OFFSET 0x1d5
#define REGISTER470_OFFSET 0x1d6
#define REGISTER471_OFFSET 0x1d7
#define REGISTER472_OFFSET 0x1d8
#define REGISTER473_OFFSET 0x1d9
#define REGISTER474_OFFSET 0x1da
#define REGISTER475_OFFSET 0x1db
#define REGISTER476_OFFSET 0x1dc
#define REGISTER477_OFFSET 0x1dd
#define REGISTER478_OFFSET 0x1de
#define REGISTER479_OFFSET 0x1df
#define REGISTER480_OFFSET 0x1e0
#define REGISTER481_OFFSET 0x1e1
#define REGISTER482_OFFSET 0x1e2
#define REGISTER483_OFFSET 0x1e3
#define REGISTER484_OFFSET 0x1e4
#define REGISTER485_OFFSET 0x1e5
#define REGISTER486_OFFSET 0x1e6
#define REGISTER487_OFFSET 0x1e7
#define REGISTER488_OFFSET 0x1e8
#define REGISTER489_OFFSET 0x1e9
#define REGISTER490_OFFSET 0x1ea
#define REGISTER491_OFFSET 0x1eb
#define REGISTER492_OFFSET 0x1ec
#define REGISTER493_OFFSET 0x1ed
#define REGISTER494_OFFSET 0x1ee
#define REGISTER495_OFFSET 0x1ef
#define REGISTER496_OFFSET 0x1f0
#define REGISTER497_OFFSET 0x1f1
#define REGISTER498_OFFSET 0x1f2
#define REGISTER499_OFFSET 0x1f3
#define REGISTER500_OFFSET 0x1f4
#define REGISTER501_OFFSET 0x1f5
#define REGISTER502_OFFSET 0x1f6
#define REGISTER503_OFFSET 0x1f7
#define REGISTER504_OFFSET 0x1f8
#define REGISTER505_OFFSET 0x1f9
#define REGISTER506_OFFSET 0x1fa
#define REGISTER507_OFFSET 0x1fb
#define REGISTER508_OFFSET 0x1fc
#define REGISTER509_OFFSET 0x1fd
#define REGISTER510_OFFSET 0x1fe
#define REGISTER511_OFFSET 0x1ff

/* Driver function prototypes */
static int DPRAM_probe		(struct platform_device *pdev);
static int DPRAM_remove		(struct platform_device *pdev);
static ssize_t DPRAM_read	(struct file *filp, char *buffer, size_t len, loff_t *offset);
static ssize_t DPRAM_write	(struct file *filp, const char *buffer, size_t len, loff_t *offset);
static int DPRAM_open		(struct inode *inode, struct file *file);
static int DPRAM_release	(struct inode *inode, struct file *file);

/* FPGA device functions */
static ssize_t register0_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register0_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register1_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register1_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register2_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register2_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register3_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register3_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register4_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register4_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register5_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register5_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register6_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register6_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register7_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register7_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register8_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register8_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register9_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register9_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register10_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register10_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register11_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register11_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register12_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register12_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register13_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register13_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register14_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register14_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register15_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register15_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register16_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register16_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register17_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register17_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register18_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register18_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register19_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register19_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register20_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register20_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register21_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register21_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register22_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register22_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register23_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register23_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register24_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register24_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register25_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register25_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register26_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register26_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register27_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register27_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register28_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register28_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register29_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register29_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register30_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register30_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register31_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register31_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register32_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register32_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register33_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register33_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register34_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register34_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register35_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register35_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register36_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register36_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register37_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register37_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register38_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register38_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register39_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register39_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register40_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register40_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register41_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register41_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register42_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register42_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register43_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register43_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register44_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register44_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register45_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register45_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register46_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register46_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register47_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register47_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register48_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register48_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register49_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register49_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register50_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register50_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register51_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register51_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register52_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register52_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register53_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register53_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register54_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register54_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register55_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register55_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register56_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register56_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register57_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register57_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register58_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register58_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register59_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register59_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register60_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register60_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register61_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register61_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register62_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register62_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register63_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register63_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register64_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register64_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register65_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register65_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register66_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register66_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register67_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register67_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register68_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register68_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register69_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register69_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register70_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register70_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register71_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register71_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register72_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register72_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register73_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register73_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register74_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register74_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register75_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register75_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register76_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register76_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register77_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register77_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register78_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register78_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register79_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register79_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register80_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register80_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register81_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register81_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register82_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register82_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register83_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register83_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register84_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register84_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register85_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register85_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register86_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register86_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register87_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register87_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register88_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register88_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register89_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register89_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register90_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register90_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register91_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register91_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register92_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register92_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register93_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register93_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register94_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register94_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register95_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register95_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register96_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register96_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register97_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register97_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register98_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register98_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register99_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register99_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register100_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register100_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register101_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register101_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register102_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register102_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register103_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register103_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register104_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register104_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register105_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register105_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register106_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register106_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register107_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register107_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register108_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register108_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register109_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register109_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register110_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register110_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register111_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register111_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register112_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register112_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register113_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register113_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register114_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register114_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register115_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register115_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register116_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register116_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register117_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register117_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register118_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register118_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register119_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register119_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register120_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register120_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register121_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register121_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register122_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register122_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register123_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register123_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register124_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register124_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register125_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register125_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register126_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register126_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register127_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register127_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register128_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register128_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register129_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register129_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register130_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register130_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register131_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register131_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register132_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register132_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register133_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register133_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register134_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register134_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register135_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register135_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register136_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register136_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register137_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register137_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register138_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register138_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register139_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register139_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register140_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register140_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register141_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register141_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register142_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register142_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register143_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register143_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register144_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register144_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register145_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register145_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register146_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register146_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register147_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register147_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register148_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register148_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register149_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register149_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register150_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register150_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register151_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register151_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register152_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register152_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register153_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register153_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register154_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register154_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register155_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register155_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register156_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register156_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register157_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register157_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register158_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register158_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register159_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register159_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register160_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register160_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register161_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register161_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register162_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register162_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register163_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register163_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register164_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register164_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register165_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register165_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register166_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register166_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register167_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register167_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register168_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register168_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register169_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register169_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register170_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register170_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register171_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register171_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register172_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register172_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register173_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register173_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register174_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register174_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register175_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register175_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register176_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register176_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register177_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register177_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register178_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register178_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register179_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register179_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register180_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register180_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register181_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register181_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register182_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register182_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register183_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register183_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register184_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register184_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register185_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register185_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register186_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register186_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register187_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register187_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register188_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register188_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register189_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register189_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register190_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register190_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register191_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register191_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register192_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register192_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register193_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register193_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register194_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register194_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register195_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register195_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register196_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register196_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register197_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register197_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register198_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register198_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register199_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register199_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register200_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register200_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register201_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register201_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register202_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register202_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register203_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register203_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register204_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register204_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register205_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register205_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register206_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register206_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register207_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register207_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register208_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register208_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register209_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register209_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register210_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register210_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register211_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register211_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register212_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register212_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register213_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register213_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register214_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register214_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register215_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register215_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register216_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register216_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register217_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register217_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register218_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register218_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register219_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register219_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register220_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register220_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register221_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register221_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register222_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register222_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register223_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register223_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register224_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register224_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register225_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register225_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register226_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register226_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register227_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register227_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register228_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register228_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register229_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register229_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register230_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register230_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register231_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register231_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register232_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register232_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register233_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register233_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register234_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register234_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register235_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register235_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register236_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register236_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register237_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register237_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register238_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register238_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register239_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register239_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register240_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register240_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register241_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register241_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register242_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register242_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register243_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register243_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register244_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register244_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register245_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register245_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register246_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register246_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register247_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register247_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register248_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register248_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register249_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register249_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register250_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register250_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register251_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register251_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register252_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register252_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register253_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register253_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register254_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register254_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register255_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register255_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register256_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register256_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register257_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register257_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register258_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register258_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register259_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register259_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register260_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register260_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register261_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register261_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register262_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register262_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register263_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register263_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register264_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register264_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register265_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register265_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register266_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register266_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register267_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register267_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register268_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register268_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register269_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register269_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register270_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register270_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register271_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register271_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register272_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register272_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register273_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register273_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register274_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register274_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register275_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register275_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register276_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register276_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register277_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register277_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register278_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register278_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register279_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register279_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register280_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register280_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register281_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register281_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register282_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register282_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register283_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register283_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register284_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register284_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register285_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register285_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register286_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register286_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register287_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register287_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register288_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register288_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register289_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register289_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register290_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register290_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register291_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register291_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register292_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register292_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register293_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register293_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register294_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register294_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register295_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register295_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register296_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register296_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register297_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register297_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register298_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register298_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register299_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register299_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register300_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register300_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register301_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register301_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register302_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register302_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register303_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register303_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register304_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register304_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register305_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register305_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register306_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register306_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register307_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register307_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register308_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register308_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register309_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register309_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register310_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register310_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register311_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register311_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register312_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register312_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register313_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register313_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register314_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register314_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register315_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register315_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register316_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register316_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register317_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register317_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register318_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register318_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register319_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register319_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register320_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register320_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register321_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register321_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register322_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register322_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register323_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register323_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register324_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register324_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register325_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register325_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register326_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register326_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register327_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register327_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register328_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register328_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register329_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register329_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register330_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register330_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register331_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register331_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register332_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register332_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register333_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register333_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register334_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register334_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register335_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register335_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register336_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register336_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register337_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register337_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register338_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register338_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register339_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register339_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register340_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register340_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register341_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register341_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register342_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register342_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register343_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register343_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register344_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register344_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register345_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register345_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register346_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register346_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register347_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register347_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register348_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register348_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register349_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register349_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register350_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register350_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register351_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register351_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register352_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register352_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register353_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register353_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register354_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register354_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register355_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register355_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register356_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register356_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register357_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register357_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register358_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register358_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register359_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register359_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register360_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register360_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register361_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register361_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register362_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register362_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register363_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register363_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register364_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register364_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register365_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register365_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register366_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register366_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register367_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register367_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register368_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register368_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register369_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register369_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register370_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register370_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register371_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register371_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register372_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register372_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register373_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register373_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register374_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register374_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register375_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register375_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register376_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register376_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register377_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register377_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register378_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register378_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register379_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register379_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register380_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register380_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register381_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register381_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register382_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register382_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register383_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register383_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register384_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register384_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register385_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register385_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register386_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register386_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register387_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register387_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register388_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register388_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register389_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register389_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register390_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register390_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register391_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register391_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register392_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register392_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register393_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register393_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register394_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register394_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register395_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register395_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register396_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register396_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register397_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register397_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register398_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register398_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register399_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register399_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register400_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register400_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register401_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register401_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register402_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register402_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register403_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register403_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register404_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register404_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register405_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register405_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register406_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register406_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register407_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register407_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register408_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register408_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register409_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register409_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register410_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register410_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register411_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register411_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register412_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register412_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register413_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register413_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register414_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register414_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register415_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register415_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register416_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register416_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register417_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register417_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register418_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register418_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register419_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register419_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register420_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register420_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register421_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register421_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register422_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register422_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register423_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register423_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register424_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register424_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register425_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register425_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register426_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register426_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register427_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register427_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register428_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register428_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register429_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register429_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register430_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register430_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register431_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register431_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register432_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register432_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register433_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register433_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register434_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register434_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register435_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register435_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register436_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register436_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register437_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register437_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register438_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register438_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register439_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register439_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register440_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register440_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register441_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register441_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register442_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register442_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register443_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register443_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register444_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register444_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register445_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register445_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register446_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register446_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register447_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register447_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register448_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register448_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register449_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register449_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register450_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register450_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register451_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register451_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register452_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register452_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register453_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register453_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register454_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register454_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register455_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register455_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register456_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register456_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register457_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register457_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register458_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register458_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register459_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register459_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register460_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register460_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register461_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register461_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register462_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register462_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register463_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register463_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register464_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register464_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register465_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register465_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register466_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register466_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register467_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register467_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register468_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register468_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register469_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register469_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register470_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register470_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register471_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register471_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register472_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register472_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register473_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register473_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register474_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register474_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register475_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register475_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register476_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register476_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register477_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register477_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register478_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register478_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register479_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register479_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register480_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register480_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register481_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register481_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register482_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register482_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register483_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register483_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register484_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register484_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register485_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register485_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register486_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register486_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register487_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register487_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register488_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register488_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register489_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register489_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register490_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register490_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register491_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register491_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register492_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register492_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register493_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register493_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register494_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register494_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register495_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register495_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register496_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register496_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register497_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register497_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register498_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register498_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register499_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register499_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register500_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register500_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register501_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register501_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register502_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register502_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register503_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register503_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register504_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register504_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register505_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register505_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register506_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register506_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register507_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register507_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register508_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register508_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register509_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register509_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register510_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register510_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t register511_write 	 (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t register511_read 	 (struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t name_read	         (struct device *dev, struct device_attribute *attr, char *buf);

/* Device attributes that show up in /sys/class for device */
DEVICE_ATTR (register0,	 0664, 	 register0_read,	 register0_write);
DEVICE_ATTR (register1,	 0664, 	 register1_read,	 register1_write);
DEVICE_ATTR (register2,	 0664, 	 register2_read,	 register2_write);
DEVICE_ATTR (register3,	 0664, 	 register3_read,	 register3_write);
DEVICE_ATTR (register4,	 0664, 	 register4_read,	 register4_write);
DEVICE_ATTR (register5,	 0664, 	 register5_read,	 register5_write);
DEVICE_ATTR (register6,	 0664, 	 register6_read,	 register6_write);
DEVICE_ATTR (register7,	 0664, 	 register7_read,	 register7_write);
DEVICE_ATTR (register8,	 0664, 	 register8_read,	 register8_write);
DEVICE_ATTR (register9,	 0664, 	 register9_read,	 register9_write);
DEVICE_ATTR (register10,	 0664, 	 register10_read,	 register10_write);
DEVICE_ATTR (register11,	 0664, 	 register11_read,	 register11_write);
DEVICE_ATTR (register12,	 0664, 	 register12_read,	 register12_write);
DEVICE_ATTR (register13,	 0664, 	 register13_read,	 register13_write);
DEVICE_ATTR (register14,	 0664, 	 register14_read,	 register14_write);
DEVICE_ATTR (register15,	 0664, 	 register15_read,	 register15_write);
DEVICE_ATTR (register16,	 0664, 	 register16_read,	 register16_write);
DEVICE_ATTR (register17,	 0664, 	 register17_read,	 register17_write);
DEVICE_ATTR (register18,	 0664, 	 register18_read,	 register18_write);
DEVICE_ATTR (register19,	 0664, 	 register19_read,	 register19_write);
DEVICE_ATTR (register20,	 0664, 	 register20_read,	 register20_write);
DEVICE_ATTR (register21,	 0664, 	 register21_read,	 register21_write);
DEVICE_ATTR (register22,	 0664, 	 register22_read,	 register22_write);
DEVICE_ATTR (register23,	 0664, 	 register23_read,	 register23_write);
DEVICE_ATTR (register24,	 0664, 	 register24_read,	 register24_write);
DEVICE_ATTR (register25,	 0664, 	 register25_read,	 register25_write);
DEVICE_ATTR (register26,	 0664, 	 register26_read,	 register26_write);
DEVICE_ATTR (register27,	 0664, 	 register27_read,	 register27_write);
DEVICE_ATTR (register28,	 0664, 	 register28_read,	 register28_write);
DEVICE_ATTR (register29,	 0664, 	 register29_read,	 register29_write);
DEVICE_ATTR (register30,	 0664, 	 register30_read,	 register30_write);
DEVICE_ATTR (register31,	 0664, 	 register31_read,	 register31_write);
DEVICE_ATTR (register32,	 0664, 	 register32_read,	 register32_write);
DEVICE_ATTR (register33,	 0664, 	 register33_read,	 register33_write);
DEVICE_ATTR (register34,	 0664, 	 register34_read,	 register34_write);
DEVICE_ATTR (register35,	 0664, 	 register35_read,	 register35_write);
DEVICE_ATTR (register36,	 0664, 	 register36_read,	 register36_write);
DEVICE_ATTR (register37,	 0664, 	 register37_read,	 register37_write);
DEVICE_ATTR (register38,	 0664, 	 register38_read,	 register38_write);
DEVICE_ATTR (register39,	 0664, 	 register39_read,	 register39_write);
DEVICE_ATTR (register40,	 0664, 	 register40_read,	 register40_write);
DEVICE_ATTR (register41,	 0664, 	 register41_read,	 register41_write);
DEVICE_ATTR (register42,	 0664, 	 register42_read,	 register42_write);
DEVICE_ATTR (register43,	 0664, 	 register43_read,	 register43_write);
DEVICE_ATTR (register44,	 0664, 	 register44_read,	 register44_write);
DEVICE_ATTR (register45,	 0664, 	 register45_read,	 register45_write);
DEVICE_ATTR (register46,	 0664, 	 register46_read,	 register46_write);
DEVICE_ATTR (register47,	 0664, 	 register47_read,	 register47_write);
DEVICE_ATTR (register48,	 0664, 	 register48_read,	 register48_write);
DEVICE_ATTR (register49,	 0664, 	 register49_read,	 register49_write);
DEVICE_ATTR (register50,	 0664, 	 register50_read,	 register50_write);
DEVICE_ATTR (register51,	 0664, 	 register51_read,	 register51_write);
DEVICE_ATTR (register52,	 0664, 	 register52_read,	 register52_write);
DEVICE_ATTR (register53,	 0664, 	 register53_read,	 register53_write);
DEVICE_ATTR (register54,	 0664, 	 register54_read,	 register54_write);
DEVICE_ATTR (register55,	 0664, 	 register55_read,	 register55_write);
DEVICE_ATTR (register56,	 0664, 	 register56_read,	 register56_write);
DEVICE_ATTR (register57,	 0664, 	 register57_read,	 register57_write);
DEVICE_ATTR (register58,	 0664, 	 register58_read,	 register58_write);
DEVICE_ATTR (register59,	 0664, 	 register59_read,	 register59_write);
DEVICE_ATTR (register60,	 0664, 	 register60_read,	 register60_write);
DEVICE_ATTR (register61,	 0664, 	 register61_read,	 register61_write);
DEVICE_ATTR (register62,	 0664, 	 register62_read,	 register62_write);
DEVICE_ATTR (register63,	 0664, 	 register63_read,	 register63_write);
DEVICE_ATTR (register64,	 0664, 	 register64_read,	 register64_write);
DEVICE_ATTR (register65,	 0664, 	 register65_read,	 register65_write);
DEVICE_ATTR (register66,	 0664, 	 register66_read,	 register66_write);
DEVICE_ATTR (register67,	 0664, 	 register67_read,	 register67_write);
DEVICE_ATTR (register68,	 0664, 	 register68_read,	 register68_write);
DEVICE_ATTR (register69,	 0664, 	 register69_read,	 register69_write);
DEVICE_ATTR (register70,	 0664, 	 register70_read,	 register70_write);
DEVICE_ATTR (register71,	 0664, 	 register71_read,	 register71_write);
DEVICE_ATTR (register72,	 0664, 	 register72_read,	 register72_write);
DEVICE_ATTR (register73,	 0664, 	 register73_read,	 register73_write);
DEVICE_ATTR (register74,	 0664, 	 register74_read,	 register74_write);
DEVICE_ATTR (register75,	 0664, 	 register75_read,	 register75_write);
DEVICE_ATTR (register76,	 0664, 	 register76_read,	 register76_write);
DEVICE_ATTR (register77,	 0664, 	 register77_read,	 register77_write);
DEVICE_ATTR (register78,	 0664, 	 register78_read,	 register78_write);
DEVICE_ATTR (register79,	 0664, 	 register79_read,	 register79_write);
DEVICE_ATTR (register80,	 0664, 	 register80_read,	 register80_write);
DEVICE_ATTR (register81,	 0664, 	 register81_read,	 register81_write);
DEVICE_ATTR (register82,	 0664, 	 register82_read,	 register82_write);
DEVICE_ATTR (register83,	 0664, 	 register83_read,	 register83_write);
DEVICE_ATTR (register84,	 0664, 	 register84_read,	 register84_write);
DEVICE_ATTR (register85,	 0664, 	 register85_read,	 register85_write);
DEVICE_ATTR (register86,	 0664, 	 register86_read,	 register86_write);
DEVICE_ATTR (register87,	 0664, 	 register87_read,	 register87_write);
DEVICE_ATTR (register88,	 0664, 	 register88_read,	 register88_write);
DEVICE_ATTR (register89,	 0664, 	 register89_read,	 register89_write);
DEVICE_ATTR (register90,	 0664, 	 register90_read,	 register90_write);
DEVICE_ATTR (register91,	 0664, 	 register91_read,	 register91_write);
DEVICE_ATTR (register92,	 0664, 	 register92_read,	 register92_write);
DEVICE_ATTR (register93,	 0664, 	 register93_read,	 register93_write);
DEVICE_ATTR (register94,	 0664, 	 register94_read,	 register94_write);
DEVICE_ATTR (register95,	 0664, 	 register95_read,	 register95_write);
DEVICE_ATTR (register96,	 0664, 	 register96_read,	 register96_write);
DEVICE_ATTR (register97,	 0664, 	 register97_read,	 register97_write);
DEVICE_ATTR (register98,	 0664, 	 register98_read,	 register98_write);
DEVICE_ATTR (register99,	 0664, 	 register99_read,	 register99_write);
DEVICE_ATTR (register100,	 0664, 	 register100_read,	 register100_write);
DEVICE_ATTR (register101,	 0664, 	 register101_read,	 register101_write);
DEVICE_ATTR (register102,	 0664, 	 register102_read,	 register102_write);
DEVICE_ATTR (register103,	 0664, 	 register103_read,	 register103_write);
DEVICE_ATTR (register104,	 0664, 	 register104_read,	 register104_write);
DEVICE_ATTR (register105,	 0664, 	 register105_read,	 register105_write);
DEVICE_ATTR (register106,	 0664, 	 register106_read,	 register106_write);
DEVICE_ATTR (register107,	 0664, 	 register107_read,	 register107_write);
DEVICE_ATTR (register108,	 0664, 	 register108_read,	 register108_write);
DEVICE_ATTR (register109,	 0664, 	 register109_read,	 register109_write);
DEVICE_ATTR (register110,	 0664, 	 register110_read,	 register110_write);
DEVICE_ATTR (register111,	 0664, 	 register111_read,	 register111_write);
DEVICE_ATTR (register112,	 0664, 	 register112_read,	 register112_write);
DEVICE_ATTR (register113,	 0664, 	 register113_read,	 register113_write);
DEVICE_ATTR (register114,	 0664, 	 register114_read,	 register114_write);
DEVICE_ATTR (register115,	 0664, 	 register115_read,	 register115_write);
DEVICE_ATTR (register116,	 0664, 	 register116_read,	 register116_write);
DEVICE_ATTR (register117,	 0664, 	 register117_read,	 register117_write);
DEVICE_ATTR (register118,	 0664, 	 register118_read,	 register118_write);
DEVICE_ATTR (register119,	 0664, 	 register119_read,	 register119_write);
DEVICE_ATTR (register120,	 0664, 	 register120_read,	 register120_write);
DEVICE_ATTR (register121,	 0664, 	 register121_read,	 register121_write);
DEVICE_ATTR (register122,	 0664, 	 register122_read,	 register122_write);
DEVICE_ATTR (register123,	 0664, 	 register123_read,	 register123_write);
DEVICE_ATTR (register124,	 0664, 	 register124_read,	 register124_write);
DEVICE_ATTR (register125,	 0664, 	 register125_read,	 register125_write);
DEVICE_ATTR (register126,	 0664, 	 register126_read,	 register126_write);
DEVICE_ATTR (register127,	 0664, 	 register127_read,	 register127_write);
DEVICE_ATTR (register128,	 0664, 	 register128_read,	 register128_write);
DEVICE_ATTR (register129,	 0664, 	 register129_read,	 register129_write);
DEVICE_ATTR (register130,	 0664, 	 register130_read,	 register130_write);
DEVICE_ATTR (register131,	 0664, 	 register131_read,	 register131_write);
DEVICE_ATTR (register132,	 0664, 	 register132_read,	 register132_write);
DEVICE_ATTR (register133,	 0664, 	 register133_read,	 register133_write);
DEVICE_ATTR (register134,	 0664, 	 register134_read,	 register134_write);
DEVICE_ATTR (register135,	 0664, 	 register135_read,	 register135_write);
DEVICE_ATTR (register136,	 0664, 	 register136_read,	 register136_write);
DEVICE_ATTR (register137,	 0664, 	 register137_read,	 register137_write);
DEVICE_ATTR (register138,	 0664, 	 register138_read,	 register138_write);
DEVICE_ATTR (register139,	 0664, 	 register139_read,	 register139_write);
DEVICE_ATTR (register140,	 0664, 	 register140_read,	 register140_write);
DEVICE_ATTR (register141,	 0664, 	 register141_read,	 register141_write);
DEVICE_ATTR (register142,	 0664, 	 register142_read,	 register142_write);
DEVICE_ATTR (register143,	 0664, 	 register143_read,	 register143_write);
DEVICE_ATTR (register144,	 0664, 	 register144_read,	 register144_write);
DEVICE_ATTR (register145,	 0664, 	 register145_read,	 register145_write);
DEVICE_ATTR (register146,	 0664, 	 register146_read,	 register146_write);
DEVICE_ATTR (register147,	 0664, 	 register147_read,	 register147_write);
DEVICE_ATTR (register148,	 0664, 	 register148_read,	 register148_write);
DEVICE_ATTR (register149,	 0664, 	 register149_read,	 register149_write);
DEVICE_ATTR (register150,	 0664, 	 register150_read,	 register150_write);
DEVICE_ATTR (register151,	 0664, 	 register151_read,	 register151_write);
DEVICE_ATTR (register152,	 0664, 	 register152_read,	 register152_write);
DEVICE_ATTR (register153,	 0664, 	 register153_read,	 register153_write);
DEVICE_ATTR (register154,	 0664, 	 register154_read,	 register154_write);
DEVICE_ATTR (register155,	 0664, 	 register155_read,	 register155_write);
DEVICE_ATTR (register156,	 0664, 	 register156_read,	 register156_write);
DEVICE_ATTR (register157,	 0664, 	 register157_read,	 register157_write);
DEVICE_ATTR (register158,	 0664, 	 register158_read,	 register158_write);
DEVICE_ATTR (register159,	 0664, 	 register159_read,	 register159_write);
DEVICE_ATTR (register160,	 0664, 	 register160_read,	 register160_write);
DEVICE_ATTR (register161,	 0664, 	 register161_read,	 register161_write);
DEVICE_ATTR (register162,	 0664, 	 register162_read,	 register162_write);
DEVICE_ATTR (register163,	 0664, 	 register163_read,	 register163_write);
DEVICE_ATTR (register164,	 0664, 	 register164_read,	 register164_write);
DEVICE_ATTR (register165,	 0664, 	 register165_read,	 register165_write);
DEVICE_ATTR (register166,	 0664, 	 register166_read,	 register166_write);
DEVICE_ATTR (register167,	 0664, 	 register167_read,	 register167_write);
DEVICE_ATTR (register168,	 0664, 	 register168_read,	 register168_write);
DEVICE_ATTR (register169,	 0664, 	 register169_read,	 register169_write);
DEVICE_ATTR (register170,	 0664, 	 register170_read,	 register170_write);
DEVICE_ATTR (register171,	 0664, 	 register171_read,	 register171_write);
DEVICE_ATTR (register172,	 0664, 	 register172_read,	 register172_write);
DEVICE_ATTR (register173,	 0664, 	 register173_read,	 register173_write);
DEVICE_ATTR (register174,	 0664, 	 register174_read,	 register174_write);
DEVICE_ATTR (register175,	 0664, 	 register175_read,	 register175_write);
DEVICE_ATTR (register176,	 0664, 	 register176_read,	 register176_write);
DEVICE_ATTR (register177,	 0664, 	 register177_read,	 register177_write);
DEVICE_ATTR (register178,	 0664, 	 register178_read,	 register178_write);
DEVICE_ATTR (register179,	 0664, 	 register179_read,	 register179_write);
DEVICE_ATTR (register180,	 0664, 	 register180_read,	 register180_write);
DEVICE_ATTR (register181,	 0664, 	 register181_read,	 register181_write);
DEVICE_ATTR (register182,	 0664, 	 register182_read,	 register182_write);
DEVICE_ATTR (register183,	 0664, 	 register183_read,	 register183_write);
DEVICE_ATTR (register184,	 0664, 	 register184_read,	 register184_write);
DEVICE_ATTR (register185,	 0664, 	 register185_read,	 register185_write);
DEVICE_ATTR (register186,	 0664, 	 register186_read,	 register186_write);
DEVICE_ATTR (register187,	 0664, 	 register187_read,	 register187_write);
DEVICE_ATTR (register188,	 0664, 	 register188_read,	 register188_write);
DEVICE_ATTR (register189,	 0664, 	 register189_read,	 register189_write);
DEVICE_ATTR (register190,	 0664, 	 register190_read,	 register190_write);
DEVICE_ATTR (register191,	 0664, 	 register191_read,	 register191_write);
DEVICE_ATTR (register192,	 0664, 	 register192_read,	 register192_write);
DEVICE_ATTR (register193,	 0664, 	 register193_read,	 register193_write);
DEVICE_ATTR (register194,	 0664, 	 register194_read,	 register194_write);
DEVICE_ATTR (register195,	 0664, 	 register195_read,	 register195_write);
DEVICE_ATTR (register196,	 0664, 	 register196_read,	 register196_write);
DEVICE_ATTR (register197,	 0664, 	 register197_read,	 register197_write);
DEVICE_ATTR (register198,	 0664, 	 register198_read,	 register198_write);
DEVICE_ATTR (register199,	 0664, 	 register199_read,	 register199_write);
DEVICE_ATTR (register200,	 0664, 	 register200_read,	 register200_write);
DEVICE_ATTR (register201,	 0664, 	 register201_read,	 register201_write);
DEVICE_ATTR (register202,	 0664, 	 register202_read,	 register202_write);
DEVICE_ATTR (register203,	 0664, 	 register203_read,	 register203_write);
DEVICE_ATTR (register204,	 0664, 	 register204_read,	 register204_write);
DEVICE_ATTR (register205,	 0664, 	 register205_read,	 register205_write);
DEVICE_ATTR (register206,	 0664, 	 register206_read,	 register206_write);
DEVICE_ATTR (register207,	 0664, 	 register207_read,	 register207_write);
DEVICE_ATTR (register208,	 0664, 	 register208_read,	 register208_write);
DEVICE_ATTR (register209,	 0664, 	 register209_read,	 register209_write);
DEVICE_ATTR (register210,	 0664, 	 register210_read,	 register210_write);
DEVICE_ATTR (register211,	 0664, 	 register211_read,	 register211_write);
DEVICE_ATTR (register212,	 0664, 	 register212_read,	 register212_write);
DEVICE_ATTR (register213,	 0664, 	 register213_read,	 register213_write);
DEVICE_ATTR (register214,	 0664, 	 register214_read,	 register214_write);
DEVICE_ATTR (register215,	 0664, 	 register215_read,	 register215_write);
DEVICE_ATTR (register216,	 0664, 	 register216_read,	 register216_write);
DEVICE_ATTR (register217,	 0664, 	 register217_read,	 register217_write);
DEVICE_ATTR (register218,	 0664, 	 register218_read,	 register218_write);
DEVICE_ATTR (register219,	 0664, 	 register219_read,	 register219_write);
DEVICE_ATTR (register220,	 0664, 	 register220_read,	 register220_write);
DEVICE_ATTR (register221,	 0664, 	 register221_read,	 register221_write);
DEVICE_ATTR (register222,	 0664, 	 register222_read,	 register222_write);
DEVICE_ATTR (register223,	 0664, 	 register223_read,	 register223_write);
DEVICE_ATTR (register224,	 0664, 	 register224_read,	 register224_write);
DEVICE_ATTR (register225,	 0664, 	 register225_read,	 register225_write);
DEVICE_ATTR (register226,	 0664, 	 register226_read,	 register226_write);
DEVICE_ATTR (register227,	 0664, 	 register227_read,	 register227_write);
DEVICE_ATTR (register228,	 0664, 	 register228_read,	 register228_write);
DEVICE_ATTR (register229,	 0664, 	 register229_read,	 register229_write);
DEVICE_ATTR (register230,	 0664, 	 register230_read,	 register230_write);
DEVICE_ATTR (register231,	 0664, 	 register231_read,	 register231_write);
DEVICE_ATTR (register232,	 0664, 	 register232_read,	 register232_write);
DEVICE_ATTR (register233,	 0664, 	 register233_read,	 register233_write);
DEVICE_ATTR (register234,	 0664, 	 register234_read,	 register234_write);
DEVICE_ATTR (register235,	 0664, 	 register235_read,	 register235_write);
DEVICE_ATTR (register236,	 0664, 	 register236_read,	 register236_write);
DEVICE_ATTR (register237,	 0664, 	 register237_read,	 register237_write);
DEVICE_ATTR (register238,	 0664, 	 register238_read,	 register238_write);
DEVICE_ATTR (register239,	 0664, 	 register239_read,	 register239_write);
DEVICE_ATTR (register240,	 0664, 	 register240_read,	 register240_write);
DEVICE_ATTR (register241,	 0664, 	 register241_read,	 register241_write);
DEVICE_ATTR (register242,	 0664, 	 register242_read,	 register242_write);
DEVICE_ATTR (register243,	 0664, 	 register243_read,	 register243_write);
DEVICE_ATTR (register244,	 0664, 	 register244_read,	 register244_write);
DEVICE_ATTR (register245,	 0664, 	 register245_read,	 register245_write);
DEVICE_ATTR (register246,	 0664, 	 register246_read,	 register246_write);
DEVICE_ATTR (register247,	 0664, 	 register247_read,	 register247_write);
DEVICE_ATTR (register248,	 0664, 	 register248_read,	 register248_write);
DEVICE_ATTR (register249,	 0664, 	 register249_read,	 register249_write);
DEVICE_ATTR (register250,	 0664, 	 register250_read,	 register250_write);
DEVICE_ATTR (register251,	 0664, 	 register251_read,	 register251_write);
DEVICE_ATTR (register252,	 0664, 	 register252_read,	 register252_write);
DEVICE_ATTR (register253,	 0664, 	 register253_read,	 register253_write);
DEVICE_ATTR (register254,	 0664, 	 register254_read,	 register254_write);
DEVICE_ATTR (register255,	 0664, 	 register255_read,	 register255_write);
DEVICE_ATTR (register256,	 0664, 	 register256_read,	 register256_write);
DEVICE_ATTR (register257,	 0664, 	 register257_read,	 register257_write);
DEVICE_ATTR (register258,	 0664, 	 register258_read,	 register258_write);
DEVICE_ATTR (register259,	 0664, 	 register259_read,	 register259_write);
DEVICE_ATTR (register260,	 0664, 	 register260_read,	 register260_write);
DEVICE_ATTR (register261,	 0664, 	 register261_read,	 register261_write);
DEVICE_ATTR (register262,	 0664, 	 register262_read,	 register262_write);
DEVICE_ATTR (register263,	 0664, 	 register263_read,	 register263_write);
DEVICE_ATTR (register264,	 0664, 	 register264_read,	 register264_write);
DEVICE_ATTR (register265,	 0664, 	 register265_read,	 register265_write);
DEVICE_ATTR (register266,	 0664, 	 register266_read,	 register266_write);
DEVICE_ATTR (register267,	 0664, 	 register267_read,	 register267_write);
DEVICE_ATTR (register268,	 0664, 	 register268_read,	 register268_write);
DEVICE_ATTR (register269,	 0664, 	 register269_read,	 register269_write);
DEVICE_ATTR (register270,	 0664, 	 register270_read,	 register270_write);
DEVICE_ATTR (register271,	 0664, 	 register271_read,	 register271_write);
DEVICE_ATTR (register272,	 0664, 	 register272_read,	 register272_write);
DEVICE_ATTR (register273,	 0664, 	 register273_read,	 register273_write);
DEVICE_ATTR (register274,	 0664, 	 register274_read,	 register274_write);
DEVICE_ATTR (register275,	 0664, 	 register275_read,	 register275_write);
DEVICE_ATTR (register276,	 0664, 	 register276_read,	 register276_write);
DEVICE_ATTR (register277,	 0664, 	 register277_read,	 register277_write);
DEVICE_ATTR (register278,	 0664, 	 register278_read,	 register278_write);
DEVICE_ATTR (register279,	 0664, 	 register279_read,	 register279_write);
DEVICE_ATTR (register280,	 0664, 	 register280_read,	 register280_write);
DEVICE_ATTR (register281,	 0664, 	 register281_read,	 register281_write);
DEVICE_ATTR (register282,	 0664, 	 register282_read,	 register282_write);
DEVICE_ATTR (register283,	 0664, 	 register283_read,	 register283_write);
DEVICE_ATTR (register284,	 0664, 	 register284_read,	 register284_write);
DEVICE_ATTR (register285,	 0664, 	 register285_read,	 register285_write);
DEVICE_ATTR (register286,	 0664, 	 register286_read,	 register286_write);
DEVICE_ATTR (register287,	 0664, 	 register287_read,	 register287_write);
DEVICE_ATTR (register288,	 0664, 	 register288_read,	 register288_write);
DEVICE_ATTR (register289,	 0664, 	 register289_read,	 register289_write);
DEVICE_ATTR (register290,	 0664, 	 register290_read,	 register290_write);
DEVICE_ATTR (register291,	 0664, 	 register291_read,	 register291_write);
DEVICE_ATTR (register292,	 0664, 	 register292_read,	 register292_write);
DEVICE_ATTR (register293,	 0664, 	 register293_read,	 register293_write);
DEVICE_ATTR (register294,	 0664, 	 register294_read,	 register294_write);
DEVICE_ATTR (register295,	 0664, 	 register295_read,	 register295_write);
DEVICE_ATTR (register296,	 0664, 	 register296_read,	 register296_write);
DEVICE_ATTR (register297,	 0664, 	 register297_read,	 register297_write);
DEVICE_ATTR (register298,	 0664, 	 register298_read,	 register298_write);
DEVICE_ATTR (register299,	 0664, 	 register299_read,	 register299_write);
DEVICE_ATTR (register300,	 0664, 	 register300_read,	 register300_write);
DEVICE_ATTR (register301,	 0664, 	 register301_read,	 register301_write);
DEVICE_ATTR (register302,	 0664, 	 register302_read,	 register302_write);
DEVICE_ATTR (register303,	 0664, 	 register303_read,	 register303_write);
DEVICE_ATTR (register304,	 0664, 	 register304_read,	 register304_write);
DEVICE_ATTR (register305,	 0664, 	 register305_read,	 register305_write);
DEVICE_ATTR (register306,	 0664, 	 register306_read,	 register306_write);
DEVICE_ATTR (register307,	 0664, 	 register307_read,	 register307_write);
DEVICE_ATTR (register308,	 0664, 	 register308_read,	 register308_write);
DEVICE_ATTR (register309,	 0664, 	 register309_read,	 register309_write);
DEVICE_ATTR (register310,	 0664, 	 register310_read,	 register310_write);
DEVICE_ATTR (register311,	 0664, 	 register311_read,	 register311_write);
DEVICE_ATTR (register312,	 0664, 	 register312_read,	 register312_write);
DEVICE_ATTR (register313,	 0664, 	 register313_read,	 register313_write);
DEVICE_ATTR (register314,	 0664, 	 register314_read,	 register314_write);
DEVICE_ATTR (register315,	 0664, 	 register315_read,	 register315_write);
DEVICE_ATTR (register316,	 0664, 	 register316_read,	 register316_write);
DEVICE_ATTR (register317,	 0664, 	 register317_read,	 register317_write);
DEVICE_ATTR (register318,	 0664, 	 register318_read,	 register318_write);
DEVICE_ATTR (register319,	 0664, 	 register319_read,	 register319_write);
DEVICE_ATTR (register320,	 0664, 	 register320_read,	 register320_write);
DEVICE_ATTR (register321,	 0664, 	 register321_read,	 register321_write);
DEVICE_ATTR (register322,	 0664, 	 register322_read,	 register322_write);
DEVICE_ATTR (register323,	 0664, 	 register323_read,	 register323_write);
DEVICE_ATTR (register324,	 0664, 	 register324_read,	 register324_write);
DEVICE_ATTR (register325,	 0664, 	 register325_read,	 register325_write);
DEVICE_ATTR (register326,	 0664, 	 register326_read,	 register326_write);
DEVICE_ATTR (register327,	 0664, 	 register327_read,	 register327_write);
DEVICE_ATTR (register328,	 0664, 	 register328_read,	 register328_write);
DEVICE_ATTR (register329,	 0664, 	 register329_read,	 register329_write);
DEVICE_ATTR (register330,	 0664, 	 register330_read,	 register330_write);
DEVICE_ATTR (register331,	 0664, 	 register331_read,	 register331_write);
DEVICE_ATTR (register332,	 0664, 	 register332_read,	 register332_write);
DEVICE_ATTR (register333,	 0664, 	 register333_read,	 register333_write);
DEVICE_ATTR (register334,	 0664, 	 register334_read,	 register334_write);
DEVICE_ATTR (register335,	 0664, 	 register335_read,	 register335_write);
DEVICE_ATTR (register336,	 0664, 	 register336_read,	 register336_write);
DEVICE_ATTR (register337,	 0664, 	 register337_read,	 register337_write);
DEVICE_ATTR (register338,	 0664, 	 register338_read,	 register338_write);
DEVICE_ATTR (register339,	 0664, 	 register339_read,	 register339_write);
DEVICE_ATTR (register340,	 0664, 	 register340_read,	 register340_write);
DEVICE_ATTR (register341,	 0664, 	 register341_read,	 register341_write);
DEVICE_ATTR (register342,	 0664, 	 register342_read,	 register342_write);
DEVICE_ATTR (register343,	 0664, 	 register343_read,	 register343_write);
DEVICE_ATTR (register344,	 0664, 	 register344_read,	 register344_write);
DEVICE_ATTR (register345,	 0664, 	 register345_read,	 register345_write);
DEVICE_ATTR (register346,	 0664, 	 register346_read,	 register346_write);
DEVICE_ATTR (register347,	 0664, 	 register347_read,	 register347_write);
DEVICE_ATTR (register348,	 0664, 	 register348_read,	 register348_write);
DEVICE_ATTR (register349,	 0664, 	 register349_read,	 register349_write);
DEVICE_ATTR (register350,	 0664, 	 register350_read,	 register350_write);
DEVICE_ATTR (register351,	 0664, 	 register351_read,	 register351_write);
DEVICE_ATTR (register352,	 0664, 	 register352_read,	 register352_write);
DEVICE_ATTR (register353,	 0664, 	 register353_read,	 register353_write);
DEVICE_ATTR (register354,	 0664, 	 register354_read,	 register354_write);
DEVICE_ATTR (register355,	 0664, 	 register355_read,	 register355_write);
DEVICE_ATTR (register356,	 0664, 	 register356_read,	 register356_write);
DEVICE_ATTR (register357,	 0664, 	 register357_read,	 register357_write);
DEVICE_ATTR (register358,	 0664, 	 register358_read,	 register358_write);
DEVICE_ATTR (register359,	 0664, 	 register359_read,	 register359_write);
DEVICE_ATTR (register360,	 0664, 	 register360_read,	 register360_write);
DEVICE_ATTR (register361,	 0664, 	 register361_read,	 register361_write);
DEVICE_ATTR (register362,	 0664, 	 register362_read,	 register362_write);
DEVICE_ATTR (register363,	 0664, 	 register363_read,	 register363_write);
DEVICE_ATTR (register364,	 0664, 	 register364_read,	 register364_write);
DEVICE_ATTR (register365,	 0664, 	 register365_read,	 register365_write);
DEVICE_ATTR (register366,	 0664, 	 register366_read,	 register366_write);
DEVICE_ATTR (register367,	 0664, 	 register367_read,	 register367_write);
DEVICE_ATTR (register368,	 0664, 	 register368_read,	 register368_write);
DEVICE_ATTR (register369,	 0664, 	 register369_read,	 register369_write);
DEVICE_ATTR (register370,	 0664, 	 register370_read,	 register370_write);
DEVICE_ATTR (register371,	 0664, 	 register371_read,	 register371_write);
DEVICE_ATTR (register372,	 0664, 	 register372_read,	 register372_write);
DEVICE_ATTR (register373,	 0664, 	 register373_read,	 register373_write);
DEVICE_ATTR (register374,	 0664, 	 register374_read,	 register374_write);
DEVICE_ATTR (register375,	 0664, 	 register375_read,	 register375_write);
DEVICE_ATTR (register376,	 0664, 	 register376_read,	 register376_write);
DEVICE_ATTR (register377,	 0664, 	 register377_read,	 register377_write);
DEVICE_ATTR (register378,	 0664, 	 register378_read,	 register378_write);
DEVICE_ATTR (register379,	 0664, 	 register379_read,	 register379_write);
DEVICE_ATTR (register380,	 0664, 	 register380_read,	 register380_write);
DEVICE_ATTR (register381,	 0664, 	 register381_read,	 register381_write);
DEVICE_ATTR (register382,	 0664, 	 register382_read,	 register382_write);
DEVICE_ATTR (register383,	 0664, 	 register383_read,	 register383_write);
DEVICE_ATTR (register384,	 0664, 	 register384_read,	 register384_write);
DEVICE_ATTR (register385,	 0664, 	 register385_read,	 register385_write);
DEVICE_ATTR (register386,	 0664, 	 register386_read,	 register386_write);
DEVICE_ATTR (register387,	 0664, 	 register387_read,	 register387_write);
DEVICE_ATTR (register388,	 0664, 	 register388_read,	 register388_write);
DEVICE_ATTR (register389,	 0664, 	 register389_read,	 register389_write);
DEVICE_ATTR (register390,	 0664, 	 register390_read,	 register390_write);
DEVICE_ATTR (register391,	 0664, 	 register391_read,	 register391_write);
DEVICE_ATTR (register392,	 0664, 	 register392_read,	 register392_write);
DEVICE_ATTR (register393,	 0664, 	 register393_read,	 register393_write);
DEVICE_ATTR (register394,	 0664, 	 register394_read,	 register394_write);
DEVICE_ATTR (register395,	 0664, 	 register395_read,	 register395_write);
DEVICE_ATTR (register396,	 0664, 	 register396_read,	 register396_write);
DEVICE_ATTR (register397,	 0664, 	 register397_read,	 register397_write);
DEVICE_ATTR (register398,	 0664, 	 register398_read,	 register398_write);
DEVICE_ATTR (register399,	 0664, 	 register399_read,	 register399_write);
DEVICE_ATTR (register400,	 0664, 	 register400_read,	 register400_write);
DEVICE_ATTR (register401,	 0664, 	 register401_read,	 register401_write);
DEVICE_ATTR (register402,	 0664, 	 register402_read,	 register402_write);
DEVICE_ATTR (register403,	 0664, 	 register403_read,	 register403_write);
DEVICE_ATTR (register404,	 0664, 	 register404_read,	 register404_write);
DEVICE_ATTR (register405,	 0664, 	 register405_read,	 register405_write);
DEVICE_ATTR (register406,	 0664, 	 register406_read,	 register406_write);
DEVICE_ATTR (register407,	 0664, 	 register407_read,	 register407_write);
DEVICE_ATTR (register408,	 0664, 	 register408_read,	 register408_write);
DEVICE_ATTR (register409,	 0664, 	 register409_read,	 register409_write);
DEVICE_ATTR (register410,	 0664, 	 register410_read,	 register410_write);
DEVICE_ATTR (register411,	 0664, 	 register411_read,	 register411_write);
DEVICE_ATTR (register412,	 0664, 	 register412_read,	 register412_write);
DEVICE_ATTR (register413,	 0664, 	 register413_read,	 register413_write);
DEVICE_ATTR (register414,	 0664, 	 register414_read,	 register414_write);
DEVICE_ATTR (register415,	 0664, 	 register415_read,	 register415_write);
DEVICE_ATTR (register416,	 0664, 	 register416_read,	 register416_write);
DEVICE_ATTR (register417,	 0664, 	 register417_read,	 register417_write);
DEVICE_ATTR (register418,	 0664, 	 register418_read,	 register418_write);
DEVICE_ATTR (register419,	 0664, 	 register419_read,	 register419_write);
DEVICE_ATTR (register420,	 0664, 	 register420_read,	 register420_write);
DEVICE_ATTR (register421,	 0664, 	 register421_read,	 register421_write);
DEVICE_ATTR (register422,	 0664, 	 register422_read,	 register422_write);
DEVICE_ATTR (register423,	 0664, 	 register423_read,	 register423_write);
DEVICE_ATTR (register424,	 0664, 	 register424_read,	 register424_write);
DEVICE_ATTR (register425,	 0664, 	 register425_read,	 register425_write);
DEVICE_ATTR (register426,	 0664, 	 register426_read,	 register426_write);
DEVICE_ATTR (register427,	 0664, 	 register427_read,	 register427_write);
DEVICE_ATTR (register428,	 0664, 	 register428_read,	 register428_write);
DEVICE_ATTR (register429,	 0664, 	 register429_read,	 register429_write);
DEVICE_ATTR (register430,	 0664, 	 register430_read,	 register430_write);
DEVICE_ATTR (register431,	 0664, 	 register431_read,	 register431_write);
DEVICE_ATTR (register432,	 0664, 	 register432_read,	 register432_write);
DEVICE_ATTR (register433,	 0664, 	 register433_read,	 register433_write);
DEVICE_ATTR (register434,	 0664, 	 register434_read,	 register434_write);
DEVICE_ATTR (register435,	 0664, 	 register435_read,	 register435_write);
DEVICE_ATTR (register436,	 0664, 	 register436_read,	 register436_write);
DEVICE_ATTR (register437,	 0664, 	 register437_read,	 register437_write);
DEVICE_ATTR (register438,	 0664, 	 register438_read,	 register438_write);
DEVICE_ATTR (register439,	 0664, 	 register439_read,	 register439_write);
DEVICE_ATTR (register440,	 0664, 	 register440_read,	 register440_write);
DEVICE_ATTR (register441,	 0664, 	 register441_read,	 register441_write);
DEVICE_ATTR (register442,	 0664, 	 register442_read,	 register442_write);
DEVICE_ATTR (register443,	 0664, 	 register443_read,	 register443_write);
DEVICE_ATTR (register444,	 0664, 	 register444_read,	 register444_write);
DEVICE_ATTR (register445,	 0664, 	 register445_read,	 register445_write);
DEVICE_ATTR (register446,	 0664, 	 register446_read,	 register446_write);
DEVICE_ATTR (register447,	 0664, 	 register447_read,	 register447_write);
DEVICE_ATTR (register448,	 0664, 	 register448_read,	 register448_write);
DEVICE_ATTR (register449,	 0664, 	 register449_read,	 register449_write);
DEVICE_ATTR (register450,	 0664, 	 register450_read,	 register450_write);
DEVICE_ATTR (register451,	 0664, 	 register451_read,	 register451_write);
DEVICE_ATTR (register452,	 0664, 	 register452_read,	 register452_write);
DEVICE_ATTR (register453,	 0664, 	 register453_read,	 register453_write);
DEVICE_ATTR (register454,	 0664, 	 register454_read,	 register454_write);
DEVICE_ATTR (register455,	 0664, 	 register455_read,	 register455_write);
DEVICE_ATTR (register456,	 0664, 	 register456_read,	 register456_write);
DEVICE_ATTR (register457,	 0664, 	 register457_read,	 register457_write);
DEVICE_ATTR (register458,	 0664, 	 register458_read,	 register458_write);
DEVICE_ATTR (register459,	 0664, 	 register459_read,	 register459_write);
DEVICE_ATTR (register460,	 0664, 	 register460_read,	 register460_write);
DEVICE_ATTR (register461,	 0664, 	 register461_read,	 register461_write);
DEVICE_ATTR (register462,	 0664, 	 register462_read,	 register462_write);
DEVICE_ATTR (register463,	 0664, 	 register463_read,	 register463_write);
DEVICE_ATTR (register464,	 0664, 	 register464_read,	 register464_write);
DEVICE_ATTR (register465,	 0664, 	 register465_read,	 register465_write);
DEVICE_ATTR (register466,	 0664, 	 register466_read,	 register466_write);
DEVICE_ATTR (register467,	 0664, 	 register467_read,	 register467_write);
DEVICE_ATTR (register468,	 0664, 	 register468_read,	 register468_write);
DEVICE_ATTR (register469,	 0664, 	 register469_read,	 register469_write);
DEVICE_ATTR (register470,	 0664, 	 register470_read,	 register470_write);
DEVICE_ATTR (register471,	 0664, 	 register471_read,	 register471_write);
DEVICE_ATTR (register472,	 0664, 	 register472_read,	 register472_write);
DEVICE_ATTR (register473,	 0664, 	 register473_read,	 register473_write);
DEVICE_ATTR (register474,	 0664, 	 register474_read,	 register474_write);
DEVICE_ATTR (register475,	 0664, 	 register475_read,	 register475_write);
DEVICE_ATTR (register476,	 0664, 	 register476_read,	 register476_write);
DEVICE_ATTR (register477,	 0664, 	 register477_read,	 register477_write);
DEVICE_ATTR (register478,	 0664, 	 register478_read,	 register478_write);
DEVICE_ATTR (register479,	 0664, 	 register479_read,	 register479_write);
DEVICE_ATTR (register480,	 0664, 	 register480_read,	 register480_write);
DEVICE_ATTR (register481,	 0664, 	 register481_read,	 register481_write);
DEVICE_ATTR (register482,	 0664, 	 register482_read,	 register482_write);
DEVICE_ATTR (register483,	 0664, 	 register483_read,	 register483_write);
DEVICE_ATTR (register484,	 0664, 	 register484_read,	 register484_write);
DEVICE_ATTR (register485,	 0664, 	 register485_read,	 register485_write);
DEVICE_ATTR (register486,	 0664, 	 register486_read,	 register486_write);
DEVICE_ATTR (register487,	 0664, 	 register487_read,	 register487_write);
DEVICE_ATTR (register488,	 0664, 	 register488_read,	 register488_write);
DEVICE_ATTR (register489,	 0664, 	 register489_read,	 register489_write);
DEVICE_ATTR (register490,	 0664, 	 register490_read,	 register490_write);
DEVICE_ATTR (register491,	 0664, 	 register491_read,	 register491_write);
DEVICE_ATTR (register492,	 0664, 	 register492_read,	 register492_write);
DEVICE_ATTR (register493,	 0664, 	 register493_read,	 register493_write);
DEVICE_ATTR (register494,	 0664, 	 register494_read,	 register494_write);
DEVICE_ATTR (register495,	 0664, 	 register495_read,	 register495_write);
DEVICE_ATTR (register496,	 0664, 	 register496_read,	 register496_write);
DEVICE_ATTR (register497,	 0664, 	 register497_read,	 register497_write);
DEVICE_ATTR (register498,	 0664, 	 register498_read,	 register498_write);
DEVICE_ATTR (register499,	 0664, 	 register499_read,	 register499_write);
DEVICE_ATTR (register500,	 0664, 	 register500_read,	 register500_write);
DEVICE_ATTR (register501,	 0664, 	 register501_read,	 register501_write);
DEVICE_ATTR (register502,	 0664, 	 register502_read,	 register502_write);
DEVICE_ATTR (register503,	 0664, 	 register503_read,	 register503_write);
DEVICE_ATTR (register504,	 0664, 	 register504_read,	 register504_write);
DEVICE_ATTR (register505,	 0664, 	 register505_read,	 register505_write);
DEVICE_ATTR (register506,	 0664, 	 register506_read,	 register506_write);
DEVICE_ATTR (register507,	 0664, 	 register507_read,	 register507_write);
DEVICE_ATTR (register508,	 0664, 	 register508_read,	 register508_write);
DEVICE_ATTR (register509,	 0664, 	 register509_read,	 register509_write);
DEVICE_ATTR (register510,	 0664, 	 register510_read,	 register510_write);
DEVICE_ATTR (register511,	 0664, 	 register511_read,	 register511_write);
DEVICE_ATTR (name, 			 0444, 	 name_read, 		 NULL);

/* Device struct */
struct fe_DPRAM_dev {
  struct cdev cdev;
  char *name;
  void __iomem *regs;
  int register0;
  int register1;
  int register2;
  int register3;
  int register4;
  int register5;
  int register6;
  int register7;
  int register8;
  int register9;
  int register10;
  int register11;
  int register12;
  int register13;
  int register14;
  int register15;
  int register16;
  int register17;
  int register18;
  int register19;
  int register20;
  int register21;
  int register22;
  int register23;
  int register24;
  int register25;
  int register26;
  int register27;
  int register28;
  int register29;
  int register30;
  int register31;
  int register32;
  int register33;
  int register34;
  int register35;
  int register36;
  int register37;
  int register38;
  int register39;
  int register40;
  int register41;
  int register42;
  int register43;
  int register44;
  int register45;
  int register46;
  int register47;
  int register48;
  int register49;
  int register50;
  int register51;
  int register52;
  int register53;
  int register54;
  int register55;
  int register56;
  int register57;
  int register58;
  int register59;
  int register60;
  int register61;
  int register62;
  int register63;
  int register64;
  int register65;
  int register66;
  int register67;
  int register68;
  int register69;
  int register70;
  int register71;
  int register72;
  int register73;
  int register74;
  int register75;
  int register76;
  int register77;
  int register78;
  int register79;
  int register80;
  int register81;
  int register82;
  int register83;
  int register84;
  int register85;
  int register86;
  int register87;
  int register88;
  int register89;
  int register90;
  int register91;
  int register92;
  int register93;
  int register94;
  int register95;
  int register96;
  int register97;
  int register98;
  int register99;
  int register100;
  int register101;
  int register102;
  int register103;
  int register104;
  int register105;
  int register106;
  int register107;
  int register108;
  int register109;
  int register110;
  int register111;
  int register112;
  int register113;
  int register114;
  int register115;
  int register116;
  int register117;
  int register118;
  int register119;
  int register120;
  int register121;
  int register122;
  int register123;
  int register124;
  int register125;
  int register126;
  int register127;
  int register128;
  int register129;
  int register130;
  int register131;
  int register132;
  int register133;
  int register134;
  int register135;
  int register136;
  int register137;
  int register138;
  int register139;
  int register140;
  int register141;
  int register142;
  int register143;
  int register144;
  int register145;
  int register146;
  int register147;
  int register148;
  int register149;
  int register150;
  int register151;
  int register152;
  int register153;
  int register154;
  int register155;
  int register156;
  int register157;
  int register158;
  int register159;
  int register160;
  int register161;
  int register162;
  int register163;
  int register164;
  int register165;
  int register166;
  int register167;
  int register168;
  int register169;
  int register170;
  int register171;
  int register172;
  int register173;
  int register174;
  int register175;
  int register176;
  int register177;
  int register178;
  int register179;
  int register180;
  int register181;
  int register182;
  int register183;
  int register184;
  int register185;
  int register186;
  int register187;
  int register188;
  int register189;
  int register190;
  int register191;
  int register192;
  int register193;
  int register194;
  int register195;
  int register196;
  int register197;
  int register198;
  int register199;
  int register200;
  int register201;
  int register202;
  int register203;
  int register204;
  int register205;
  int register206;
  int register207;
  int register208;
  int register209;
  int register210;
  int register211;
  int register212;
  int register213;
  int register214;
  int register215;
  int register216;
  int register217;
  int register218;
  int register219;
  int register220;
  int register221;
  int register222;
  int register223;
  int register224;
  int register225;
  int register226;
  int register227;
  int register228;
  int register229;
  int register230;
  int register231;
  int register232;
  int register233;
  int register234;
  int register235;
  int register236;
  int register237;
  int register238;
  int register239;
  int register240;
  int register241;
  int register242;
  int register243;
  int register244;
  int register245;
  int register246;
  int register247;
  int register248;
  int register249;
  int register250;
  int register251;
  int register252;
  int register253;
  int register254;
  int register255;
  int register256;
  int register257;
  int register258;
  int register259;
  int register260;
  int register261;
  int register262;
  int register263;
  int register264;
  int register265;
  int register266;
  int register267;
  int register268;
  int register269;
  int register270;
  int register271;
  int register272;
  int register273;
  int register274;
  int register275;
  int register276;
  int register277;
  int register278;
  int register279;
  int register280;
  int register281;
  int register282;
  int register283;
  int register284;
  int register285;
  int register286;
  int register287;
  int register288;
  int register289;
  int register290;
  int register291;
  int register292;
  int register293;
  int register294;
  int register295;
  int register296;
  int register297;
  int register298;
  int register299;
  int register300;
  int register301;
  int register302;
  int register303;
  int register304;
  int register305;
  int register306;
  int register307;
  int register308;
  int register309;
  int register310;
  int register311;
  int register312;
  int register313;
  int register314;
  int register315;
  int register316;
  int register317;
  int register318;
  int register319;
  int register320;
  int register321;
  int register322;
  int register323;
  int register324;
  int register325;
  int register326;
  int register327;
  int register328;
  int register329;
  int register330;
  int register331;
  int register332;
  int register333;
  int register334;
  int register335;
  int register336;
  int register337;
  int register338;
  int register339;
  int register340;
  int register341;
  int register342;
  int register343;
  int register344;
  int register345;
  int register346;
  int register347;
  int register348;
  int register349;
  int register350;
  int register351;
  int register352;
  int register353;
  int register354;
  int register355;
  int register356;
  int register357;
  int register358;
  int register359;
  int register360;
  int register361;
  int register362;
  int register363;
  int register364;
  int register365;
  int register366;
  int register367;
  int register368;
  int register369;
  int register370;
  int register371;
  int register372;
  int register373;
  int register374;
  int register375;
  int register376;
  int register377;
  int register378;
  int register379;
  int register380;
  int register381;
  int register382;
  int register383;
  int register384;
  int register385;
  int register386;
  int register387;
  int register388;
  int register389;
  int register390;
  int register391;
  int register392;
  int register393;
  int register394;
  int register395;
  int register396;
  int register397;
  int register398;
  int register399;
  int register400;
  int register401;
  int register402;
  int register403;
  int register404;
  int register405;
  int register406;
  int register407;
  int register408;
  int register409;
  int register410;
  int register411;
  int register412;
  int register413;
  int register414;
  int register415;
  int register416;
  int register417;
  int register418;
  int register419;
  int register420;
  int register421;
  int register422;
  int register423;
  int register424;
  int register425;
  int register426;
  int register427;
  int register428;
  int register429;
  int register430;
  int register431;
  int register432;
  int register433;
  int register434;
  int register435;
  int register436;
  int register437;
  int register438;
  int register439;
  int register440;
  int register441;
  int register442;
  int register443;
  int register444;
  int register445;
  int register446;
  int register447;
  int register448;
  int register449;
  int register450;
  int register451;
  int register452;
  int register453;
  int register454;
  int register455;
  int register456;
  int register457;
  int register458;
  int register459;
  int register460;
  int register461;
  int register462;
  int register463;
  int register464;
  int register465;
  int register466;
  int register467;
  int register468;
  int register469;
  int register470;
  int register471;
  int register472;
  int register473;
  int register474;
  int register475;
  int register476;
  int register477;
  int register478;
  int register479;
  int register480;
  int register481;
  int register482;
  int register483;
  int register484;
  int register485;
  int register486;
  int register487;
  int register488;
  int register489;
  int register490;
  int register491;
  int register492;
  int register493;
  int register494;
  int register495;
  int register496;
  int register497;
  int register498;
  int register499;
  int register500;
  int register501;
  int register502;
  int register503;
  int register504;
  int register505;
  int register506;
  int register507;
  int register508;
  int register509;
  int register510;
  int register511;
};
typedef struct fe_DPRAM_dev fe_DPRAM_dev_t;

/* Global Variables */
static struct class *cl;  // Global variable for the device class
static dev_t dev_num;

/* ID Matching struct to match driver to tree */
static struct of_device_id fe_DPRAM_dt_ids[] = {
  {
    .compatible = "dev,fe-dpram"
  },
  { }
};
MODULE_DEVICE_TABLE(of, fe_DPRAM_dt_ids);

/* Platform driver struct */
static struct platform_driver DPRAM_platform = {
  .probe = DPRAM_probe,
  .remove = DPRAM_remove,
  .driver = {
    .name = "Flat Earth Dual Port RAM Driver",
    .owner = THIS_MODULE,
    .of_match_table = fe_DPRAM_dt_ids
  }
};

/* File ops struct */
static const struct file_operations fe_DPRAM_fops = {
  .owner = THIS_MODULE,
  .read = DPRAM_read,
  .write = DPRAM_write,
  .open = DPRAM_open,
  .release = DPRAM_release
};

/* Initialize device driver */
static int DPRAM_init(void) {
  int ret_val = 0;

  pr_info("Initializing the Flat Earth Dual Port RAM module...\n");

  pr_info("Probe function start...\n");

  // Register our driver with the "Platform Driver" bus
  ret_val = platform_driver_register(&DPRAM_platform);  
  if (ret_val != 0) {
    pr_err("platform_driver_register returned %d\n", ret_val);
    return ret_val;
  }
  pr_info("Flat Earth Dual Port RAM module successfully initialized!\n");
  
  return 0;
}

/* Probe function of device driver */
static int DPRAM_probe(struct platform_device *pdev) {
  
  int ret_val = -EBUSY;

  char deviceName[20] = "fe_DPRAM_";
  char deviceMinor[20];
  int status;
 
  struct device *device_obj;
  fe_DPRAM_dev_t * fe_DPRAM_devp;
 
  pr_info("DPRAM_probe enter...\n");
 
  struct resource *r = 0;
  r = platform_get_resource(pdev, IORESOURCE_MEM, 0); 
  if (r == NULL) {
    pr_err("IORESOURCE_MEM (register space) does not exist\n");
    goto bad_exit_return;  }
  
  // Create structure to hold device information
  fe_DPRAM_devp = devm_kzalloc(&pdev->dev, sizeof(fe_DPRAM_dev_t), GFP_KERNEL);
  fe_DPRAM_devp->regs = devm_ioremap_resource(&pdev->dev, r);
  if (IS_ERR(fe_DPRAM_devp->regs))
    goto bad_ioremap;
  
  platform_set_drvdata(pdev, (void *)fe_DPRAM_devp);
  fe_DPRAM_devp->name = devm_kzalloc(&pdev->dev, 50, GFP_KERNEL);
  if (fe_DPRAM_devp->name == NULL)
    goto bad_mem_alloc;
  
  strcpy(fe_DPRAM_devp->name, (char *)pdev->name);
  pr_info("%s\n", (char *)pdev->name);
  status = alloc_chrdev_region(&dev_num, 0, 1, "fe_DPRAM_");
  if (status != 0)
    goto bad_alloc_chrdev_region;
  
  sprintf(deviceMinor, "%d", MAJOR(dev_num));
  strcat(deviceName, deviceMinor);
  pr_info("%s\n", deviceName);
  cl = class_create(THIS_MODULE, deviceName);
  if (cl == NULL)
    goto bad_class_create;
  
  cdev_init(&fe_DPRAM_devp->cdev, &fe_DPRAM_fops);
  status = cdev_add(&fe_DPRAM_devp->cdev, dev_num, 1);
  if (status != 0)
    goto bad_cdev_add;
  
  device_obj = device_create(cl, NULL, dev_num, NULL, deviceName);
  if (device_obj == NULL)
    goto bad_device_create;

  dev_set_drvdata(device_obj, fe_DPRAM_devp);  

  // Check each register value, upon initialization reset the component
    status = device_create_file(device_obj, &dev_attr_register0);
  if(status)
    goto bad_device_create_file_1;

  status = device_create_file(device_obj, &dev_attr_register1);
  if(status)
    goto bad_device_create_file_2;

  status = device_create_file(device_obj, &dev_attr_register2);
  if(status)
    goto bad_device_create_file_3;

  status = device_create_file(device_obj, &dev_attr_register3);
  if(status)
    goto bad_device_create_file_4;

  status = device_create_file(device_obj, &dev_attr_register4);
  if(status)
    goto bad_device_create_file_5;

  status = device_create_file(device_obj, &dev_attr_register5);
  if(status)
    goto bad_device_create_file_6;

  status = device_create_file(device_obj, &dev_attr_register6);
  if(status)
    goto bad_device_create_file_7;

  status = device_create_file(device_obj, &dev_attr_register7);
  if(status)
    goto bad_device_create_file_8;

  status = device_create_file(device_obj, &dev_attr_register8);
  if(status)
    goto bad_device_create_file_9;

  status = device_create_file(device_obj, &dev_attr_register9);
  if(status)
    goto bad_device_create_file_10;

  status = device_create_file(device_obj, &dev_attr_register10);
  if(status)
    goto bad_device_create_file_11;

  status = device_create_file(device_obj, &dev_attr_register11);
  if(status)
    goto bad_device_create_file_12;

  status = device_create_file(device_obj, &dev_attr_register12);
  if(status)
    goto bad_device_create_file_13;

  status = device_create_file(device_obj, &dev_attr_register13);
  if(status)
    goto bad_device_create_file_14;

  status = device_create_file(device_obj, &dev_attr_register14);
  if(status)
    goto bad_device_create_file_15;

  status = device_create_file(device_obj, &dev_attr_register15);
  if(status)
    goto bad_device_create_file_16;

  status = device_create_file(device_obj, &dev_attr_register16);
  if(status)
    goto bad_device_create_file_17;

  status = device_create_file(device_obj, &dev_attr_register17);
  if(status)
    goto bad_device_create_file_18;

  status = device_create_file(device_obj, &dev_attr_register18);
  if(status)
    goto bad_device_create_file_19;

  status = device_create_file(device_obj, &dev_attr_register19);
  if(status)
    goto bad_device_create_file_20;

  status = device_create_file(device_obj, &dev_attr_register20);
  if(status)
    goto bad_device_create_file_21;

  status = device_create_file(device_obj, &dev_attr_register21);
  if(status)
    goto bad_device_create_file_22;

  status = device_create_file(device_obj, &dev_attr_register22);
  if(status)
    goto bad_device_create_file_23;

  status = device_create_file(device_obj, &dev_attr_register23);
  if(status)
    goto bad_device_create_file_24;

  status = device_create_file(device_obj, &dev_attr_register24);
  if(status)
    goto bad_device_create_file_25;

  status = device_create_file(device_obj, &dev_attr_register25);
  if(status)
    goto bad_device_create_file_26;

  status = device_create_file(device_obj, &dev_attr_register26);
  if(status)
    goto bad_device_create_file_27;

  status = device_create_file(device_obj, &dev_attr_register27);
  if(status)
    goto bad_device_create_file_28;

  status = device_create_file(device_obj, &dev_attr_register28);
  if(status)
    goto bad_device_create_file_29;

  status = device_create_file(device_obj, &dev_attr_register29);
  if(status)
    goto bad_device_create_file_30;

  status = device_create_file(device_obj, &dev_attr_register30);
  if(status)
    goto bad_device_create_file_31;

  status = device_create_file(device_obj, &dev_attr_register31);
  if(status)
    goto bad_device_create_file_32;

  status = device_create_file(device_obj, &dev_attr_register32);
  if(status)
    goto bad_device_create_file_33;

  status = device_create_file(device_obj, &dev_attr_register33);
  if(status)
    goto bad_device_create_file_34;

  status = device_create_file(device_obj, &dev_attr_register34);
  if(status)
    goto bad_device_create_file_35;

  status = device_create_file(device_obj, &dev_attr_register35);
  if(status)
    goto bad_device_create_file_36;

  status = device_create_file(device_obj, &dev_attr_register36);
  if(status)
    goto bad_device_create_file_37;

  status = device_create_file(device_obj, &dev_attr_register37);
  if(status)
    goto bad_device_create_file_38;

  status = device_create_file(device_obj, &dev_attr_register38);
  if(status)
    goto bad_device_create_file_39;

  status = device_create_file(device_obj, &dev_attr_register39);
  if(status)
    goto bad_device_create_file_40;

  status = device_create_file(device_obj, &dev_attr_register40);
  if(status)
    goto bad_device_create_file_41;

  status = device_create_file(device_obj, &dev_attr_register41);
  if(status)
    goto bad_device_create_file_42;

  status = device_create_file(device_obj, &dev_attr_register42);
  if(status)
    goto bad_device_create_file_43;

  status = device_create_file(device_obj, &dev_attr_register43);
  if(status)
    goto bad_device_create_file_44;

  status = device_create_file(device_obj, &dev_attr_register44);
  if(status)
    goto bad_device_create_file_45;

  status = device_create_file(device_obj, &dev_attr_register45);
  if(status)
    goto bad_device_create_file_46;

  status = device_create_file(device_obj, &dev_attr_register46);
  if(status)
    goto bad_device_create_file_47;

  status = device_create_file(device_obj, &dev_attr_register47);
  if(status)
    goto bad_device_create_file_48;

  status = device_create_file(device_obj, &dev_attr_register48);
  if(status)
    goto bad_device_create_file_49;

  status = device_create_file(device_obj, &dev_attr_register49);
  if(status)
    goto bad_device_create_file_50;

  status = device_create_file(device_obj, &dev_attr_register50);
  if(status)
    goto bad_device_create_file_51;

  status = device_create_file(device_obj, &dev_attr_register51);
  if(status)
    goto bad_device_create_file_52;

  status = device_create_file(device_obj, &dev_attr_register52);
  if(status)
    goto bad_device_create_file_53;

  status = device_create_file(device_obj, &dev_attr_register53);
  if(status)
    goto bad_device_create_file_54;

  status = device_create_file(device_obj, &dev_attr_register54);
  if(status)
    goto bad_device_create_file_55;

  status = device_create_file(device_obj, &dev_attr_register55);
  if(status)
    goto bad_device_create_file_56;

  status = device_create_file(device_obj, &dev_attr_register56);
  if(status)
    goto bad_device_create_file_57;

  status = device_create_file(device_obj, &dev_attr_register57);
  if(status)
    goto bad_device_create_file_58;

  status = device_create_file(device_obj, &dev_attr_register58);
  if(status)
    goto bad_device_create_file_59;

  status = device_create_file(device_obj, &dev_attr_register59);
  if(status)
    goto bad_device_create_file_60;

  status = device_create_file(device_obj, &dev_attr_register60);
  if(status)
    goto bad_device_create_file_61;

  status = device_create_file(device_obj, &dev_attr_register61);
  if(status)
    goto bad_device_create_file_62;

  status = device_create_file(device_obj, &dev_attr_register62);
  if(status)
    goto bad_device_create_file_63;

  status = device_create_file(device_obj, &dev_attr_register63);
  if(status)
    goto bad_device_create_file_64;

  status = device_create_file(device_obj, &dev_attr_register64);
  if(status)
    goto bad_device_create_file_65;

  status = device_create_file(device_obj, &dev_attr_register65);
  if(status)
    goto bad_device_create_file_66;

  status = device_create_file(device_obj, &dev_attr_register66);
  if(status)
    goto bad_device_create_file_67;

  status = device_create_file(device_obj, &dev_attr_register67);
  if(status)
    goto bad_device_create_file_68;

  status = device_create_file(device_obj, &dev_attr_register68);
  if(status)
    goto bad_device_create_file_69;

  status = device_create_file(device_obj, &dev_attr_register69);
  if(status)
    goto bad_device_create_file_70;

  status = device_create_file(device_obj, &dev_attr_register70);
  if(status)
    goto bad_device_create_file_71;

  status = device_create_file(device_obj, &dev_attr_register71);
  if(status)
    goto bad_device_create_file_72;

  status = device_create_file(device_obj, &dev_attr_register72);
  if(status)
    goto bad_device_create_file_73;

  status = device_create_file(device_obj, &dev_attr_register73);
  if(status)
    goto bad_device_create_file_74;

  status = device_create_file(device_obj, &dev_attr_register74);
  if(status)
    goto bad_device_create_file_75;

  status = device_create_file(device_obj, &dev_attr_register75);
  if(status)
    goto bad_device_create_file_76;

  status = device_create_file(device_obj, &dev_attr_register76);
  if(status)
    goto bad_device_create_file_77;

  status = device_create_file(device_obj, &dev_attr_register77);
  if(status)
    goto bad_device_create_file_78;

  status = device_create_file(device_obj, &dev_attr_register78);
  if(status)
    goto bad_device_create_file_79;

  status = device_create_file(device_obj, &dev_attr_register79);
  if(status)
    goto bad_device_create_file_80;

  status = device_create_file(device_obj, &dev_attr_register80);
  if(status)
    goto bad_device_create_file_81;

  status = device_create_file(device_obj, &dev_attr_register81);
  if(status)
    goto bad_device_create_file_82;

  status = device_create_file(device_obj, &dev_attr_register82);
  if(status)
    goto bad_device_create_file_83;

  status = device_create_file(device_obj, &dev_attr_register83);
  if(status)
    goto bad_device_create_file_84;

  status = device_create_file(device_obj, &dev_attr_register84);
  if(status)
    goto bad_device_create_file_85;

  status = device_create_file(device_obj, &dev_attr_register85);
  if(status)
    goto bad_device_create_file_86;

  status = device_create_file(device_obj, &dev_attr_register86);
  if(status)
    goto bad_device_create_file_87;

  status = device_create_file(device_obj, &dev_attr_register87);
  if(status)
    goto bad_device_create_file_88;

  status = device_create_file(device_obj, &dev_attr_register88);
  if(status)
    goto bad_device_create_file_89;

  status = device_create_file(device_obj, &dev_attr_register89);
  if(status)
    goto bad_device_create_file_90;

  status = device_create_file(device_obj, &dev_attr_register90);
  if(status)
    goto bad_device_create_file_91;

  status = device_create_file(device_obj, &dev_attr_register91);
  if(status)
    goto bad_device_create_file_92;

  status = device_create_file(device_obj, &dev_attr_register92);
  if(status)
    goto bad_device_create_file_93;

  status = device_create_file(device_obj, &dev_attr_register93);
  if(status)
    goto bad_device_create_file_94;

  status = device_create_file(device_obj, &dev_attr_register94);
  if(status)
    goto bad_device_create_file_95;

  status = device_create_file(device_obj, &dev_attr_register95);
  if(status)
    goto bad_device_create_file_96;

  status = device_create_file(device_obj, &dev_attr_register96);
  if(status)
    goto bad_device_create_file_97;

  status = device_create_file(device_obj, &dev_attr_register97);
  if(status)
    goto bad_device_create_file_98;

  status = device_create_file(device_obj, &dev_attr_register98);
  if(status)
    goto bad_device_create_file_99;

  status = device_create_file(device_obj, &dev_attr_register99);
  if(status)
    goto bad_device_create_file_100;

  status = device_create_file(device_obj, &dev_attr_register100);
  if(status)
    goto bad_device_create_file_101;

  status = device_create_file(device_obj, &dev_attr_register101);
  if(status)
    goto bad_device_create_file_102;

  status = device_create_file(device_obj, &dev_attr_register102);
  if(status)
    goto bad_device_create_file_103;

  status = device_create_file(device_obj, &dev_attr_register103);
  if(status)
    goto bad_device_create_file_104;

  status = device_create_file(device_obj, &dev_attr_register104);
  if(status)
    goto bad_device_create_file_105;

  status = device_create_file(device_obj, &dev_attr_register105);
  if(status)
    goto bad_device_create_file_106;

  status = device_create_file(device_obj, &dev_attr_register106);
  if(status)
    goto bad_device_create_file_107;

  status = device_create_file(device_obj, &dev_attr_register107);
  if(status)
    goto bad_device_create_file_108;

  status = device_create_file(device_obj, &dev_attr_register108);
  if(status)
    goto bad_device_create_file_109;

  status = device_create_file(device_obj, &dev_attr_register109);
  if(status)
    goto bad_device_create_file_110;

  status = device_create_file(device_obj, &dev_attr_register110);
  if(status)
    goto bad_device_create_file_111;

  status = device_create_file(device_obj, &dev_attr_register111);
  if(status)
    goto bad_device_create_file_112;

  status = device_create_file(device_obj, &dev_attr_register112);
  if(status)
    goto bad_device_create_file_113;

  status = device_create_file(device_obj, &dev_attr_register113);
  if(status)
    goto bad_device_create_file_114;

  status = device_create_file(device_obj, &dev_attr_register114);
  if(status)
    goto bad_device_create_file_115;

  status = device_create_file(device_obj, &dev_attr_register115);
  if(status)
    goto bad_device_create_file_116;

  status = device_create_file(device_obj, &dev_attr_register116);
  if(status)
    goto bad_device_create_file_117;

  status = device_create_file(device_obj, &dev_attr_register117);
  if(status)
    goto bad_device_create_file_118;

  status = device_create_file(device_obj, &dev_attr_register118);
  if(status)
    goto bad_device_create_file_119;

  status = device_create_file(device_obj, &dev_attr_register119);
  if(status)
    goto bad_device_create_file_120;

  status = device_create_file(device_obj, &dev_attr_register120);
  if(status)
    goto bad_device_create_file_121;

  status = device_create_file(device_obj, &dev_attr_register121);
  if(status)
    goto bad_device_create_file_122;

  status = device_create_file(device_obj, &dev_attr_register122);
  if(status)
    goto bad_device_create_file_123;

  status = device_create_file(device_obj, &dev_attr_register123);
  if(status)
    goto bad_device_create_file_124;

  status = device_create_file(device_obj, &dev_attr_register124);
  if(status)
    goto bad_device_create_file_125;

  status = device_create_file(device_obj, &dev_attr_register125);
  if(status)
    goto bad_device_create_file_126;

  status = device_create_file(device_obj, &dev_attr_register126);
  if(status)
    goto bad_device_create_file_127;

  status = device_create_file(device_obj, &dev_attr_register127);
  if(status)
    goto bad_device_create_file_128;

  status = device_create_file(device_obj, &dev_attr_register128);
  if(status)
    goto bad_device_create_file_129;

  status = device_create_file(device_obj, &dev_attr_register129);
  if(status)
    goto bad_device_create_file_130;

  status = device_create_file(device_obj, &dev_attr_register130);
  if(status)
    goto bad_device_create_file_131;

  status = device_create_file(device_obj, &dev_attr_register131);
  if(status)
    goto bad_device_create_file_132;

  status = device_create_file(device_obj, &dev_attr_register132);
  if(status)
    goto bad_device_create_file_133;

  status = device_create_file(device_obj, &dev_attr_register133);
  if(status)
    goto bad_device_create_file_134;

  status = device_create_file(device_obj, &dev_attr_register134);
  if(status)
    goto bad_device_create_file_135;

  status = device_create_file(device_obj, &dev_attr_register135);
  if(status)
    goto bad_device_create_file_136;

  status = device_create_file(device_obj, &dev_attr_register136);
  if(status)
    goto bad_device_create_file_137;

  status = device_create_file(device_obj, &dev_attr_register137);
  if(status)
    goto bad_device_create_file_138;

  status = device_create_file(device_obj, &dev_attr_register138);
  if(status)
    goto bad_device_create_file_139;

  status = device_create_file(device_obj, &dev_attr_register139);
  if(status)
    goto bad_device_create_file_140;

  status = device_create_file(device_obj, &dev_attr_register140);
  if(status)
    goto bad_device_create_file_141;

  status = device_create_file(device_obj, &dev_attr_register141);
  if(status)
    goto bad_device_create_file_142;

  status = device_create_file(device_obj, &dev_attr_register142);
  if(status)
    goto bad_device_create_file_143;

  status = device_create_file(device_obj, &dev_attr_register143);
  if(status)
    goto bad_device_create_file_144;

  status = device_create_file(device_obj, &dev_attr_register144);
  if(status)
    goto bad_device_create_file_145;

  status = device_create_file(device_obj, &dev_attr_register145);
  if(status)
    goto bad_device_create_file_146;

  status = device_create_file(device_obj, &dev_attr_register146);
  if(status)
    goto bad_device_create_file_147;

  status = device_create_file(device_obj, &dev_attr_register147);
  if(status)
    goto bad_device_create_file_148;

  status = device_create_file(device_obj, &dev_attr_register148);
  if(status)
    goto bad_device_create_file_149;

  status = device_create_file(device_obj, &dev_attr_register149);
  if(status)
    goto bad_device_create_file_150;

  status = device_create_file(device_obj, &dev_attr_register150);
  if(status)
    goto bad_device_create_file_151;

  status = device_create_file(device_obj, &dev_attr_register151);
  if(status)
    goto bad_device_create_file_152;

  status = device_create_file(device_obj, &dev_attr_register152);
  if(status)
    goto bad_device_create_file_153;

  status = device_create_file(device_obj, &dev_attr_register153);
  if(status)
    goto bad_device_create_file_154;

  status = device_create_file(device_obj, &dev_attr_register154);
  if(status)
    goto bad_device_create_file_155;

  status = device_create_file(device_obj, &dev_attr_register155);
  if(status)
    goto bad_device_create_file_156;

  status = device_create_file(device_obj, &dev_attr_register156);
  if(status)
    goto bad_device_create_file_157;

  status = device_create_file(device_obj, &dev_attr_register157);
  if(status)
    goto bad_device_create_file_158;

  status = device_create_file(device_obj, &dev_attr_register158);
  if(status)
    goto bad_device_create_file_159;

  status = device_create_file(device_obj, &dev_attr_register159);
  if(status)
    goto bad_device_create_file_160;

  status = device_create_file(device_obj, &dev_attr_register160);
  if(status)
    goto bad_device_create_file_161;

  status = device_create_file(device_obj, &dev_attr_register161);
  if(status)
    goto bad_device_create_file_162;

  status = device_create_file(device_obj, &dev_attr_register162);
  if(status)
    goto bad_device_create_file_163;

  status = device_create_file(device_obj, &dev_attr_register163);
  if(status)
    goto bad_device_create_file_164;

  status = device_create_file(device_obj, &dev_attr_register164);
  if(status)
    goto bad_device_create_file_165;

  status = device_create_file(device_obj, &dev_attr_register165);
  if(status)
    goto bad_device_create_file_166;

  status = device_create_file(device_obj, &dev_attr_register166);
  if(status)
    goto bad_device_create_file_167;

  status = device_create_file(device_obj, &dev_attr_register167);
  if(status)
    goto bad_device_create_file_168;

  status = device_create_file(device_obj, &dev_attr_register168);
  if(status)
    goto bad_device_create_file_169;

  status = device_create_file(device_obj, &dev_attr_register169);
  if(status)
    goto bad_device_create_file_170;

  status = device_create_file(device_obj, &dev_attr_register170);
  if(status)
    goto bad_device_create_file_171;

  status = device_create_file(device_obj, &dev_attr_register171);
  if(status)
    goto bad_device_create_file_172;

  status = device_create_file(device_obj, &dev_attr_register172);
  if(status)
    goto bad_device_create_file_173;

  status = device_create_file(device_obj, &dev_attr_register173);
  if(status)
    goto bad_device_create_file_174;

  status = device_create_file(device_obj, &dev_attr_register174);
  if(status)
    goto bad_device_create_file_175;

  status = device_create_file(device_obj, &dev_attr_register175);
  if(status)
    goto bad_device_create_file_176;

  status = device_create_file(device_obj, &dev_attr_register176);
  if(status)
    goto bad_device_create_file_177;

  status = device_create_file(device_obj, &dev_attr_register177);
  if(status)
    goto bad_device_create_file_178;

  status = device_create_file(device_obj, &dev_attr_register178);
  if(status)
    goto bad_device_create_file_179;

  status = device_create_file(device_obj, &dev_attr_register179);
  if(status)
    goto bad_device_create_file_180;

  status = device_create_file(device_obj, &dev_attr_register180);
  if(status)
    goto bad_device_create_file_181;

  status = device_create_file(device_obj, &dev_attr_register181);
  if(status)
    goto bad_device_create_file_182;

  status = device_create_file(device_obj, &dev_attr_register182);
  if(status)
    goto bad_device_create_file_183;

  status = device_create_file(device_obj, &dev_attr_register183);
  if(status)
    goto bad_device_create_file_184;

  status = device_create_file(device_obj, &dev_attr_register184);
  if(status)
    goto bad_device_create_file_185;

  status = device_create_file(device_obj, &dev_attr_register185);
  if(status)
    goto bad_device_create_file_186;

  status = device_create_file(device_obj, &dev_attr_register186);
  if(status)
    goto bad_device_create_file_187;

  status = device_create_file(device_obj, &dev_attr_register187);
  if(status)
    goto bad_device_create_file_188;

  status = device_create_file(device_obj, &dev_attr_register188);
  if(status)
    goto bad_device_create_file_189;

  status = device_create_file(device_obj, &dev_attr_register189);
  if(status)
    goto bad_device_create_file_190;

  status = device_create_file(device_obj, &dev_attr_register190);
  if(status)
    goto bad_device_create_file_191;

  status = device_create_file(device_obj, &dev_attr_register191);
  if(status)
    goto bad_device_create_file_192;

  status = device_create_file(device_obj, &dev_attr_register192);
  if(status)
    goto bad_device_create_file_193;

  status = device_create_file(device_obj, &dev_attr_register193);
  if(status)
    goto bad_device_create_file_194;

  status = device_create_file(device_obj, &dev_attr_register194);
  if(status)
    goto bad_device_create_file_195;

  status = device_create_file(device_obj, &dev_attr_register195);
  if(status)
    goto bad_device_create_file_196;

  status = device_create_file(device_obj, &dev_attr_register196);
  if(status)
    goto bad_device_create_file_197;

  status = device_create_file(device_obj, &dev_attr_register197);
  if(status)
    goto bad_device_create_file_198;

  status = device_create_file(device_obj, &dev_attr_register198);
  if(status)
    goto bad_device_create_file_199;

  status = device_create_file(device_obj, &dev_attr_register199);
  if(status)
    goto bad_device_create_file_200;

  status = device_create_file(device_obj, &dev_attr_register200);
  if(status)
    goto bad_device_create_file_201;

  status = device_create_file(device_obj, &dev_attr_register201);
  if(status)
    goto bad_device_create_file_202;

  status = device_create_file(device_obj, &dev_attr_register202);
  if(status)
    goto bad_device_create_file_203;

  status = device_create_file(device_obj, &dev_attr_register203);
  if(status)
    goto bad_device_create_file_204;

  status = device_create_file(device_obj, &dev_attr_register204);
  if(status)
    goto bad_device_create_file_205;

  status = device_create_file(device_obj, &dev_attr_register205);
  if(status)
    goto bad_device_create_file_206;

  status = device_create_file(device_obj, &dev_attr_register206);
  if(status)
    goto bad_device_create_file_207;

  status = device_create_file(device_obj, &dev_attr_register207);
  if(status)
    goto bad_device_create_file_208;

  status = device_create_file(device_obj, &dev_attr_register208);
  if(status)
    goto bad_device_create_file_209;

  status = device_create_file(device_obj, &dev_attr_register209);
  if(status)
    goto bad_device_create_file_210;

  status = device_create_file(device_obj, &dev_attr_register210);
  if(status)
    goto bad_device_create_file_211;

  status = device_create_file(device_obj, &dev_attr_register211);
  if(status)
    goto bad_device_create_file_212;

  status = device_create_file(device_obj, &dev_attr_register212);
  if(status)
    goto bad_device_create_file_213;

  status = device_create_file(device_obj, &dev_attr_register213);
  if(status)
    goto bad_device_create_file_214;

  status = device_create_file(device_obj, &dev_attr_register214);
  if(status)
    goto bad_device_create_file_215;

  status = device_create_file(device_obj, &dev_attr_register215);
  if(status)
    goto bad_device_create_file_216;

  status = device_create_file(device_obj, &dev_attr_register216);
  if(status)
    goto bad_device_create_file_217;

  status = device_create_file(device_obj, &dev_attr_register217);
  if(status)
    goto bad_device_create_file_218;

  status = device_create_file(device_obj, &dev_attr_register218);
  if(status)
    goto bad_device_create_file_219;

  status = device_create_file(device_obj, &dev_attr_register219);
  if(status)
    goto bad_device_create_file_220;

  status = device_create_file(device_obj, &dev_attr_register220);
  if(status)
    goto bad_device_create_file_221;

  status = device_create_file(device_obj, &dev_attr_register221);
  if(status)
    goto bad_device_create_file_222;

  status = device_create_file(device_obj, &dev_attr_register222);
  if(status)
    goto bad_device_create_file_223;

  status = device_create_file(device_obj, &dev_attr_register223);
  if(status)
    goto bad_device_create_file_224;

  status = device_create_file(device_obj, &dev_attr_register224);
  if(status)
    goto bad_device_create_file_225;

  status = device_create_file(device_obj, &dev_attr_register225);
  if(status)
    goto bad_device_create_file_226;

  status = device_create_file(device_obj, &dev_attr_register226);
  if(status)
    goto bad_device_create_file_227;

  status = device_create_file(device_obj, &dev_attr_register227);
  if(status)
    goto bad_device_create_file_228;

  status = device_create_file(device_obj, &dev_attr_register228);
  if(status)
    goto bad_device_create_file_229;

  status = device_create_file(device_obj, &dev_attr_register229);
  if(status)
    goto bad_device_create_file_230;

  status = device_create_file(device_obj, &dev_attr_register230);
  if(status)
    goto bad_device_create_file_231;

  status = device_create_file(device_obj, &dev_attr_register231);
  if(status)
    goto bad_device_create_file_232;

  status = device_create_file(device_obj, &dev_attr_register232);
  if(status)
    goto bad_device_create_file_233;

  status = device_create_file(device_obj, &dev_attr_register233);
  if(status)
    goto bad_device_create_file_234;

  status = device_create_file(device_obj, &dev_attr_register234);
  if(status)
    goto bad_device_create_file_235;

  status = device_create_file(device_obj, &dev_attr_register235);
  if(status)
    goto bad_device_create_file_236;

  status = device_create_file(device_obj, &dev_attr_register236);
  if(status)
    goto bad_device_create_file_237;

  status = device_create_file(device_obj, &dev_attr_register237);
  if(status)
    goto bad_device_create_file_238;

  status = device_create_file(device_obj, &dev_attr_register238);
  if(status)
    goto bad_device_create_file_239;

  status = device_create_file(device_obj, &dev_attr_register239);
  if(status)
    goto bad_device_create_file_240;

  status = device_create_file(device_obj, &dev_attr_register240);
  if(status)
    goto bad_device_create_file_241;

  status = device_create_file(device_obj, &dev_attr_register241);
  if(status)
    goto bad_device_create_file_242;

  status = device_create_file(device_obj, &dev_attr_register242);
  if(status)
    goto bad_device_create_file_243;

  status = device_create_file(device_obj, &dev_attr_register243);
  if(status)
    goto bad_device_create_file_244;

  status = device_create_file(device_obj, &dev_attr_register244);
  if(status)
    goto bad_device_create_file_245;

  status = device_create_file(device_obj, &dev_attr_register245);
  if(status)
    goto bad_device_create_file_246;

  status = device_create_file(device_obj, &dev_attr_register246);
  if(status)
    goto bad_device_create_file_247;

  status = device_create_file(device_obj, &dev_attr_register247);
  if(status)
    goto bad_device_create_file_248;

  status = device_create_file(device_obj, &dev_attr_register248);
  if(status)
    goto bad_device_create_file_249;

  status = device_create_file(device_obj, &dev_attr_register249);
  if(status)
    goto bad_device_create_file_250;

  status = device_create_file(device_obj, &dev_attr_register250);
  if(status)
    goto bad_device_create_file_251;

  status = device_create_file(device_obj, &dev_attr_register251);
  if(status)
    goto bad_device_create_file_252;

  status = device_create_file(device_obj, &dev_attr_register252);
  if(status)
    goto bad_device_create_file_253;

  status = device_create_file(device_obj, &dev_attr_register253);
  if(status)
    goto bad_device_create_file_254;

  status = device_create_file(device_obj, &dev_attr_register254);
  if(status)
    goto bad_device_create_file_255;

  status = device_create_file(device_obj, &dev_attr_register255);
  if(status)
    goto bad_device_create_file_256;

  status = device_create_file(device_obj, &dev_attr_register256);
  if(status)
    goto bad_device_create_file_257;

  status = device_create_file(device_obj, &dev_attr_register257);
  if(status)
    goto bad_device_create_file_258;

  status = device_create_file(device_obj, &dev_attr_register258);
  if(status)
    goto bad_device_create_file_259;

  status = device_create_file(device_obj, &dev_attr_register259);
  if(status)
    goto bad_device_create_file_260;

  status = device_create_file(device_obj, &dev_attr_register260);
  if(status)
    goto bad_device_create_file_261;

  status = device_create_file(device_obj, &dev_attr_register261);
  if(status)
    goto bad_device_create_file_262;

  status = device_create_file(device_obj, &dev_attr_register262);
  if(status)
    goto bad_device_create_file_263;

  status = device_create_file(device_obj, &dev_attr_register263);
  if(status)
    goto bad_device_create_file_264;

  status = device_create_file(device_obj, &dev_attr_register264);
  if(status)
    goto bad_device_create_file_265;

  status = device_create_file(device_obj, &dev_attr_register265);
  if(status)
    goto bad_device_create_file_266;

  status = device_create_file(device_obj, &dev_attr_register266);
  if(status)
    goto bad_device_create_file_267;

  status = device_create_file(device_obj, &dev_attr_register267);
  if(status)
    goto bad_device_create_file_268;

  status = device_create_file(device_obj, &dev_attr_register268);
  if(status)
    goto bad_device_create_file_269;

  status = device_create_file(device_obj, &dev_attr_register269);
  if(status)
    goto bad_device_create_file_270;

  status = device_create_file(device_obj, &dev_attr_register270);
  if(status)
    goto bad_device_create_file_271;

  status = device_create_file(device_obj, &dev_attr_register271);
  if(status)
    goto bad_device_create_file_272;

  status = device_create_file(device_obj, &dev_attr_register272);
  if(status)
    goto bad_device_create_file_273;

  status = device_create_file(device_obj, &dev_attr_register273);
  if(status)
    goto bad_device_create_file_274;

  status = device_create_file(device_obj, &dev_attr_register274);
  if(status)
    goto bad_device_create_file_275;

  status = device_create_file(device_obj, &dev_attr_register275);
  if(status)
    goto bad_device_create_file_276;

  status = device_create_file(device_obj, &dev_attr_register276);
  if(status)
    goto bad_device_create_file_277;

  status = device_create_file(device_obj, &dev_attr_register277);
  if(status)
    goto bad_device_create_file_278;

  status = device_create_file(device_obj, &dev_attr_register278);
  if(status)
    goto bad_device_create_file_279;

  status = device_create_file(device_obj, &dev_attr_register279);
  if(status)
    goto bad_device_create_file_280;

  status = device_create_file(device_obj, &dev_attr_register280);
  if(status)
    goto bad_device_create_file_281;

  status = device_create_file(device_obj, &dev_attr_register281);
  if(status)
    goto bad_device_create_file_282;

  status = device_create_file(device_obj, &dev_attr_register282);
  if(status)
    goto bad_device_create_file_283;

  status = device_create_file(device_obj, &dev_attr_register283);
  if(status)
    goto bad_device_create_file_284;

  status = device_create_file(device_obj, &dev_attr_register284);
  if(status)
    goto bad_device_create_file_285;

  status = device_create_file(device_obj, &dev_attr_register285);
  if(status)
    goto bad_device_create_file_286;

  status = device_create_file(device_obj, &dev_attr_register286);
  if(status)
    goto bad_device_create_file_287;

  status = device_create_file(device_obj, &dev_attr_register287);
  if(status)
    goto bad_device_create_file_288;

  status = device_create_file(device_obj, &dev_attr_register288);
  if(status)
    goto bad_device_create_file_289;

  status = device_create_file(device_obj, &dev_attr_register289);
  if(status)
    goto bad_device_create_file_290;

  status = device_create_file(device_obj, &dev_attr_register290);
  if(status)
    goto bad_device_create_file_291;

  status = device_create_file(device_obj, &dev_attr_register291);
  if(status)
    goto bad_device_create_file_292;

  status = device_create_file(device_obj, &dev_attr_register292);
  if(status)
    goto bad_device_create_file_293;

  status = device_create_file(device_obj, &dev_attr_register293);
  if(status)
    goto bad_device_create_file_294;

  status = device_create_file(device_obj, &dev_attr_register294);
  if(status)
    goto bad_device_create_file_295;

  status = device_create_file(device_obj, &dev_attr_register295);
  if(status)
    goto bad_device_create_file_296;

  status = device_create_file(device_obj, &dev_attr_register296);
  if(status)
    goto bad_device_create_file_297;

  status = device_create_file(device_obj, &dev_attr_register297);
  if(status)
    goto bad_device_create_file_298;

  status = device_create_file(device_obj, &dev_attr_register298);
  if(status)
    goto bad_device_create_file_299;

  status = device_create_file(device_obj, &dev_attr_register299);
  if(status)
    goto bad_device_create_file_300;

  status = device_create_file(device_obj, &dev_attr_register300);
  if(status)
    goto bad_device_create_file_301;

  status = device_create_file(device_obj, &dev_attr_register301);
  if(status)
    goto bad_device_create_file_302;

  status = device_create_file(device_obj, &dev_attr_register302);
  if(status)
    goto bad_device_create_file_303;

  status = device_create_file(device_obj, &dev_attr_register303);
  if(status)
    goto bad_device_create_file_304;

  status = device_create_file(device_obj, &dev_attr_register304);
  if(status)
    goto bad_device_create_file_305;

  status = device_create_file(device_obj, &dev_attr_register305);
  if(status)
    goto bad_device_create_file_306;

  status = device_create_file(device_obj, &dev_attr_register306);
  if(status)
    goto bad_device_create_file_307;

  status = device_create_file(device_obj, &dev_attr_register307);
  if(status)
    goto bad_device_create_file_308;

  status = device_create_file(device_obj, &dev_attr_register308);
  if(status)
    goto bad_device_create_file_309;

  status = device_create_file(device_obj, &dev_attr_register309);
  if(status)
    goto bad_device_create_file_310;

  status = device_create_file(device_obj, &dev_attr_register310);
  if(status)
    goto bad_device_create_file_311;

  status = device_create_file(device_obj, &dev_attr_register311);
  if(status)
    goto bad_device_create_file_312;

  status = device_create_file(device_obj, &dev_attr_register312);
  if(status)
    goto bad_device_create_file_313;

  status = device_create_file(device_obj, &dev_attr_register313);
  if(status)
    goto bad_device_create_file_314;

  status = device_create_file(device_obj, &dev_attr_register314);
  if(status)
    goto bad_device_create_file_315;

  status = device_create_file(device_obj, &dev_attr_register315);
  if(status)
    goto bad_device_create_file_316;

  status = device_create_file(device_obj, &dev_attr_register316);
  if(status)
    goto bad_device_create_file_317;

  status = device_create_file(device_obj, &dev_attr_register317);
  if(status)
    goto bad_device_create_file_318;

  status = device_create_file(device_obj, &dev_attr_register318);
  if(status)
    goto bad_device_create_file_319;

  status = device_create_file(device_obj, &dev_attr_register319);
  if(status)
    goto bad_device_create_file_320;

  status = device_create_file(device_obj, &dev_attr_register320);
  if(status)
    goto bad_device_create_file_321;

  status = device_create_file(device_obj, &dev_attr_register321);
  if(status)
    goto bad_device_create_file_322;

  status = device_create_file(device_obj, &dev_attr_register322);
  if(status)
    goto bad_device_create_file_323;

  status = device_create_file(device_obj, &dev_attr_register323);
  if(status)
    goto bad_device_create_file_324;

  status = device_create_file(device_obj, &dev_attr_register324);
  if(status)
    goto bad_device_create_file_325;

  status = device_create_file(device_obj, &dev_attr_register325);
  if(status)
    goto bad_device_create_file_326;

  status = device_create_file(device_obj, &dev_attr_register326);
  if(status)
    goto bad_device_create_file_327;

  status = device_create_file(device_obj, &dev_attr_register327);
  if(status)
    goto bad_device_create_file_328;

  status = device_create_file(device_obj, &dev_attr_register328);
  if(status)
    goto bad_device_create_file_329;

  status = device_create_file(device_obj, &dev_attr_register329);
  if(status)
    goto bad_device_create_file_330;

  status = device_create_file(device_obj, &dev_attr_register330);
  if(status)
    goto bad_device_create_file_331;

  status = device_create_file(device_obj, &dev_attr_register331);
  if(status)
    goto bad_device_create_file_332;

  status = device_create_file(device_obj, &dev_attr_register332);
  if(status)
    goto bad_device_create_file_333;

  status = device_create_file(device_obj, &dev_attr_register333);
  if(status)
    goto bad_device_create_file_334;

  status = device_create_file(device_obj, &dev_attr_register334);
  if(status)
    goto bad_device_create_file_335;

  status = device_create_file(device_obj, &dev_attr_register335);
  if(status)
    goto bad_device_create_file_336;

  status = device_create_file(device_obj, &dev_attr_register336);
  if(status)
    goto bad_device_create_file_337;

  status = device_create_file(device_obj, &dev_attr_register337);
  if(status)
    goto bad_device_create_file_338;

  status = device_create_file(device_obj, &dev_attr_register338);
  if(status)
    goto bad_device_create_file_339;

  status = device_create_file(device_obj, &dev_attr_register339);
  if(status)
    goto bad_device_create_file_340;

  status = device_create_file(device_obj, &dev_attr_register340);
  if(status)
    goto bad_device_create_file_341;

  status = device_create_file(device_obj, &dev_attr_register341);
  if(status)
    goto bad_device_create_file_342;

  status = device_create_file(device_obj, &dev_attr_register342);
  if(status)
    goto bad_device_create_file_343;

  status = device_create_file(device_obj, &dev_attr_register343);
  if(status)
    goto bad_device_create_file_344;

  status = device_create_file(device_obj, &dev_attr_register344);
  if(status)
    goto bad_device_create_file_345;

  status = device_create_file(device_obj, &dev_attr_register345);
  if(status)
    goto bad_device_create_file_346;

  status = device_create_file(device_obj, &dev_attr_register346);
  if(status)
    goto bad_device_create_file_347;

  status = device_create_file(device_obj, &dev_attr_register347);
  if(status)
    goto bad_device_create_file_348;

  status = device_create_file(device_obj, &dev_attr_register348);
  if(status)
    goto bad_device_create_file_349;

  status = device_create_file(device_obj, &dev_attr_register349);
  if(status)
    goto bad_device_create_file_350;

  status = device_create_file(device_obj, &dev_attr_register350);
  if(status)
    goto bad_device_create_file_351;

  status = device_create_file(device_obj, &dev_attr_register351);
  if(status)
    goto bad_device_create_file_352;

  status = device_create_file(device_obj, &dev_attr_register352);
  if(status)
    goto bad_device_create_file_353;

  status = device_create_file(device_obj, &dev_attr_register353);
  if(status)
    goto bad_device_create_file_354;

  status = device_create_file(device_obj, &dev_attr_register354);
  if(status)
    goto bad_device_create_file_355;

  status = device_create_file(device_obj, &dev_attr_register355);
  if(status)
    goto bad_device_create_file_356;

  status = device_create_file(device_obj, &dev_attr_register356);
  if(status)
    goto bad_device_create_file_357;

  status = device_create_file(device_obj, &dev_attr_register357);
  if(status)
    goto bad_device_create_file_358;

  status = device_create_file(device_obj, &dev_attr_register358);
  if(status)
    goto bad_device_create_file_359;

  status = device_create_file(device_obj, &dev_attr_register359);
  if(status)
    goto bad_device_create_file_360;

  status = device_create_file(device_obj, &dev_attr_register360);
  if(status)
    goto bad_device_create_file_361;

  status = device_create_file(device_obj, &dev_attr_register361);
  if(status)
    goto bad_device_create_file_362;

  status = device_create_file(device_obj, &dev_attr_register362);
  if(status)
    goto bad_device_create_file_363;

  status = device_create_file(device_obj, &dev_attr_register363);
  if(status)
    goto bad_device_create_file_364;

  status = device_create_file(device_obj, &dev_attr_register364);
  if(status)
    goto bad_device_create_file_365;

  status = device_create_file(device_obj, &dev_attr_register365);
  if(status)
    goto bad_device_create_file_366;

  status = device_create_file(device_obj, &dev_attr_register366);
  if(status)
    goto bad_device_create_file_367;

  status = device_create_file(device_obj, &dev_attr_register367);
  if(status)
    goto bad_device_create_file_368;

  status = device_create_file(device_obj, &dev_attr_register368);
  if(status)
    goto bad_device_create_file_369;

  status = device_create_file(device_obj, &dev_attr_register369);
  if(status)
    goto bad_device_create_file_370;

  status = device_create_file(device_obj, &dev_attr_register370);
  if(status)
    goto bad_device_create_file_371;

  status = device_create_file(device_obj, &dev_attr_register371);
  if(status)
    goto bad_device_create_file_372;

  status = device_create_file(device_obj, &dev_attr_register372);
  if(status)
    goto bad_device_create_file_373;

  status = device_create_file(device_obj, &dev_attr_register373);
  if(status)
    goto bad_device_create_file_374;

  status = device_create_file(device_obj, &dev_attr_register374);
  if(status)
    goto bad_device_create_file_375;

  status = device_create_file(device_obj, &dev_attr_register375);
  if(status)
    goto bad_device_create_file_376;

  status = device_create_file(device_obj, &dev_attr_register376);
  if(status)
    goto bad_device_create_file_377;

  status = device_create_file(device_obj, &dev_attr_register377);
  if(status)
    goto bad_device_create_file_378;

  status = device_create_file(device_obj, &dev_attr_register378);
  if(status)
    goto bad_device_create_file_379;

  status = device_create_file(device_obj, &dev_attr_register379);
  if(status)
    goto bad_device_create_file_380;

  status = device_create_file(device_obj, &dev_attr_register380);
  if(status)
    goto bad_device_create_file_381;

  status = device_create_file(device_obj, &dev_attr_register381);
  if(status)
    goto bad_device_create_file_382;

  status = device_create_file(device_obj, &dev_attr_register382);
  if(status)
    goto bad_device_create_file_383;

  status = device_create_file(device_obj, &dev_attr_register383);
  if(status)
    goto bad_device_create_file_384;

  status = device_create_file(device_obj, &dev_attr_register384);
  if(status)
    goto bad_device_create_file_385;

  status = device_create_file(device_obj, &dev_attr_register385);
  if(status)
    goto bad_device_create_file_386;

  status = device_create_file(device_obj, &dev_attr_register386);
  if(status)
    goto bad_device_create_file_387;

  status = device_create_file(device_obj, &dev_attr_register387);
  if(status)
    goto bad_device_create_file_388;

  status = device_create_file(device_obj, &dev_attr_register388);
  if(status)
    goto bad_device_create_file_389;

  status = device_create_file(device_obj, &dev_attr_register389);
  if(status)
    goto bad_device_create_file_390;

  status = device_create_file(device_obj, &dev_attr_register390);
  if(status)
    goto bad_device_create_file_391;

  status = device_create_file(device_obj, &dev_attr_register391);
  if(status)
    goto bad_device_create_file_392;

  status = device_create_file(device_obj, &dev_attr_register392);
  if(status)
    goto bad_device_create_file_393;

  status = device_create_file(device_obj, &dev_attr_register393);
  if(status)
    goto bad_device_create_file_394;

  status = device_create_file(device_obj, &dev_attr_register394);
  if(status)
    goto bad_device_create_file_395;

  status = device_create_file(device_obj, &dev_attr_register395);
  if(status)
    goto bad_device_create_file_396;

  status = device_create_file(device_obj, &dev_attr_register396);
  if(status)
    goto bad_device_create_file_397;

  status = device_create_file(device_obj, &dev_attr_register397);
  if(status)
    goto bad_device_create_file_398;

  status = device_create_file(device_obj, &dev_attr_register398);
  if(status)
    goto bad_device_create_file_399;

  status = device_create_file(device_obj, &dev_attr_register399);
  if(status)
    goto bad_device_create_file_400;

  status = device_create_file(device_obj, &dev_attr_register400);
  if(status)
    goto bad_device_create_file_401;

  status = device_create_file(device_obj, &dev_attr_register401);
  if(status)
    goto bad_device_create_file_402;

  status = device_create_file(device_obj, &dev_attr_register402);
  if(status)
    goto bad_device_create_file_403;

  status = device_create_file(device_obj, &dev_attr_register403);
  if(status)
    goto bad_device_create_file_404;

  status = device_create_file(device_obj, &dev_attr_register404);
  if(status)
    goto bad_device_create_file_405;

  status = device_create_file(device_obj, &dev_attr_register405);
  if(status)
    goto bad_device_create_file_406;

  status = device_create_file(device_obj, &dev_attr_register406);
  if(status)
    goto bad_device_create_file_407;

  status = device_create_file(device_obj, &dev_attr_register407);
  if(status)
    goto bad_device_create_file_408;

  status = device_create_file(device_obj, &dev_attr_register408);
  if(status)
    goto bad_device_create_file_409;

  status = device_create_file(device_obj, &dev_attr_register409);
  if(status)
    goto bad_device_create_file_410;

  status = device_create_file(device_obj, &dev_attr_register410);
  if(status)
    goto bad_device_create_file_411;

  status = device_create_file(device_obj, &dev_attr_register411);
  if(status)
    goto bad_device_create_file_412;

  status = device_create_file(device_obj, &dev_attr_register412);
  if(status)
    goto bad_device_create_file_413;

  status = device_create_file(device_obj, &dev_attr_register413);
  if(status)
    goto bad_device_create_file_414;

  status = device_create_file(device_obj, &dev_attr_register414);
  if(status)
    goto bad_device_create_file_415;

  status = device_create_file(device_obj, &dev_attr_register415);
  if(status)
    goto bad_device_create_file_416;

  status = device_create_file(device_obj, &dev_attr_register416);
  if(status)
    goto bad_device_create_file_417;

  status = device_create_file(device_obj, &dev_attr_register417);
  if(status)
    goto bad_device_create_file_418;

  status = device_create_file(device_obj, &dev_attr_register418);
  if(status)
    goto bad_device_create_file_419;

  status = device_create_file(device_obj, &dev_attr_register419);
  if(status)
    goto bad_device_create_file_420;

  status = device_create_file(device_obj, &dev_attr_register420);
  if(status)
    goto bad_device_create_file_421;

  status = device_create_file(device_obj, &dev_attr_register421);
  if(status)
    goto bad_device_create_file_422;

  status = device_create_file(device_obj, &dev_attr_register422);
  if(status)
    goto bad_device_create_file_423;

  status = device_create_file(device_obj, &dev_attr_register423);
  if(status)
    goto bad_device_create_file_424;

  status = device_create_file(device_obj, &dev_attr_register424);
  if(status)
    goto bad_device_create_file_425;

  status = device_create_file(device_obj, &dev_attr_register425);
  if(status)
    goto bad_device_create_file_426;

  status = device_create_file(device_obj, &dev_attr_register426);
  if(status)
    goto bad_device_create_file_427;

  status = device_create_file(device_obj, &dev_attr_register427);
  if(status)
    goto bad_device_create_file_428;

  status = device_create_file(device_obj, &dev_attr_register428);
  if(status)
    goto bad_device_create_file_429;

  status = device_create_file(device_obj, &dev_attr_register429);
  if(status)
    goto bad_device_create_file_430;

  status = device_create_file(device_obj, &dev_attr_register430);
  if(status)
    goto bad_device_create_file_431;

  status = device_create_file(device_obj, &dev_attr_register431);
  if(status)
    goto bad_device_create_file_432;

  status = device_create_file(device_obj, &dev_attr_register432);
  if(status)
    goto bad_device_create_file_433;

  status = device_create_file(device_obj, &dev_attr_register433);
  if(status)
    goto bad_device_create_file_434;

  status = device_create_file(device_obj, &dev_attr_register434);
  if(status)
    goto bad_device_create_file_435;

  status = device_create_file(device_obj, &dev_attr_register435);
  if(status)
    goto bad_device_create_file_436;

  status = device_create_file(device_obj, &dev_attr_register436);
  if(status)
    goto bad_device_create_file_437;

  status = device_create_file(device_obj, &dev_attr_register437);
  if(status)
    goto bad_device_create_file_438;

  status = device_create_file(device_obj, &dev_attr_register438);
  if(status)
    goto bad_device_create_file_439;

  status = device_create_file(device_obj, &dev_attr_register439);
  if(status)
    goto bad_device_create_file_440;

  status = device_create_file(device_obj, &dev_attr_register440);
  if(status)
    goto bad_device_create_file_441;

  status = device_create_file(device_obj, &dev_attr_register441);
  if(status)
    goto bad_device_create_file_442;

  status = device_create_file(device_obj, &dev_attr_register442);
  if(status)
    goto bad_device_create_file_443;

  status = device_create_file(device_obj, &dev_attr_register443);
  if(status)
    goto bad_device_create_file_444;

  status = device_create_file(device_obj, &dev_attr_register444);
  if(status)
    goto bad_device_create_file_445;

  status = device_create_file(device_obj, &dev_attr_register445);
  if(status)
    goto bad_device_create_file_446;

  status = device_create_file(device_obj, &dev_attr_register446);
  if(status)
    goto bad_device_create_file_447;

  status = device_create_file(device_obj, &dev_attr_register447);
  if(status)
    goto bad_device_create_file_448;

  status = device_create_file(device_obj, &dev_attr_register448);
  if(status)
    goto bad_device_create_file_449;

  status = device_create_file(device_obj, &dev_attr_register449);
  if(status)
    goto bad_device_create_file_450;

  status = device_create_file(device_obj, &dev_attr_register450);
  if(status)
    goto bad_device_create_file_451;

  status = device_create_file(device_obj, &dev_attr_register451);
  if(status)
    goto bad_device_create_file_452;

  status = device_create_file(device_obj, &dev_attr_register452);
  if(status)
    goto bad_device_create_file_453;

  status = device_create_file(device_obj, &dev_attr_register453);
  if(status)
    goto bad_device_create_file_454;

  status = device_create_file(device_obj, &dev_attr_register454);
  if(status)
    goto bad_device_create_file_455;

  status = device_create_file(device_obj, &dev_attr_register455);
  if(status)
    goto bad_device_create_file_456;

  status = device_create_file(device_obj, &dev_attr_register456);
  if(status)
    goto bad_device_create_file_457;

  status = device_create_file(device_obj, &dev_attr_register457);
  if(status)
    goto bad_device_create_file_458;

  status = device_create_file(device_obj, &dev_attr_register458);
  if(status)
    goto bad_device_create_file_459;

  status = device_create_file(device_obj, &dev_attr_register459);
  if(status)
    goto bad_device_create_file_460;

  status = device_create_file(device_obj, &dev_attr_register460);
  if(status)
    goto bad_device_create_file_461;

  status = device_create_file(device_obj, &dev_attr_register461);
  if(status)
    goto bad_device_create_file_462;

  status = device_create_file(device_obj, &dev_attr_register462);
  if(status)
    goto bad_device_create_file_463;

  status = device_create_file(device_obj, &dev_attr_register463);
  if(status)
    goto bad_device_create_file_464;

  status = device_create_file(device_obj, &dev_attr_register464);
  if(status)
    goto bad_device_create_file_465;

  status = device_create_file(device_obj, &dev_attr_register465);
  if(status)
    goto bad_device_create_file_466;

  status = device_create_file(device_obj, &dev_attr_register466);
  if(status)
    goto bad_device_create_file_467;

  status = device_create_file(device_obj, &dev_attr_register467);
  if(status)
    goto bad_device_create_file_468;

  status = device_create_file(device_obj, &dev_attr_register468);
  if(status)
    goto bad_device_create_file_469;

  status = device_create_file(device_obj, &dev_attr_register469);
  if(status)
    goto bad_device_create_file_470;

  status = device_create_file(device_obj, &dev_attr_register470);
  if(status)
    goto bad_device_create_file_471;

  status = device_create_file(device_obj, &dev_attr_register471);
  if(status)
    goto bad_device_create_file_472;

  status = device_create_file(device_obj, &dev_attr_register472);
  if(status)
    goto bad_device_create_file_473;

  status = device_create_file(device_obj, &dev_attr_register473);
  if(status)
    goto bad_device_create_file_474;

  status = device_create_file(device_obj, &dev_attr_register474);
  if(status)
    goto bad_device_create_file_475;

  status = device_create_file(device_obj, &dev_attr_register475);
  if(status)
    goto bad_device_create_file_476;

  status = device_create_file(device_obj, &dev_attr_register476);
  if(status)
    goto bad_device_create_file_477;

  status = device_create_file(device_obj, &dev_attr_register477);
  if(status)
    goto bad_device_create_file_478;

  status = device_create_file(device_obj, &dev_attr_register478);
  if(status)
    goto bad_device_create_file_479;

  status = device_create_file(device_obj, &dev_attr_register479);
  if(status)
    goto bad_device_create_file_480;

  status = device_create_file(device_obj, &dev_attr_register480);
  if(status)
    goto bad_device_create_file_481;

  status = device_create_file(device_obj, &dev_attr_register481);
  if(status)
    goto bad_device_create_file_482;

  status = device_create_file(device_obj, &dev_attr_register482);
  if(status)
    goto bad_device_create_file_483;

  status = device_create_file(device_obj, &dev_attr_register483);
  if(status)
    goto bad_device_create_file_484;

  status = device_create_file(device_obj, &dev_attr_register484);
  if(status)
    goto bad_device_create_file_485;

  status = device_create_file(device_obj, &dev_attr_register485);
  if(status)
    goto bad_device_create_file_486;

  status = device_create_file(device_obj, &dev_attr_register486);
  if(status)
    goto bad_device_create_file_487;

  status = device_create_file(device_obj, &dev_attr_register487);
  if(status)
    goto bad_device_create_file_488;

  status = device_create_file(device_obj, &dev_attr_register488);
  if(status)
    goto bad_device_create_file_489;

  status = device_create_file(device_obj, &dev_attr_register489);
  if(status)
    goto bad_device_create_file_490;

  status = device_create_file(device_obj, &dev_attr_register490);
  if(status)
    goto bad_device_create_file_491;

  status = device_create_file(device_obj, &dev_attr_register491);
  if(status)
    goto bad_device_create_file_492;

  status = device_create_file(device_obj, &dev_attr_register492);
  if(status)
    goto bad_device_create_file_493;

  status = device_create_file(device_obj, &dev_attr_register493);
  if(status)
    goto bad_device_create_file_494;

  status = device_create_file(device_obj, &dev_attr_register494);
  if(status)
    goto bad_device_create_file_495;

  status = device_create_file(device_obj, &dev_attr_register495);
  if(status)
    goto bad_device_create_file_496;

  status = device_create_file(device_obj, &dev_attr_register496);
  if(status)
    goto bad_device_create_file_497;

  status = device_create_file(device_obj, &dev_attr_register497);
  if(status)
    goto bad_device_create_file_498;

  status = device_create_file(device_obj, &dev_attr_register498);
  if(status)
    goto bad_device_create_file_499;

  status = device_create_file(device_obj, &dev_attr_register499);
  if(status)
    goto bad_device_create_file_500;

  status = device_create_file(device_obj, &dev_attr_register500);
  if(status)
    goto bad_device_create_file_501;

  status = device_create_file(device_obj, &dev_attr_register501);
  if(status)
    goto bad_device_create_file_502;

  status = device_create_file(device_obj, &dev_attr_register502);
  if(status)
    goto bad_device_create_file_503;

  status = device_create_file(device_obj, &dev_attr_register503);
  if(status)
    goto bad_device_create_file_504;

  status = device_create_file(device_obj, &dev_attr_register504);
  if(status)
    goto bad_device_create_file_505;

  status = device_create_file(device_obj, &dev_attr_register505);
  if(status)
    goto bad_device_create_file_506;

  status = device_create_file(device_obj, &dev_attr_register506);
  if(status)
    goto bad_device_create_file_507;

  status = device_create_file(device_obj, &dev_attr_register507);
  if(status)
    goto bad_device_create_file_508;

  status = device_create_file(device_obj, &dev_attr_register508);
  if(status)
    goto bad_device_create_file_509;

  status = device_create_file(device_obj, &dev_attr_register509);
  if(status)
    goto bad_device_create_file_510;

  status = device_create_file(device_obj, &dev_attr_register510);
  if(status)
    goto bad_device_create_file_511;

  status = device_create_file(device_obj, &dev_attr_register511);
  if(status)
    goto bad_device_create_file_512;

  pr_info("DPRAM_probe exit\n");
  return 0;

  // Error functions for probe function  
  bad_device_create_file_512:
    device_remove_file(device_obj, &dev_attr_register511);

  bad_device_create_file_511:
    device_remove_file(device_obj, &dev_attr_register510);

  bad_device_create_file_510:
    device_remove_file(device_obj, &dev_attr_register509);

  bad_device_create_file_509:
    device_remove_file(device_obj, &dev_attr_register508);

  bad_device_create_file_508:
    device_remove_file(device_obj, &dev_attr_register507);

  bad_device_create_file_507:
    device_remove_file(device_obj, &dev_attr_register506);

  bad_device_create_file_506:
    device_remove_file(device_obj, &dev_attr_register505);

  bad_device_create_file_505:
    device_remove_file(device_obj, &dev_attr_register504);

  bad_device_create_file_504:
    device_remove_file(device_obj, &dev_attr_register503);

  bad_device_create_file_503:
    device_remove_file(device_obj, &dev_attr_register502);

  bad_device_create_file_502:
    device_remove_file(device_obj, &dev_attr_register501);

  bad_device_create_file_501:
    device_remove_file(device_obj, &dev_attr_register500);

  bad_device_create_file_500:
    device_remove_file(device_obj, &dev_attr_register499);

  bad_device_create_file_499:
    device_remove_file(device_obj, &dev_attr_register498);

  bad_device_create_file_498:
    device_remove_file(device_obj, &dev_attr_register497);

  bad_device_create_file_497:
    device_remove_file(device_obj, &dev_attr_register496);

  bad_device_create_file_496:
    device_remove_file(device_obj, &dev_attr_register495);

  bad_device_create_file_495:
    device_remove_file(device_obj, &dev_attr_register494);

  bad_device_create_file_494:
    device_remove_file(device_obj, &dev_attr_register493);

  bad_device_create_file_493:
    device_remove_file(device_obj, &dev_attr_register492);

  bad_device_create_file_492:
    device_remove_file(device_obj, &dev_attr_register491);

  bad_device_create_file_491:
    device_remove_file(device_obj, &dev_attr_register490);

  bad_device_create_file_490:
    device_remove_file(device_obj, &dev_attr_register489);

  bad_device_create_file_489:
    device_remove_file(device_obj, &dev_attr_register488);

  bad_device_create_file_488:
    device_remove_file(device_obj, &dev_attr_register487);

  bad_device_create_file_487:
    device_remove_file(device_obj, &dev_attr_register486);

  bad_device_create_file_486:
    device_remove_file(device_obj, &dev_attr_register485);

  bad_device_create_file_485:
    device_remove_file(device_obj, &dev_attr_register484);

  bad_device_create_file_484:
    device_remove_file(device_obj, &dev_attr_register483);

  bad_device_create_file_483:
    device_remove_file(device_obj, &dev_attr_register482);

  bad_device_create_file_482:
    device_remove_file(device_obj, &dev_attr_register481);

  bad_device_create_file_481:
    device_remove_file(device_obj, &dev_attr_register480);

  bad_device_create_file_480:
    device_remove_file(device_obj, &dev_attr_register479);

  bad_device_create_file_479:
    device_remove_file(device_obj, &dev_attr_register478);

  bad_device_create_file_478:
    device_remove_file(device_obj, &dev_attr_register477);

  bad_device_create_file_477:
    device_remove_file(device_obj, &dev_attr_register476);

  bad_device_create_file_476:
    device_remove_file(device_obj, &dev_attr_register475);

  bad_device_create_file_475:
    device_remove_file(device_obj, &dev_attr_register474);

  bad_device_create_file_474:
    device_remove_file(device_obj, &dev_attr_register473);

  bad_device_create_file_473:
    device_remove_file(device_obj, &dev_attr_register472);

  bad_device_create_file_472:
    device_remove_file(device_obj, &dev_attr_register471);

  bad_device_create_file_471:
    device_remove_file(device_obj, &dev_attr_register470);

  bad_device_create_file_470:
    device_remove_file(device_obj, &dev_attr_register469);

  bad_device_create_file_469:
    device_remove_file(device_obj, &dev_attr_register468);

  bad_device_create_file_468:
    device_remove_file(device_obj, &dev_attr_register467);

  bad_device_create_file_467:
    device_remove_file(device_obj, &dev_attr_register466);

  bad_device_create_file_466:
    device_remove_file(device_obj, &dev_attr_register465);

  bad_device_create_file_465:
    device_remove_file(device_obj, &dev_attr_register464);

  bad_device_create_file_464:
    device_remove_file(device_obj, &dev_attr_register463);

  bad_device_create_file_463:
    device_remove_file(device_obj, &dev_attr_register462);

  bad_device_create_file_462:
    device_remove_file(device_obj, &dev_attr_register461);

  bad_device_create_file_461:
    device_remove_file(device_obj, &dev_attr_register460);

  bad_device_create_file_460:
    device_remove_file(device_obj, &dev_attr_register459);

  bad_device_create_file_459:
    device_remove_file(device_obj, &dev_attr_register458);

  bad_device_create_file_458:
    device_remove_file(device_obj, &dev_attr_register457);

  bad_device_create_file_457:
    device_remove_file(device_obj, &dev_attr_register456);

  bad_device_create_file_456:
    device_remove_file(device_obj, &dev_attr_register455);

  bad_device_create_file_455:
    device_remove_file(device_obj, &dev_attr_register454);

  bad_device_create_file_454:
    device_remove_file(device_obj, &dev_attr_register453);

  bad_device_create_file_453:
    device_remove_file(device_obj, &dev_attr_register452);

  bad_device_create_file_452:
    device_remove_file(device_obj, &dev_attr_register451);

  bad_device_create_file_451:
    device_remove_file(device_obj, &dev_attr_register450);

  bad_device_create_file_450:
    device_remove_file(device_obj, &dev_attr_register449);

  bad_device_create_file_449:
    device_remove_file(device_obj, &dev_attr_register448);

  bad_device_create_file_448:
    device_remove_file(device_obj, &dev_attr_register447);

  bad_device_create_file_447:
    device_remove_file(device_obj, &dev_attr_register446);

  bad_device_create_file_446:
    device_remove_file(device_obj, &dev_attr_register445);

  bad_device_create_file_445:
    device_remove_file(device_obj, &dev_attr_register444);

  bad_device_create_file_444:
    device_remove_file(device_obj, &dev_attr_register443);

  bad_device_create_file_443:
    device_remove_file(device_obj, &dev_attr_register442);

  bad_device_create_file_442:
    device_remove_file(device_obj, &dev_attr_register441);

  bad_device_create_file_441:
    device_remove_file(device_obj, &dev_attr_register440);

  bad_device_create_file_440:
    device_remove_file(device_obj, &dev_attr_register439);

  bad_device_create_file_439:
    device_remove_file(device_obj, &dev_attr_register438);

  bad_device_create_file_438:
    device_remove_file(device_obj, &dev_attr_register437);

  bad_device_create_file_437:
    device_remove_file(device_obj, &dev_attr_register436);

  bad_device_create_file_436:
    device_remove_file(device_obj, &dev_attr_register435);

  bad_device_create_file_435:
    device_remove_file(device_obj, &dev_attr_register434);

  bad_device_create_file_434:
    device_remove_file(device_obj, &dev_attr_register433);

  bad_device_create_file_433:
    device_remove_file(device_obj, &dev_attr_register432);

  bad_device_create_file_432:
    device_remove_file(device_obj, &dev_attr_register431);

  bad_device_create_file_431:
    device_remove_file(device_obj, &dev_attr_register430);

  bad_device_create_file_430:
    device_remove_file(device_obj, &dev_attr_register429);

  bad_device_create_file_429:
    device_remove_file(device_obj, &dev_attr_register428);

  bad_device_create_file_428:
    device_remove_file(device_obj, &dev_attr_register427);

  bad_device_create_file_427:
    device_remove_file(device_obj, &dev_attr_register426);

  bad_device_create_file_426:
    device_remove_file(device_obj, &dev_attr_register425);

  bad_device_create_file_425:
    device_remove_file(device_obj, &dev_attr_register424);

  bad_device_create_file_424:
    device_remove_file(device_obj, &dev_attr_register423);

  bad_device_create_file_423:
    device_remove_file(device_obj, &dev_attr_register422);

  bad_device_create_file_422:
    device_remove_file(device_obj, &dev_attr_register421);

  bad_device_create_file_421:
    device_remove_file(device_obj, &dev_attr_register420);

  bad_device_create_file_420:
    device_remove_file(device_obj, &dev_attr_register419);

  bad_device_create_file_419:
    device_remove_file(device_obj, &dev_attr_register418);

  bad_device_create_file_418:
    device_remove_file(device_obj, &dev_attr_register417);

  bad_device_create_file_417:
    device_remove_file(device_obj, &dev_attr_register416);

  bad_device_create_file_416:
    device_remove_file(device_obj, &dev_attr_register415);

  bad_device_create_file_415:
    device_remove_file(device_obj, &dev_attr_register414);

  bad_device_create_file_414:
    device_remove_file(device_obj, &dev_attr_register413);

  bad_device_create_file_413:
    device_remove_file(device_obj, &dev_attr_register412);

  bad_device_create_file_412:
    device_remove_file(device_obj, &dev_attr_register411);

  bad_device_create_file_411:
    device_remove_file(device_obj, &dev_attr_register410);

  bad_device_create_file_410:
    device_remove_file(device_obj, &dev_attr_register409);

  bad_device_create_file_409:
    device_remove_file(device_obj, &dev_attr_register408);

  bad_device_create_file_408:
    device_remove_file(device_obj, &dev_attr_register407);

  bad_device_create_file_407:
    device_remove_file(device_obj, &dev_attr_register406);

  bad_device_create_file_406:
    device_remove_file(device_obj, &dev_attr_register405);

  bad_device_create_file_405:
    device_remove_file(device_obj, &dev_attr_register404);

  bad_device_create_file_404:
    device_remove_file(device_obj, &dev_attr_register403);

  bad_device_create_file_403:
    device_remove_file(device_obj, &dev_attr_register402);

  bad_device_create_file_402:
    device_remove_file(device_obj, &dev_attr_register401);

  bad_device_create_file_401:
    device_remove_file(device_obj, &dev_attr_register400);

  bad_device_create_file_400:
    device_remove_file(device_obj, &dev_attr_register399);

  bad_device_create_file_399:
    device_remove_file(device_obj, &dev_attr_register398);

  bad_device_create_file_398:
    device_remove_file(device_obj, &dev_attr_register397);

  bad_device_create_file_397:
    device_remove_file(device_obj, &dev_attr_register396);

  bad_device_create_file_396:
    device_remove_file(device_obj, &dev_attr_register395);

  bad_device_create_file_395:
    device_remove_file(device_obj, &dev_attr_register394);

  bad_device_create_file_394:
    device_remove_file(device_obj, &dev_attr_register393);

  bad_device_create_file_393:
    device_remove_file(device_obj, &dev_attr_register392);

  bad_device_create_file_392:
    device_remove_file(device_obj, &dev_attr_register391);

  bad_device_create_file_391:
    device_remove_file(device_obj, &dev_attr_register390);

  bad_device_create_file_390:
    device_remove_file(device_obj, &dev_attr_register389);

  bad_device_create_file_389:
    device_remove_file(device_obj, &dev_attr_register388);

  bad_device_create_file_388:
    device_remove_file(device_obj, &dev_attr_register387);

  bad_device_create_file_387:
    device_remove_file(device_obj, &dev_attr_register386);

  bad_device_create_file_386:
    device_remove_file(device_obj, &dev_attr_register385);

  bad_device_create_file_385:
    device_remove_file(device_obj, &dev_attr_register384);

  bad_device_create_file_384:
    device_remove_file(device_obj, &dev_attr_register383);

  bad_device_create_file_383:
    device_remove_file(device_obj, &dev_attr_register382);

  bad_device_create_file_382:
    device_remove_file(device_obj, &dev_attr_register381);

  bad_device_create_file_381:
    device_remove_file(device_obj, &dev_attr_register380);

  bad_device_create_file_380:
    device_remove_file(device_obj, &dev_attr_register379);

  bad_device_create_file_379:
    device_remove_file(device_obj, &dev_attr_register378);

  bad_device_create_file_378:
    device_remove_file(device_obj, &dev_attr_register377);

  bad_device_create_file_377:
    device_remove_file(device_obj, &dev_attr_register376);

  bad_device_create_file_376:
    device_remove_file(device_obj, &dev_attr_register375);

  bad_device_create_file_375:
    device_remove_file(device_obj, &dev_attr_register374);

  bad_device_create_file_374:
    device_remove_file(device_obj, &dev_attr_register373);

  bad_device_create_file_373:
    device_remove_file(device_obj, &dev_attr_register372);

  bad_device_create_file_372:
    device_remove_file(device_obj, &dev_attr_register371);

  bad_device_create_file_371:
    device_remove_file(device_obj, &dev_attr_register370);

  bad_device_create_file_370:
    device_remove_file(device_obj, &dev_attr_register369);

  bad_device_create_file_369:
    device_remove_file(device_obj, &dev_attr_register368);

  bad_device_create_file_368:
    device_remove_file(device_obj, &dev_attr_register367);

  bad_device_create_file_367:
    device_remove_file(device_obj, &dev_attr_register366);

  bad_device_create_file_366:
    device_remove_file(device_obj, &dev_attr_register365);

  bad_device_create_file_365:
    device_remove_file(device_obj, &dev_attr_register364);

  bad_device_create_file_364:
    device_remove_file(device_obj, &dev_attr_register363);

  bad_device_create_file_363:
    device_remove_file(device_obj, &dev_attr_register362);

  bad_device_create_file_362:
    device_remove_file(device_obj, &dev_attr_register361);

  bad_device_create_file_361:
    device_remove_file(device_obj, &dev_attr_register360);

  bad_device_create_file_360:
    device_remove_file(device_obj, &dev_attr_register359);

  bad_device_create_file_359:
    device_remove_file(device_obj, &dev_attr_register358);

  bad_device_create_file_358:
    device_remove_file(device_obj, &dev_attr_register357);

  bad_device_create_file_357:
    device_remove_file(device_obj, &dev_attr_register356);

  bad_device_create_file_356:
    device_remove_file(device_obj, &dev_attr_register355);

  bad_device_create_file_355:
    device_remove_file(device_obj, &dev_attr_register354);

  bad_device_create_file_354:
    device_remove_file(device_obj, &dev_attr_register353);

  bad_device_create_file_353:
    device_remove_file(device_obj, &dev_attr_register352);

  bad_device_create_file_352:
    device_remove_file(device_obj, &dev_attr_register351);

  bad_device_create_file_351:
    device_remove_file(device_obj, &dev_attr_register350);

  bad_device_create_file_350:
    device_remove_file(device_obj, &dev_attr_register349);

  bad_device_create_file_349:
    device_remove_file(device_obj, &dev_attr_register348);

  bad_device_create_file_348:
    device_remove_file(device_obj, &dev_attr_register347);

  bad_device_create_file_347:
    device_remove_file(device_obj, &dev_attr_register346);

  bad_device_create_file_346:
    device_remove_file(device_obj, &dev_attr_register345);

  bad_device_create_file_345:
    device_remove_file(device_obj, &dev_attr_register344);

  bad_device_create_file_344:
    device_remove_file(device_obj, &dev_attr_register343);

  bad_device_create_file_343:
    device_remove_file(device_obj, &dev_attr_register342);

  bad_device_create_file_342:
    device_remove_file(device_obj, &dev_attr_register341);

  bad_device_create_file_341:
    device_remove_file(device_obj, &dev_attr_register340);

  bad_device_create_file_340:
    device_remove_file(device_obj, &dev_attr_register339);

  bad_device_create_file_339:
    device_remove_file(device_obj, &dev_attr_register338);

  bad_device_create_file_338:
    device_remove_file(device_obj, &dev_attr_register337);

  bad_device_create_file_337:
    device_remove_file(device_obj, &dev_attr_register336);

  bad_device_create_file_336:
    device_remove_file(device_obj, &dev_attr_register335);

  bad_device_create_file_335:
    device_remove_file(device_obj, &dev_attr_register334);

  bad_device_create_file_334:
    device_remove_file(device_obj, &dev_attr_register333);

  bad_device_create_file_333:
    device_remove_file(device_obj, &dev_attr_register332);

  bad_device_create_file_332:
    device_remove_file(device_obj, &dev_attr_register331);

  bad_device_create_file_331:
    device_remove_file(device_obj, &dev_attr_register330);

  bad_device_create_file_330:
    device_remove_file(device_obj, &dev_attr_register329);

  bad_device_create_file_329:
    device_remove_file(device_obj, &dev_attr_register328);

  bad_device_create_file_328:
    device_remove_file(device_obj, &dev_attr_register327);

  bad_device_create_file_327:
    device_remove_file(device_obj, &dev_attr_register326);

  bad_device_create_file_326:
    device_remove_file(device_obj, &dev_attr_register325);

  bad_device_create_file_325:
    device_remove_file(device_obj, &dev_attr_register324);

  bad_device_create_file_324:
    device_remove_file(device_obj, &dev_attr_register323);

  bad_device_create_file_323:
    device_remove_file(device_obj, &dev_attr_register322);

  bad_device_create_file_322:
    device_remove_file(device_obj, &dev_attr_register321);

  bad_device_create_file_321:
    device_remove_file(device_obj, &dev_attr_register320);

  bad_device_create_file_320:
    device_remove_file(device_obj, &dev_attr_register319);

  bad_device_create_file_319:
    device_remove_file(device_obj, &dev_attr_register318);

  bad_device_create_file_318:
    device_remove_file(device_obj, &dev_attr_register317);

  bad_device_create_file_317:
    device_remove_file(device_obj, &dev_attr_register316);

  bad_device_create_file_316:
    device_remove_file(device_obj, &dev_attr_register315);

  bad_device_create_file_315:
    device_remove_file(device_obj, &dev_attr_register314);

  bad_device_create_file_314:
    device_remove_file(device_obj, &dev_attr_register313);

  bad_device_create_file_313:
    device_remove_file(device_obj, &dev_attr_register312);

  bad_device_create_file_312:
    device_remove_file(device_obj, &dev_attr_register311);

  bad_device_create_file_311:
    device_remove_file(device_obj, &dev_attr_register310);

  bad_device_create_file_310:
    device_remove_file(device_obj, &dev_attr_register309);

  bad_device_create_file_309:
    device_remove_file(device_obj, &dev_attr_register308);

  bad_device_create_file_308:
    device_remove_file(device_obj, &dev_attr_register307);

  bad_device_create_file_307:
    device_remove_file(device_obj, &dev_attr_register306);

  bad_device_create_file_306:
    device_remove_file(device_obj, &dev_attr_register305);

  bad_device_create_file_305:
    device_remove_file(device_obj, &dev_attr_register304);

  bad_device_create_file_304:
    device_remove_file(device_obj, &dev_attr_register303);

  bad_device_create_file_303:
    device_remove_file(device_obj, &dev_attr_register302);

  bad_device_create_file_302:
    device_remove_file(device_obj, &dev_attr_register301);

  bad_device_create_file_301:
    device_remove_file(device_obj, &dev_attr_register300);

  bad_device_create_file_300:
    device_remove_file(device_obj, &dev_attr_register299);

  bad_device_create_file_299:
    device_remove_file(device_obj, &dev_attr_register298);

  bad_device_create_file_298:
    device_remove_file(device_obj, &dev_attr_register297);

  bad_device_create_file_297:
    device_remove_file(device_obj, &dev_attr_register296);

  bad_device_create_file_296:
    device_remove_file(device_obj, &dev_attr_register295);

  bad_device_create_file_295:
    device_remove_file(device_obj, &dev_attr_register294);

  bad_device_create_file_294:
    device_remove_file(device_obj, &dev_attr_register293);

  bad_device_create_file_293:
    device_remove_file(device_obj, &dev_attr_register292);

  bad_device_create_file_292:
    device_remove_file(device_obj, &dev_attr_register291);

  bad_device_create_file_291:
    device_remove_file(device_obj, &dev_attr_register290);

  bad_device_create_file_290:
    device_remove_file(device_obj, &dev_attr_register289);

  bad_device_create_file_289:
    device_remove_file(device_obj, &dev_attr_register288);

  bad_device_create_file_288:
    device_remove_file(device_obj, &dev_attr_register287);

  bad_device_create_file_287:
    device_remove_file(device_obj, &dev_attr_register286);

  bad_device_create_file_286:
    device_remove_file(device_obj, &dev_attr_register285);

  bad_device_create_file_285:
    device_remove_file(device_obj, &dev_attr_register284);

  bad_device_create_file_284:
    device_remove_file(device_obj, &dev_attr_register283);

  bad_device_create_file_283:
    device_remove_file(device_obj, &dev_attr_register282);

  bad_device_create_file_282:
    device_remove_file(device_obj, &dev_attr_register281);

  bad_device_create_file_281:
    device_remove_file(device_obj, &dev_attr_register280);

  bad_device_create_file_280:
    device_remove_file(device_obj, &dev_attr_register279);

  bad_device_create_file_279:
    device_remove_file(device_obj, &dev_attr_register278);

  bad_device_create_file_278:
    device_remove_file(device_obj, &dev_attr_register277);

  bad_device_create_file_277:
    device_remove_file(device_obj, &dev_attr_register276);

  bad_device_create_file_276:
    device_remove_file(device_obj, &dev_attr_register275);

  bad_device_create_file_275:
    device_remove_file(device_obj, &dev_attr_register274);

  bad_device_create_file_274:
    device_remove_file(device_obj, &dev_attr_register273);

  bad_device_create_file_273:
    device_remove_file(device_obj, &dev_attr_register272);

  bad_device_create_file_272:
    device_remove_file(device_obj, &dev_attr_register271);

  bad_device_create_file_271:
    device_remove_file(device_obj, &dev_attr_register270);

  bad_device_create_file_270:
    device_remove_file(device_obj, &dev_attr_register269);

  bad_device_create_file_269:
    device_remove_file(device_obj, &dev_attr_register268);

  bad_device_create_file_268:
    device_remove_file(device_obj, &dev_attr_register267);

  bad_device_create_file_267:
    device_remove_file(device_obj, &dev_attr_register266);

  bad_device_create_file_266:
    device_remove_file(device_obj, &dev_attr_register265);

  bad_device_create_file_265:
    device_remove_file(device_obj, &dev_attr_register264);

  bad_device_create_file_264:
    device_remove_file(device_obj, &dev_attr_register263);

  bad_device_create_file_263:
    device_remove_file(device_obj, &dev_attr_register262);

  bad_device_create_file_262:
    device_remove_file(device_obj, &dev_attr_register261);

  bad_device_create_file_261:
    device_remove_file(device_obj, &dev_attr_register260);

  bad_device_create_file_260:
    device_remove_file(device_obj, &dev_attr_register259);

  bad_device_create_file_259:
    device_remove_file(device_obj, &dev_attr_register258);

  bad_device_create_file_258:
    device_remove_file(device_obj, &dev_attr_register257);

  bad_device_create_file_257:
    device_remove_file(device_obj, &dev_attr_register256);

  bad_device_create_file_256:
    device_remove_file(device_obj, &dev_attr_register255);

  bad_device_create_file_255:
    device_remove_file(device_obj, &dev_attr_register254);

  bad_device_create_file_254:
    device_remove_file(device_obj, &dev_attr_register253);

  bad_device_create_file_253:
    device_remove_file(device_obj, &dev_attr_register252);

  bad_device_create_file_252:
    device_remove_file(device_obj, &dev_attr_register251);

  bad_device_create_file_251:
    device_remove_file(device_obj, &dev_attr_register250);

  bad_device_create_file_250:
    device_remove_file(device_obj, &dev_attr_register249);

  bad_device_create_file_249:
    device_remove_file(device_obj, &dev_attr_register248);

  bad_device_create_file_248:
    device_remove_file(device_obj, &dev_attr_register247);

  bad_device_create_file_247:
    device_remove_file(device_obj, &dev_attr_register246);

  bad_device_create_file_246:
    device_remove_file(device_obj, &dev_attr_register245);

  bad_device_create_file_245:
    device_remove_file(device_obj, &dev_attr_register244);

  bad_device_create_file_244:
    device_remove_file(device_obj, &dev_attr_register243);

  bad_device_create_file_243:
    device_remove_file(device_obj, &dev_attr_register242);

  bad_device_create_file_242:
    device_remove_file(device_obj, &dev_attr_register241);

  bad_device_create_file_241:
    device_remove_file(device_obj, &dev_attr_register240);

  bad_device_create_file_240:
    device_remove_file(device_obj, &dev_attr_register239);

  bad_device_create_file_239:
    device_remove_file(device_obj, &dev_attr_register238);

  bad_device_create_file_238:
    device_remove_file(device_obj, &dev_attr_register237);

  bad_device_create_file_237:
    device_remove_file(device_obj, &dev_attr_register236);

  bad_device_create_file_236:
    device_remove_file(device_obj, &dev_attr_register235);

  bad_device_create_file_235:
    device_remove_file(device_obj, &dev_attr_register234);

  bad_device_create_file_234:
    device_remove_file(device_obj, &dev_attr_register233);

  bad_device_create_file_233:
    device_remove_file(device_obj, &dev_attr_register232);

  bad_device_create_file_232:
    device_remove_file(device_obj, &dev_attr_register231);

  bad_device_create_file_231:
    device_remove_file(device_obj, &dev_attr_register230);

  bad_device_create_file_230:
    device_remove_file(device_obj, &dev_attr_register229);

  bad_device_create_file_229:
    device_remove_file(device_obj, &dev_attr_register228);

  bad_device_create_file_228:
    device_remove_file(device_obj, &dev_attr_register227);

  bad_device_create_file_227:
    device_remove_file(device_obj, &dev_attr_register226);

  bad_device_create_file_226:
    device_remove_file(device_obj, &dev_attr_register225);

  bad_device_create_file_225:
    device_remove_file(device_obj, &dev_attr_register224);

  bad_device_create_file_224:
    device_remove_file(device_obj, &dev_attr_register223);

  bad_device_create_file_223:
    device_remove_file(device_obj, &dev_attr_register222);

  bad_device_create_file_222:
    device_remove_file(device_obj, &dev_attr_register221);

  bad_device_create_file_221:
    device_remove_file(device_obj, &dev_attr_register220);

  bad_device_create_file_220:
    device_remove_file(device_obj, &dev_attr_register219);

  bad_device_create_file_219:
    device_remove_file(device_obj, &dev_attr_register218);

  bad_device_create_file_218:
    device_remove_file(device_obj, &dev_attr_register217);

  bad_device_create_file_217:
    device_remove_file(device_obj, &dev_attr_register216);

  bad_device_create_file_216:
    device_remove_file(device_obj, &dev_attr_register215);

  bad_device_create_file_215:
    device_remove_file(device_obj, &dev_attr_register214);

  bad_device_create_file_214:
    device_remove_file(device_obj, &dev_attr_register213);

  bad_device_create_file_213:
    device_remove_file(device_obj, &dev_attr_register212);

  bad_device_create_file_212:
    device_remove_file(device_obj, &dev_attr_register211);

  bad_device_create_file_211:
    device_remove_file(device_obj, &dev_attr_register210);

  bad_device_create_file_210:
    device_remove_file(device_obj, &dev_attr_register209);

  bad_device_create_file_209:
    device_remove_file(device_obj, &dev_attr_register208);

  bad_device_create_file_208:
    device_remove_file(device_obj, &dev_attr_register207);

  bad_device_create_file_207:
    device_remove_file(device_obj, &dev_attr_register206);

  bad_device_create_file_206:
    device_remove_file(device_obj, &dev_attr_register205);

  bad_device_create_file_205:
    device_remove_file(device_obj, &dev_attr_register204);

  bad_device_create_file_204:
    device_remove_file(device_obj, &dev_attr_register203);

  bad_device_create_file_203:
    device_remove_file(device_obj, &dev_attr_register202);

  bad_device_create_file_202:
    device_remove_file(device_obj, &dev_attr_register201);

  bad_device_create_file_201:
    device_remove_file(device_obj, &dev_attr_register200);

  bad_device_create_file_200:
    device_remove_file(device_obj, &dev_attr_register199);

  bad_device_create_file_199:
    device_remove_file(device_obj, &dev_attr_register198);

  bad_device_create_file_198:
    device_remove_file(device_obj, &dev_attr_register197);

  bad_device_create_file_197:
    device_remove_file(device_obj, &dev_attr_register196);

  bad_device_create_file_196:
    device_remove_file(device_obj, &dev_attr_register195);

  bad_device_create_file_195:
    device_remove_file(device_obj, &dev_attr_register194);

  bad_device_create_file_194:
    device_remove_file(device_obj, &dev_attr_register193);

  bad_device_create_file_193:
    device_remove_file(device_obj, &dev_attr_register192);

  bad_device_create_file_192:
    device_remove_file(device_obj, &dev_attr_register191);

  bad_device_create_file_191:
    device_remove_file(device_obj, &dev_attr_register190);

  bad_device_create_file_190:
    device_remove_file(device_obj, &dev_attr_register189);

  bad_device_create_file_189:
    device_remove_file(device_obj, &dev_attr_register188);

  bad_device_create_file_188:
    device_remove_file(device_obj, &dev_attr_register187);

  bad_device_create_file_187:
    device_remove_file(device_obj, &dev_attr_register186);

  bad_device_create_file_186:
    device_remove_file(device_obj, &dev_attr_register185);

  bad_device_create_file_185:
    device_remove_file(device_obj, &dev_attr_register184);

  bad_device_create_file_184:
    device_remove_file(device_obj, &dev_attr_register183);

  bad_device_create_file_183:
    device_remove_file(device_obj, &dev_attr_register182);

  bad_device_create_file_182:
    device_remove_file(device_obj, &dev_attr_register181);

  bad_device_create_file_181:
    device_remove_file(device_obj, &dev_attr_register180);

  bad_device_create_file_180:
    device_remove_file(device_obj, &dev_attr_register179);

  bad_device_create_file_179:
    device_remove_file(device_obj, &dev_attr_register178);

  bad_device_create_file_178:
    device_remove_file(device_obj, &dev_attr_register177);

  bad_device_create_file_177:
    device_remove_file(device_obj, &dev_attr_register176);

  bad_device_create_file_176:
    device_remove_file(device_obj, &dev_attr_register175);

  bad_device_create_file_175:
    device_remove_file(device_obj, &dev_attr_register174);

  bad_device_create_file_174:
    device_remove_file(device_obj, &dev_attr_register173);

  bad_device_create_file_173:
    device_remove_file(device_obj, &dev_attr_register172);

  bad_device_create_file_172:
    device_remove_file(device_obj, &dev_attr_register171);

  bad_device_create_file_171:
    device_remove_file(device_obj, &dev_attr_register170);

  bad_device_create_file_170:
    device_remove_file(device_obj, &dev_attr_register169);

  bad_device_create_file_169:
    device_remove_file(device_obj, &dev_attr_register168);

  bad_device_create_file_168:
    device_remove_file(device_obj, &dev_attr_register167);

  bad_device_create_file_167:
    device_remove_file(device_obj, &dev_attr_register166);

  bad_device_create_file_166:
    device_remove_file(device_obj, &dev_attr_register165);

  bad_device_create_file_165:
    device_remove_file(device_obj, &dev_attr_register164);

  bad_device_create_file_164:
    device_remove_file(device_obj, &dev_attr_register163);

  bad_device_create_file_163:
    device_remove_file(device_obj, &dev_attr_register162);

  bad_device_create_file_162:
    device_remove_file(device_obj, &dev_attr_register161);

  bad_device_create_file_161:
    device_remove_file(device_obj, &dev_attr_register160);

  bad_device_create_file_160:
    device_remove_file(device_obj, &dev_attr_register159);

  bad_device_create_file_159:
    device_remove_file(device_obj, &dev_attr_register158);

  bad_device_create_file_158:
    device_remove_file(device_obj, &dev_attr_register157);

  bad_device_create_file_157:
    device_remove_file(device_obj, &dev_attr_register156);

  bad_device_create_file_156:
    device_remove_file(device_obj, &dev_attr_register155);

  bad_device_create_file_155:
    device_remove_file(device_obj, &dev_attr_register154);

  bad_device_create_file_154:
    device_remove_file(device_obj, &dev_attr_register153);

  bad_device_create_file_153:
    device_remove_file(device_obj, &dev_attr_register152);

  bad_device_create_file_152:
    device_remove_file(device_obj, &dev_attr_register151);

  bad_device_create_file_151:
    device_remove_file(device_obj, &dev_attr_register150);

  bad_device_create_file_150:
    device_remove_file(device_obj, &dev_attr_register149);

  bad_device_create_file_149:
    device_remove_file(device_obj, &dev_attr_register148);

  bad_device_create_file_148:
    device_remove_file(device_obj, &dev_attr_register147);

  bad_device_create_file_147:
    device_remove_file(device_obj, &dev_attr_register146);

  bad_device_create_file_146:
    device_remove_file(device_obj, &dev_attr_register145);

  bad_device_create_file_145:
    device_remove_file(device_obj, &dev_attr_register144);

  bad_device_create_file_144:
    device_remove_file(device_obj, &dev_attr_register143);

  bad_device_create_file_143:
    device_remove_file(device_obj, &dev_attr_register142);

  bad_device_create_file_142:
    device_remove_file(device_obj, &dev_attr_register141);

  bad_device_create_file_141:
    device_remove_file(device_obj, &dev_attr_register140);

  bad_device_create_file_140:
    device_remove_file(device_obj, &dev_attr_register139);

  bad_device_create_file_139:
    device_remove_file(device_obj, &dev_attr_register138);

  bad_device_create_file_138:
    device_remove_file(device_obj, &dev_attr_register137);

  bad_device_create_file_137:
    device_remove_file(device_obj, &dev_attr_register136);

  bad_device_create_file_136:
    device_remove_file(device_obj, &dev_attr_register135);

  bad_device_create_file_135:
    device_remove_file(device_obj, &dev_attr_register134);

  bad_device_create_file_134:
    device_remove_file(device_obj, &dev_attr_register133);

  bad_device_create_file_133:
    device_remove_file(device_obj, &dev_attr_register132);

  bad_device_create_file_132:
    device_remove_file(device_obj, &dev_attr_register131);

  bad_device_create_file_131:
    device_remove_file(device_obj, &dev_attr_register130);

  bad_device_create_file_130:
    device_remove_file(device_obj, &dev_attr_register129);

  bad_device_create_file_129:
    device_remove_file(device_obj, &dev_attr_register128);

  bad_device_create_file_128:
    device_remove_file(device_obj, &dev_attr_register127);

  bad_device_create_file_127:
    device_remove_file(device_obj, &dev_attr_register126);

  bad_device_create_file_126:
    device_remove_file(device_obj, &dev_attr_register125);

  bad_device_create_file_125:
    device_remove_file(device_obj, &dev_attr_register124);

  bad_device_create_file_124:
    device_remove_file(device_obj, &dev_attr_register123);

  bad_device_create_file_123:
    device_remove_file(device_obj, &dev_attr_register122);

  bad_device_create_file_122:
    device_remove_file(device_obj, &dev_attr_register121);

  bad_device_create_file_121:
    device_remove_file(device_obj, &dev_attr_register120);

  bad_device_create_file_120:
    device_remove_file(device_obj, &dev_attr_register119);

  bad_device_create_file_119:
    device_remove_file(device_obj, &dev_attr_register118);

  bad_device_create_file_118:
    device_remove_file(device_obj, &dev_attr_register117);

  bad_device_create_file_117:
    device_remove_file(device_obj, &dev_attr_register116);

  bad_device_create_file_116:
    device_remove_file(device_obj, &dev_attr_register115);

  bad_device_create_file_115:
    device_remove_file(device_obj, &dev_attr_register114);

  bad_device_create_file_114:
    device_remove_file(device_obj, &dev_attr_register113);

  bad_device_create_file_113:
    device_remove_file(device_obj, &dev_attr_register112);

  bad_device_create_file_112:
    device_remove_file(device_obj, &dev_attr_register111);

  bad_device_create_file_111:
    device_remove_file(device_obj, &dev_attr_register110);

  bad_device_create_file_110:
    device_remove_file(device_obj, &dev_attr_register109);

  bad_device_create_file_109:
    device_remove_file(device_obj, &dev_attr_register108);

  bad_device_create_file_108:
    device_remove_file(device_obj, &dev_attr_register107);

  bad_device_create_file_107:
    device_remove_file(device_obj, &dev_attr_register106);

  bad_device_create_file_106:
    device_remove_file(device_obj, &dev_attr_register105);

  bad_device_create_file_105:
    device_remove_file(device_obj, &dev_attr_register104);

  bad_device_create_file_104:
    device_remove_file(device_obj, &dev_attr_register103);

  bad_device_create_file_103:
    device_remove_file(device_obj, &dev_attr_register102);

  bad_device_create_file_102:
    device_remove_file(device_obj, &dev_attr_register101);

  bad_device_create_file_101:
    device_remove_file(device_obj, &dev_attr_register100);

  bad_device_create_file_100:
    device_remove_file(device_obj, &dev_attr_register99);

  bad_device_create_file_99:
    device_remove_file(device_obj, &dev_attr_register98);

  bad_device_create_file_98:
    device_remove_file(device_obj, &dev_attr_register97);

  bad_device_create_file_97:
    device_remove_file(device_obj, &dev_attr_register96);

  bad_device_create_file_96:
    device_remove_file(device_obj, &dev_attr_register95);

  bad_device_create_file_95:
    device_remove_file(device_obj, &dev_attr_register94);

  bad_device_create_file_94:
    device_remove_file(device_obj, &dev_attr_register93);

  bad_device_create_file_93:
    device_remove_file(device_obj, &dev_attr_register92);

  bad_device_create_file_92:
    device_remove_file(device_obj, &dev_attr_register91);

  bad_device_create_file_91:
    device_remove_file(device_obj, &dev_attr_register90);

  bad_device_create_file_90:
    device_remove_file(device_obj, &dev_attr_register89);

  bad_device_create_file_89:
    device_remove_file(device_obj, &dev_attr_register88);

  bad_device_create_file_88:
    device_remove_file(device_obj, &dev_attr_register87);

  bad_device_create_file_87:
    device_remove_file(device_obj, &dev_attr_register86);

  bad_device_create_file_86:
    device_remove_file(device_obj, &dev_attr_register85);

  bad_device_create_file_85:
    device_remove_file(device_obj, &dev_attr_register84);

  bad_device_create_file_84:
    device_remove_file(device_obj, &dev_attr_register83);

  bad_device_create_file_83:
    device_remove_file(device_obj, &dev_attr_register82);

  bad_device_create_file_82:
    device_remove_file(device_obj, &dev_attr_register81);

  bad_device_create_file_81:
    device_remove_file(device_obj, &dev_attr_register80);

  bad_device_create_file_80:
    device_remove_file(device_obj, &dev_attr_register79);

  bad_device_create_file_79:
    device_remove_file(device_obj, &dev_attr_register78);

  bad_device_create_file_78:
    device_remove_file(device_obj, &dev_attr_register77);

  bad_device_create_file_77:
    device_remove_file(device_obj, &dev_attr_register76);

  bad_device_create_file_76:
    device_remove_file(device_obj, &dev_attr_register75);

  bad_device_create_file_75:
    device_remove_file(device_obj, &dev_attr_register74);

  bad_device_create_file_74:
    device_remove_file(device_obj, &dev_attr_register73);

  bad_device_create_file_73:
    device_remove_file(device_obj, &dev_attr_register72);

  bad_device_create_file_72:
    device_remove_file(device_obj, &dev_attr_register71);

  bad_device_create_file_71:
    device_remove_file(device_obj, &dev_attr_register70);

  bad_device_create_file_70:
    device_remove_file(device_obj, &dev_attr_register69);

  bad_device_create_file_69:
    device_remove_file(device_obj, &dev_attr_register68);

  bad_device_create_file_68:
    device_remove_file(device_obj, &dev_attr_register67);

  bad_device_create_file_67:
    device_remove_file(device_obj, &dev_attr_register66);

  bad_device_create_file_66:
    device_remove_file(device_obj, &dev_attr_register65);

  bad_device_create_file_65:
    device_remove_file(device_obj, &dev_attr_register64);

  bad_device_create_file_64:
    device_remove_file(device_obj, &dev_attr_register63);

  bad_device_create_file_63:
    device_remove_file(device_obj, &dev_attr_register62);

  bad_device_create_file_62:
    device_remove_file(device_obj, &dev_attr_register61);

  bad_device_create_file_61:
    device_remove_file(device_obj, &dev_attr_register60);

  bad_device_create_file_60:
    device_remove_file(device_obj, &dev_attr_register59);

  bad_device_create_file_59:
    device_remove_file(device_obj, &dev_attr_register58);

  bad_device_create_file_58:
    device_remove_file(device_obj, &dev_attr_register57);

  bad_device_create_file_57:
    device_remove_file(device_obj, &dev_attr_register56);

  bad_device_create_file_56:
    device_remove_file(device_obj, &dev_attr_register55);

  bad_device_create_file_55:
    device_remove_file(device_obj, &dev_attr_register54);

  bad_device_create_file_54:
    device_remove_file(device_obj, &dev_attr_register53);

  bad_device_create_file_53:
    device_remove_file(device_obj, &dev_attr_register52);

  bad_device_create_file_52:
    device_remove_file(device_obj, &dev_attr_register51);

  bad_device_create_file_51:
    device_remove_file(device_obj, &dev_attr_register50);

  bad_device_create_file_50:
    device_remove_file(device_obj, &dev_attr_register49);

  bad_device_create_file_49:
    device_remove_file(device_obj, &dev_attr_register48);

  bad_device_create_file_48:
    device_remove_file(device_obj, &dev_attr_register47);

  bad_device_create_file_47:
    device_remove_file(device_obj, &dev_attr_register46);

  bad_device_create_file_46:
    device_remove_file(device_obj, &dev_attr_register45);

  bad_device_create_file_45:
    device_remove_file(device_obj, &dev_attr_register44);

  bad_device_create_file_44:
    device_remove_file(device_obj, &dev_attr_register43);

  bad_device_create_file_43:
    device_remove_file(device_obj, &dev_attr_register42);

  bad_device_create_file_42:
    device_remove_file(device_obj, &dev_attr_register41);

  bad_device_create_file_41:
    device_remove_file(device_obj, &dev_attr_register40);

  bad_device_create_file_40:
    device_remove_file(device_obj, &dev_attr_register39);

  bad_device_create_file_39:
    device_remove_file(device_obj, &dev_attr_register38);

  bad_device_create_file_38:
    device_remove_file(device_obj, &dev_attr_register37);

  bad_device_create_file_37:
    device_remove_file(device_obj, &dev_attr_register36);

  bad_device_create_file_36:
    device_remove_file(device_obj, &dev_attr_register35);

  bad_device_create_file_35:
    device_remove_file(device_obj, &dev_attr_register34);

  bad_device_create_file_34:
    device_remove_file(device_obj, &dev_attr_register33);

  bad_device_create_file_33:
    device_remove_file(device_obj, &dev_attr_register32);

  bad_device_create_file_32:
    device_remove_file(device_obj, &dev_attr_register31);

  bad_device_create_file_31:
    device_remove_file(device_obj, &dev_attr_register30);

  bad_device_create_file_30:
    device_remove_file(device_obj, &dev_attr_register29);

  bad_device_create_file_29:
    device_remove_file(device_obj, &dev_attr_register28);

  bad_device_create_file_28:
    device_remove_file(device_obj, &dev_attr_register27);

  bad_device_create_file_27:
    device_remove_file(device_obj, &dev_attr_register26);

  bad_device_create_file_26:
    device_remove_file(device_obj, &dev_attr_register25);

  bad_device_create_file_25:
    device_remove_file(device_obj, &dev_attr_register24);

  bad_device_create_file_24:
    device_remove_file(device_obj, &dev_attr_register23);

  bad_device_create_file_23:
    device_remove_file(device_obj, &dev_attr_register22);

  bad_device_create_file_22:
    device_remove_file(device_obj, &dev_attr_register21);

  bad_device_create_file_21:
    device_remove_file(device_obj, &dev_attr_register20);

  bad_device_create_file_20:
    device_remove_file(device_obj, &dev_attr_register19);

  bad_device_create_file_19:
    device_remove_file(device_obj, &dev_attr_register18);

  bad_device_create_file_18:
    device_remove_file(device_obj, &dev_attr_register17);

  bad_device_create_file_17:
    device_remove_file(device_obj, &dev_attr_register16);

  bad_device_create_file_16:
    device_remove_file(device_obj, &dev_attr_register15);

  bad_device_create_file_15:
    device_remove_file(device_obj, &dev_attr_register14);

  bad_device_create_file_14:
    device_remove_file(device_obj, &dev_attr_register13);

  bad_device_create_file_13:
    device_remove_file(device_obj, &dev_attr_register12);

  bad_device_create_file_12:
    device_remove_file(device_obj, &dev_attr_register11);

  bad_device_create_file_11:
    device_remove_file(device_obj, &dev_attr_register10);

  bad_device_create_file_10:
    device_remove_file(device_obj, &dev_attr_register9);

  bad_device_create_file_9:
    device_remove_file(device_obj, &dev_attr_register8);

  bad_device_create_file_8:
    device_remove_file(device_obj, &dev_attr_register7);

  bad_device_create_file_7:
    device_remove_file(device_obj, &dev_attr_register6);

  bad_device_create_file_6:
    device_remove_file(device_obj, &dev_attr_register5);

  bad_device_create_file_5:
    device_remove_file(device_obj, &dev_attr_register4);

  bad_device_create_file_4:
    device_remove_file(device_obj, &dev_attr_register3);

  bad_device_create_file_3:
    device_remove_file(device_obj, &dev_attr_register2);

  bad_device_create_file_2:
    device_remove_file(device_obj, &dev_attr_register1);

  bad_device_create_file_1:
    device_remove_file(device_obj, &dev_attr_register0);

  bad_device_create_file_0:
    device_destroy(cl, dev_num);

  bad_device_create:
    cdev_del(&fe_DPRAM_devp->cdev);

  bad_cdev_add:
    class_destroy(cl);

  bad_class_create:
    unregister_chrdev_region(dev_num, 1);

  bad_alloc_chrdev_region:

  bad_mem_alloc:

  bad_ioremap:
    ret_val = PTR_ERR(fe_DPRAM_devp->regs);

  bad_exit_return:
    pr_info("DPRAM_probe bad exit\n");
  
  return ret_val;
}

/* FPGA Attribute functions */
static ssize_t register0_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register0, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register0_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register0 = tempValue;
  iowrite32(devp->register0,(u32 *)devp->regs + 0);  return count;
}

static ssize_t register1_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register1, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register1_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register1 = tempValue;
  iowrite32(devp->register1,(u32 *)devp->regs + 1);  return count;
}

static ssize_t register2_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register2, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register2_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register2 = tempValue;
  iowrite32(devp->register2,(u32 *)devp->regs + 2);  return count;
}

static ssize_t register3_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register3, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register3_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register3 = tempValue;
  iowrite32(devp->register3,(u32 *)devp->regs + 3);  return count;
}

static ssize_t register4_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register4, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register4_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register4 = tempValue;
  iowrite32(devp->register4,(u32 *)devp->regs + 4);  return count;
}

static ssize_t register5_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register5, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register5_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register5 = tempValue;
  iowrite32(devp->register5,(u32 *)devp->regs + 5);  return count;
}

static ssize_t register6_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register6, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register6_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register6 = tempValue;
  iowrite32(devp->register6,(u32 *)devp->regs + 6);  return count;
}

static ssize_t register7_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register7, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register7_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register7 = tempValue;
  iowrite32(devp->register7,(u32 *)devp->regs + 7);  return count;
}

static ssize_t register8_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register8, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register8_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register8 = tempValue;
  iowrite32(devp->register8,(u32 *)devp->regs + 8);  return count;
}

static ssize_t register9_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register9, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register9_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register9 = tempValue;
  iowrite32(devp->register9,(u32 *)devp->regs + 9);  return count;
}

static ssize_t register10_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register10, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register10_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register10 = tempValue;
  iowrite32(devp->register10,(u32 *)devp->regs + 10);  return count;
}

static ssize_t register11_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register11, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register11_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register11 = tempValue;
  iowrite32(devp->register11,(u32 *)devp->regs + 11);  return count;
}

static ssize_t register12_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register12, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register12_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register12 = tempValue;
  iowrite32(devp->register12,(u32 *)devp->regs + 12);  return count;
}

static ssize_t register13_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register13, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register13_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register13 = tempValue;
  iowrite32(devp->register13,(u32 *)devp->regs + 13);  return count;
}

static ssize_t register14_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register14, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register14_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register14 = tempValue;
  iowrite32(devp->register14,(u32 *)devp->regs + 14);  return count;
}

static ssize_t register15_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register15, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register15_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register15 = tempValue;
  iowrite32(devp->register15,(u32 *)devp->regs + 15);  return count;
}

static ssize_t register16_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register16, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register16_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register16 = tempValue;
  iowrite32(devp->register16,(u32 *)devp->regs + 16);  return count;
}

static ssize_t register17_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register17, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register17_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register17 = tempValue;
  iowrite32(devp->register17,(u32 *)devp->regs + 17);  return count;
}

static ssize_t register18_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register18, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register18_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register18 = tempValue;
  iowrite32(devp->register18,(u32 *)devp->regs + 18);  return count;
}

static ssize_t register19_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register19, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register19_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register19 = tempValue;
  iowrite32(devp->register19,(u32 *)devp->regs + 19);  return count;
}

static ssize_t register20_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register20, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register20_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register20 = tempValue;
  iowrite32(devp->register20,(u32 *)devp->regs + 20);  return count;
}

static ssize_t register21_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register21, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register21_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register21 = tempValue;
  iowrite32(devp->register21,(u32 *)devp->regs + 21);  return count;
}

static ssize_t register22_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register22, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register22_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register22 = tempValue;
  iowrite32(devp->register22,(u32 *)devp->regs + 22);  return count;
}

static ssize_t register23_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register23, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register23_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register23 = tempValue;
  iowrite32(devp->register23,(u32 *)devp->regs + 23);  return count;
}

static ssize_t register24_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register24, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register24_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register24 = tempValue;
  iowrite32(devp->register24,(u32 *)devp->regs + 24);  return count;
}

static ssize_t register25_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register25, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register25_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register25 = tempValue;
  iowrite32(devp->register25,(u32 *)devp->regs + 25);  return count;
}

static ssize_t register26_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register26, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register26_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register26 = tempValue;
  iowrite32(devp->register26,(u32 *)devp->regs + 26);  return count;
}

static ssize_t register27_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register27, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register27_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register27 = tempValue;
  iowrite32(devp->register27,(u32 *)devp->regs + 27);  return count;
}

static ssize_t register28_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register28, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register28_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register28 = tempValue;
  iowrite32(devp->register28,(u32 *)devp->regs + 28);  return count;
}

static ssize_t register29_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register29, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register29_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register29 = tempValue;
  iowrite32(devp->register29,(u32 *)devp->regs + 29);  return count;
}

static ssize_t register30_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register30, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register30_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register30 = tempValue;
  iowrite32(devp->register30,(u32 *)devp->regs + 30);  return count;
}

static ssize_t register31_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register31, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register31_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register31 = tempValue;
  iowrite32(devp->register31,(u32 *)devp->regs + 31);  return count;
}

static ssize_t register32_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register32, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register32_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register32 = tempValue;
  iowrite32(devp->register32,(u32 *)devp->regs + 32);  return count;
}

static ssize_t register33_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register33, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register33_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register33 = tempValue;
  iowrite32(devp->register33,(u32 *)devp->regs + 33);  return count;
}

static ssize_t register34_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register34, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register34_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register34 = tempValue;
  iowrite32(devp->register34,(u32 *)devp->regs + 34);  return count;
}

static ssize_t register35_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register35, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register35_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register35 = tempValue;
  iowrite32(devp->register35,(u32 *)devp->regs + 35);  return count;
}

static ssize_t register36_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register36, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register36_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register36 = tempValue;
  iowrite32(devp->register36,(u32 *)devp->regs + 36);  return count;
}

static ssize_t register37_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register37, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register37_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register37 = tempValue;
  iowrite32(devp->register37,(u32 *)devp->regs + 37);  return count;
}

static ssize_t register38_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register38, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register38_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register38 = tempValue;
  iowrite32(devp->register38,(u32 *)devp->regs + 38);  return count;
}

static ssize_t register39_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register39, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register39_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register39 = tempValue;
  iowrite32(devp->register39,(u32 *)devp->regs + 39);  return count;
}

static ssize_t register40_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register40, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register40_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register40 = tempValue;
  iowrite32(devp->register40,(u32 *)devp->regs + 40);  return count;
}

static ssize_t register41_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register41, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register41_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register41 = tempValue;
  iowrite32(devp->register41,(u32 *)devp->regs + 41);  return count;
}

static ssize_t register42_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register42, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register42_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register42 = tempValue;
  iowrite32(devp->register42,(u32 *)devp->regs + 42);  return count;
}

static ssize_t register43_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register43, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register43_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register43 = tempValue;
  iowrite32(devp->register43,(u32 *)devp->regs + 43);  return count;
}

static ssize_t register44_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register44, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register44_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register44 = tempValue;
  iowrite32(devp->register44,(u32 *)devp->regs + 44);  return count;
}

static ssize_t register45_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register45, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register45_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register45 = tempValue;
  iowrite32(devp->register45,(u32 *)devp->regs + 45);  return count;
}

static ssize_t register46_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register46, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register46_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register46 = tempValue;
  iowrite32(devp->register46,(u32 *)devp->regs + 46);  return count;
}

static ssize_t register47_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register47, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register47_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register47 = tempValue;
  iowrite32(devp->register47,(u32 *)devp->regs + 47);  return count;
}

static ssize_t register48_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register48, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register48_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register48 = tempValue;
  iowrite32(devp->register48,(u32 *)devp->regs + 48);  return count;
}

static ssize_t register49_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register49, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register49_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register49 = tempValue;
  iowrite32(devp->register49,(u32 *)devp->regs + 49);  return count;
}

static ssize_t register50_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register50, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register50_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register50 = tempValue;
  iowrite32(devp->register50,(u32 *)devp->regs + 50);  return count;
}

static ssize_t register51_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register51, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register51_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register51 = tempValue;
  iowrite32(devp->register51,(u32 *)devp->regs + 51);  return count;
}

static ssize_t register52_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register52, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register52_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register52 = tempValue;
  iowrite32(devp->register52,(u32 *)devp->regs + 52);  return count;
}

static ssize_t register53_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register53, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register53_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register53 = tempValue;
  iowrite32(devp->register53,(u32 *)devp->regs + 53);  return count;
}

static ssize_t register54_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register54, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register54_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register54 = tempValue;
  iowrite32(devp->register54,(u32 *)devp->regs + 54);  return count;
}

static ssize_t register55_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register55, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register55_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register55 = tempValue;
  iowrite32(devp->register55,(u32 *)devp->regs + 55);  return count;
}

static ssize_t register56_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register56, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register56_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register56 = tempValue;
  iowrite32(devp->register56,(u32 *)devp->regs + 56);  return count;
}

static ssize_t register57_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register57, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register57_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register57 = tempValue;
  iowrite32(devp->register57,(u32 *)devp->regs + 57);  return count;
}

static ssize_t register58_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register58, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register58_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register58 = tempValue;
  iowrite32(devp->register58,(u32 *)devp->regs + 58);  return count;
}

static ssize_t register59_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register59, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register59_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register59 = tempValue;
  iowrite32(devp->register59,(u32 *)devp->regs + 59);  return count;
}

static ssize_t register60_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register60, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register60_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register60 = tempValue;
  iowrite32(devp->register60,(u32 *)devp->regs + 60);  return count;
}

static ssize_t register61_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register61, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register61_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register61 = tempValue;
  iowrite32(devp->register61,(u32 *)devp->regs + 61);  return count;
}

static ssize_t register62_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register62, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register62_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register62 = tempValue;
  iowrite32(devp->register62,(u32 *)devp->regs + 62);  return count;
}

static ssize_t register63_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register63, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register63_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register63 = tempValue;
  iowrite32(devp->register63,(u32 *)devp->regs + 63);  return count;
}

static ssize_t register64_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register64, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register64_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register64 = tempValue;
  iowrite32(devp->register64,(u32 *)devp->regs + 64);  return count;
}

static ssize_t register65_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register65, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register65_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register65 = tempValue;
  iowrite32(devp->register65,(u32 *)devp->regs + 65);  return count;
}

static ssize_t register66_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register66, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register66_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register66 = tempValue;
  iowrite32(devp->register66,(u32 *)devp->regs + 66);  return count;
}

static ssize_t register67_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register67, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register67_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register67 = tempValue;
  iowrite32(devp->register67,(u32 *)devp->regs + 67);  return count;
}

static ssize_t register68_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register68, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register68_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register68 = tempValue;
  iowrite32(devp->register68,(u32 *)devp->regs + 68);  return count;
}

static ssize_t register69_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register69, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register69_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register69 = tempValue;
  iowrite32(devp->register69,(u32 *)devp->regs + 69);  return count;
}

static ssize_t register70_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register70, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register70_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register70 = tempValue;
  iowrite32(devp->register70,(u32 *)devp->regs + 70);  return count;
}

static ssize_t register71_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register71, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register71_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register71 = tempValue;
  iowrite32(devp->register71,(u32 *)devp->regs + 71);  return count;
}

static ssize_t register72_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register72, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register72_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register72 = tempValue;
  iowrite32(devp->register72,(u32 *)devp->regs + 72);  return count;
}

static ssize_t register73_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register73, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register73_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register73 = tempValue;
  iowrite32(devp->register73,(u32 *)devp->regs + 73);  return count;
}

static ssize_t register74_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register74, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register74_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register74 = tempValue;
  iowrite32(devp->register74,(u32 *)devp->regs + 74);  return count;
}

static ssize_t register75_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register75, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register75_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register75 = tempValue;
  iowrite32(devp->register75,(u32 *)devp->regs + 75);  return count;
}

static ssize_t register76_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register76, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register76_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register76 = tempValue;
  iowrite32(devp->register76,(u32 *)devp->regs + 76);  return count;
}

static ssize_t register77_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register77, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register77_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register77 = tempValue;
  iowrite32(devp->register77,(u32 *)devp->regs + 77);  return count;
}

static ssize_t register78_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register78, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register78_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register78 = tempValue;
  iowrite32(devp->register78,(u32 *)devp->regs + 78);  return count;
}

static ssize_t register79_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register79, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register79_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register79 = tempValue;
  iowrite32(devp->register79,(u32 *)devp->regs + 79);  return count;
}

static ssize_t register80_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register80, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register80_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register80 = tempValue;
  iowrite32(devp->register80,(u32 *)devp->regs + 80);  return count;
}

static ssize_t register81_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register81, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register81_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register81 = tempValue;
  iowrite32(devp->register81,(u32 *)devp->regs + 81);  return count;
}

static ssize_t register82_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register82, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register82_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register82 = tempValue;
  iowrite32(devp->register82,(u32 *)devp->regs + 82);  return count;
}

static ssize_t register83_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register83, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register83_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register83 = tempValue;
  iowrite32(devp->register83,(u32 *)devp->regs + 83);  return count;
}

static ssize_t register84_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register84, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register84_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register84 = tempValue;
  iowrite32(devp->register84,(u32 *)devp->regs + 84);  return count;
}

static ssize_t register85_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register85, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register85_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register85 = tempValue;
  iowrite32(devp->register85,(u32 *)devp->regs + 85);  return count;
}

static ssize_t register86_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register86, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register86_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register86 = tempValue;
  iowrite32(devp->register86,(u32 *)devp->regs + 86);  return count;
}

static ssize_t register87_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register87, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register87_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register87 = tempValue;
  iowrite32(devp->register87,(u32 *)devp->regs + 87);  return count;
}

static ssize_t register88_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register88, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register88_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register88 = tempValue;
  iowrite32(devp->register88,(u32 *)devp->regs + 88);  return count;
}

static ssize_t register89_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register89, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register89_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register89 = tempValue;
  iowrite32(devp->register89,(u32 *)devp->regs + 89);  return count;
}

static ssize_t register90_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register90, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register90_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register90 = tempValue;
  iowrite32(devp->register90,(u32 *)devp->regs + 90);  return count;
}

static ssize_t register91_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register91, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register91_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register91 = tempValue;
  iowrite32(devp->register91,(u32 *)devp->regs + 91);  return count;
}

static ssize_t register92_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register92, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register92_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register92 = tempValue;
  iowrite32(devp->register92,(u32 *)devp->regs + 92);  return count;
}

static ssize_t register93_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register93, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register93_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register93 = tempValue;
  iowrite32(devp->register93,(u32 *)devp->regs + 93);  return count;
}

static ssize_t register94_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register94, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register94_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register94 = tempValue;
  iowrite32(devp->register94,(u32 *)devp->regs + 94);  return count;
}

static ssize_t register95_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register95, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register95_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register95 = tempValue;
  iowrite32(devp->register95,(u32 *)devp->regs + 95);  return count;
}

static ssize_t register96_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register96, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register96_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register96 = tempValue;
  iowrite32(devp->register96,(u32 *)devp->regs + 96);  return count;
}

static ssize_t register97_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register97, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register97_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register97 = tempValue;
  iowrite32(devp->register97,(u32 *)devp->regs + 97);  return count;
}

static ssize_t register98_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register98, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register98_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register98 = tempValue;
  iowrite32(devp->register98,(u32 *)devp->regs + 98);  return count;
}

static ssize_t register99_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register99, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register99_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register99 = tempValue;
  iowrite32(devp->register99,(u32 *)devp->regs + 99);  return count;
}

static ssize_t register100_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register100, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register100_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register100 = tempValue;
  iowrite32(devp->register100,(u32 *)devp->regs + 100);  return count;
}

static ssize_t register101_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register101, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register101_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register101 = tempValue;
  iowrite32(devp->register101,(u32 *)devp->regs + 101);  return count;
}

static ssize_t register102_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register102, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register102_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register102 = tempValue;
  iowrite32(devp->register102,(u32 *)devp->regs + 102);  return count;
}

static ssize_t register103_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register103, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register103_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register103 = tempValue;
  iowrite32(devp->register103,(u32 *)devp->regs + 103);  return count;
}

static ssize_t register104_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register104, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register104_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register104 = tempValue;
  iowrite32(devp->register104,(u32 *)devp->regs + 104);  return count;
}

static ssize_t register105_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register105, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register105_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register105 = tempValue;
  iowrite32(devp->register105,(u32 *)devp->regs + 105);  return count;
}

static ssize_t register106_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register106, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register106_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register106 = tempValue;
  iowrite32(devp->register106,(u32 *)devp->regs + 106);  return count;
}

static ssize_t register107_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register107, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register107_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register107 = tempValue;
  iowrite32(devp->register107,(u32 *)devp->regs + 107);  return count;
}

static ssize_t register108_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register108, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register108_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register108 = tempValue;
  iowrite32(devp->register108,(u32 *)devp->regs + 108);  return count;
}

static ssize_t register109_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register109, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register109_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register109 = tempValue;
  iowrite32(devp->register109,(u32 *)devp->regs + 109);  return count;
}

static ssize_t register110_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register110, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register110_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register110 = tempValue;
  iowrite32(devp->register110,(u32 *)devp->regs + 110);  return count;
}

static ssize_t register111_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register111, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register111_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register111 = tempValue;
  iowrite32(devp->register111,(u32 *)devp->regs + 111);  return count;
}

static ssize_t register112_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register112, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register112_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register112 = tempValue;
  iowrite32(devp->register112,(u32 *)devp->regs + 112);  return count;
}

static ssize_t register113_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register113, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register113_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register113 = tempValue;
  iowrite32(devp->register113,(u32 *)devp->regs + 113);  return count;
}

static ssize_t register114_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register114, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register114_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register114 = tempValue;
  iowrite32(devp->register114,(u32 *)devp->regs + 114);  return count;
}

static ssize_t register115_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register115, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register115_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register115 = tempValue;
  iowrite32(devp->register115,(u32 *)devp->regs + 115);  return count;
}

static ssize_t register116_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register116, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register116_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register116 = tempValue;
  iowrite32(devp->register116,(u32 *)devp->regs + 116);  return count;
}

static ssize_t register117_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register117, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register117_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register117 = tempValue;
  iowrite32(devp->register117,(u32 *)devp->regs + 117);  return count;
}

static ssize_t register118_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register118, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register118_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register118 = tempValue;
  iowrite32(devp->register118,(u32 *)devp->regs + 118);  return count;
}

static ssize_t register119_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register119, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register119_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register119 = tempValue;
  iowrite32(devp->register119,(u32 *)devp->regs + 119);  return count;
}

static ssize_t register120_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register120, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register120_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register120 = tempValue;
  iowrite32(devp->register120,(u32 *)devp->regs + 120);  return count;
}

static ssize_t register121_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register121, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register121_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register121 = tempValue;
  iowrite32(devp->register121,(u32 *)devp->regs + 121);  return count;
}

static ssize_t register122_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register122, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register122_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register122 = tempValue;
  iowrite32(devp->register122,(u32 *)devp->regs + 122);  return count;
}

static ssize_t register123_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register123, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register123_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register123 = tempValue;
  iowrite32(devp->register123,(u32 *)devp->regs + 123);  return count;
}

static ssize_t register124_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register124, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register124_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register124 = tempValue;
  iowrite32(devp->register124,(u32 *)devp->regs + 124);  return count;
}

static ssize_t register125_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register125, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register125_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register125 = tempValue;
  iowrite32(devp->register125,(u32 *)devp->regs + 125);  return count;
}

static ssize_t register126_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register126, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register126_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register126 = tempValue;
  iowrite32(devp->register126,(u32 *)devp->regs + 126);  return count;
}

static ssize_t register127_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register127, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register127_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register127 = tempValue;
  iowrite32(devp->register127,(u32 *)devp->regs + 127);  return count;
}

static ssize_t register128_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register128, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register128_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register128 = tempValue;
  iowrite32(devp->register128,(u32 *)devp->regs + 128);  return count;
}

static ssize_t register129_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register129, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register129_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register129 = tempValue;
  iowrite32(devp->register129,(u32 *)devp->regs + 129);  return count;
}

static ssize_t register130_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register130, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register130_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register130 = tempValue;
  iowrite32(devp->register130,(u32 *)devp->regs + 130);  return count;
}

static ssize_t register131_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register131, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register131_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register131 = tempValue;
  iowrite32(devp->register131,(u32 *)devp->regs + 131);  return count;
}

static ssize_t register132_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register132, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register132_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register132 = tempValue;
  iowrite32(devp->register132,(u32 *)devp->regs + 132);  return count;
}

static ssize_t register133_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register133, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register133_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register133 = tempValue;
  iowrite32(devp->register133,(u32 *)devp->regs + 133);  return count;
}

static ssize_t register134_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register134, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register134_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register134 = tempValue;
  iowrite32(devp->register134,(u32 *)devp->regs + 134);  return count;
}

static ssize_t register135_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register135, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register135_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register135 = tempValue;
  iowrite32(devp->register135,(u32 *)devp->regs + 135);  return count;
}

static ssize_t register136_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register136, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register136_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register136 = tempValue;
  iowrite32(devp->register136,(u32 *)devp->regs + 136);  return count;
}

static ssize_t register137_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register137, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register137_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register137 = tempValue;
  iowrite32(devp->register137,(u32 *)devp->regs + 137);  return count;
}

static ssize_t register138_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register138, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register138_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register138 = tempValue;
  iowrite32(devp->register138,(u32 *)devp->regs + 138);  return count;
}

static ssize_t register139_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register139, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register139_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register139 = tempValue;
  iowrite32(devp->register139,(u32 *)devp->regs + 139);  return count;
}

static ssize_t register140_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register140, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register140_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register140 = tempValue;
  iowrite32(devp->register140,(u32 *)devp->regs + 140);  return count;
}

static ssize_t register141_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register141, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register141_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register141 = tempValue;
  iowrite32(devp->register141,(u32 *)devp->regs + 141);  return count;
}

static ssize_t register142_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register142, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register142_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register142 = tempValue;
  iowrite32(devp->register142,(u32 *)devp->regs + 142);  return count;
}

static ssize_t register143_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register143, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register143_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register143 = tempValue;
  iowrite32(devp->register143,(u32 *)devp->regs + 143);  return count;
}

static ssize_t register144_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register144, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register144_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register144 = tempValue;
  iowrite32(devp->register144,(u32 *)devp->regs + 144);  return count;
}

static ssize_t register145_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register145, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register145_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register145 = tempValue;
  iowrite32(devp->register145,(u32 *)devp->regs + 145);  return count;
}

static ssize_t register146_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register146, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register146_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register146 = tempValue;
  iowrite32(devp->register146,(u32 *)devp->regs + 146);  return count;
}

static ssize_t register147_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register147, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register147_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register147 = tempValue;
  iowrite32(devp->register147,(u32 *)devp->regs + 147);  return count;
}

static ssize_t register148_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register148, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register148_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register148 = tempValue;
  iowrite32(devp->register148,(u32 *)devp->regs + 148);  return count;
}

static ssize_t register149_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register149, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register149_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register149 = tempValue;
  iowrite32(devp->register149,(u32 *)devp->regs + 149);  return count;
}

static ssize_t register150_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register150, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register150_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register150 = tempValue;
  iowrite32(devp->register150,(u32 *)devp->regs + 150);  return count;
}

static ssize_t register151_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register151, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register151_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register151 = tempValue;
  iowrite32(devp->register151,(u32 *)devp->regs + 151);  return count;
}

static ssize_t register152_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register152, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register152_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register152 = tempValue;
  iowrite32(devp->register152,(u32 *)devp->regs + 152);  return count;
}

static ssize_t register153_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register153, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register153_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register153 = tempValue;
  iowrite32(devp->register153,(u32 *)devp->regs + 153);  return count;
}

static ssize_t register154_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register154, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register154_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register154 = tempValue;
  iowrite32(devp->register154,(u32 *)devp->regs + 154);  return count;
}

static ssize_t register155_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register155, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register155_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register155 = tempValue;
  iowrite32(devp->register155,(u32 *)devp->regs + 155);  return count;
}

static ssize_t register156_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register156, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register156_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register156 = tempValue;
  iowrite32(devp->register156,(u32 *)devp->regs + 156);  return count;
}

static ssize_t register157_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register157, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register157_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register157 = tempValue;
  iowrite32(devp->register157,(u32 *)devp->regs + 157);  return count;
}

static ssize_t register158_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register158, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register158_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register158 = tempValue;
  iowrite32(devp->register158,(u32 *)devp->regs + 158);  return count;
}

static ssize_t register159_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register159, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register159_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register159 = tempValue;
  iowrite32(devp->register159,(u32 *)devp->regs + 159);  return count;
}

static ssize_t register160_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register160, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register160_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register160 = tempValue;
  iowrite32(devp->register160,(u32 *)devp->regs + 160);  return count;
}

static ssize_t register161_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register161, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register161_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register161 = tempValue;
  iowrite32(devp->register161,(u32 *)devp->regs + 161);  return count;
}

static ssize_t register162_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register162, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register162_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register162 = tempValue;
  iowrite32(devp->register162,(u32 *)devp->regs + 162);  return count;
}

static ssize_t register163_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register163, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register163_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register163 = tempValue;
  iowrite32(devp->register163,(u32 *)devp->regs + 163);  return count;
}

static ssize_t register164_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register164, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register164_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register164 = tempValue;
  iowrite32(devp->register164,(u32 *)devp->regs + 164);  return count;
}

static ssize_t register165_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register165, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register165_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register165 = tempValue;
  iowrite32(devp->register165,(u32 *)devp->regs + 165);  return count;
}

static ssize_t register166_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register166, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register166_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register166 = tempValue;
  iowrite32(devp->register166,(u32 *)devp->regs + 166);  return count;
}

static ssize_t register167_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register167, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register167_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register167 = tempValue;
  iowrite32(devp->register167,(u32 *)devp->regs + 167);  return count;
}

static ssize_t register168_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register168, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register168_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register168 = tempValue;
  iowrite32(devp->register168,(u32 *)devp->regs + 168);  return count;
}

static ssize_t register169_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register169, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register169_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register169 = tempValue;
  iowrite32(devp->register169,(u32 *)devp->regs + 169);  return count;
}

static ssize_t register170_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register170, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register170_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register170 = tempValue;
  iowrite32(devp->register170,(u32 *)devp->regs + 170);  return count;
}

static ssize_t register171_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register171, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register171_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register171 = tempValue;
  iowrite32(devp->register171,(u32 *)devp->regs + 171);  return count;
}

static ssize_t register172_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register172, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register172_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register172 = tempValue;
  iowrite32(devp->register172,(u32 *)devp->regs + 172);  return count;
}

static ssize_t register173_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register173, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register173_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register173 = tempValue;
  iowrite32(devp->register173,(u32 *)devp->regs + 173);  return count;
}

static ssize_t register174_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register174, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register174_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register174 = tempValue;
  iowrite32(devp->register174,(u32 *)devp->regs + 174);  return count;
}

static ssize_t register175_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register175, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register175_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register175 = tempValue;
  iowrite32(devp->register175,(u32 *)devp->regs + 175);  return count;
}

static ssize_t register176_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register176, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register176_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register176 = tempValue;
  iowrite32(devp->register176,(u32 *)devp->regs + 176);  return count;
}

static ssize_t register177_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register177, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register177_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register177 = tempValue;
  iowrite32(devp->register177,(u32 *)devp->regs + 177);  return count;
}

static ssize_t register178_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register178, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register178_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register178 = tempValue;
  iowrite32(devp->register178,(u32 *)devp->regs + 178);  return count;
}

static ssize_t register179_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register179, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register179_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register179 = tempValue;
  iowrite32(devp->register179,(u32 *)devp->regs + 179);  return count;
}

static ssize_t register180_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register180, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register180_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register180 = tempValue;
  iowrite32(devp->register180,(u32 *)devp->regs + 180);  return count;
}

static ssize_t register181_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register181, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register181_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register181 = tempValue;
  iowrite32(devp->register181,(u32 *)devp->regs + 181);  return count;
}

static ssize_t register182_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register182, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register182_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register182 = tempValue;
  iowrite32(devp->register182,(u32 *)devp->regs + 182);  return count;
}

static ssize_t register183_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register183, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register183_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register183 = tempValue;
  iowrite32(devp->register183,(u32 *)devp->regs + 183);  return count;
}

static ssize_t register184_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register184, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register184_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register184 = tempValue;
  iowrite32(devp->register184,(u32 *)devp->regs + 184);  return count;
}

static ssize_t register185_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register185, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register185_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register185 = tempValue;
  iowrite32(devp->register185,(u32 *)devp->regs + 185);  return count;
}

static ssize_t register186_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register186, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register186_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register186 = tempValue;
  iowrite32(devp->register186,(u32 *)devp->regs + 186);  return count;
}

static ssize_t register187_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register187, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register187_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register187 = tempValue;
  iowrite32(devp->register187,(u32 *)devp->regs + 187);  return count;
}

static ssize_t register188_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register188, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register188_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register188 = tempValue;
  iowrite32(devp->register188,(u32 *)devp->regs + 188);  return count;
}

static ssize_t register189_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register189, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register189_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register189 = tempValue;
  iowrite32(devp->register189,(u32 *)devp->regs + 189);  return count;
}

static ssize_t register190_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register190, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register190_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register190 = tempValue;
  iowrite32(devp->register190,(u32 *)devp->regs + 190);  return count;
}

static ssize_t register191_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register191, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register191_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register191 = tempValue;
  iowrite32(devp->register191,(u32 *)devp->regs + 191);  return count;
}

static ssize_t register192_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register192, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register192_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register192 = tempValue;
  iowrite32(devp->register192,(u32 *)devp->regs + 192);  return count;
}

static ssize_t register193_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register193, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register193_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register193 = tempValue;
  iowrite32(devp->register193,(u32 *)devp->regs + 193);  return count;
}

static ssize_t register194_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register194, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register194_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register194 = tempValue;
  iowrite32(devp->register194,(u32 *)devp->regs + 194);  return count;
}

static ssize_t register195_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register195, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register195_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register195 = tempValue;
  iowrite32(devp->register195,(u32 *)devp->regs + 195);  return count;
}

static ssize_t register196_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register196, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register196_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register196 = tempValue;
  iowrite32(devp->register196,(u32 *)devp->regs + 196);  return count;
}

static ssize_t register197_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register197, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register197_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register197 = tempValue;
  iowrite32(devp->register197,(u32 *)devp->regs + 197);  return count;
}

static ssize_t register198_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register198, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register198_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register198 = tempValue;
  iowrite32(devp->register198,(u32 *)devp->regs + 198);  return count;
}

static ssize_t register199_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register199, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register199_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register199 = tempValue;
  iowrite32(devp->register199,(u32 *)devp->regs + 199);  return count;
}

static ssize_t register200_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register200, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register200_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register200 = tempValue;
  iowrite32(devp->register200,(u32 *)devp->regs + 200);  return count;
}

static ssize_t register201_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register201, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register201_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register201 = tempValue;
  iowrite32(devp->register201,(u32 *)devp->regs + 201);  return count;
}

static ssize_t register202_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register202, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register202_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register202 = tempValue;
  iowrite32(devp->register202,(u32 *)devp->regs + 202);  return count;
}

static ssize_t register203_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register203, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register203_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register203 = tempValue;
  iowrite32(devp->register203,(u32 *)devp->regs + 203);  return count;
}

static ssize_t register204_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register204, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register204_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register204 = tempValue;
  iowrite32(devp->register204,(u32 *)devp->regs + 204);  return count;
}

static ssize_t register205_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register205, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register205_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register205 = tempValue;
  iowrite32(devp->register205,(u32 *)devp->regs + 205);  return count;
}

static ssize_t register206_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register206, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register206_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register206 = tempValue;
  iowrite32(devp->register206,(u32 *)devp->regs + 206);  return count;
}

static ssize_t register207_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register207, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register207_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register207 = tempValue;
  iowrite32(devp->register207,(u32 *)devp->regs + 207);  return count;
}

static ssize_t register208_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register208, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register208_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register208 = tempValue;
  iowrite32(devp->register208,(u32 *)devp->regs + 208);  return count;
}

static ssize_t register209_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register209, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register209_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register209 = tempValue;
  iowrite32(devp->register209,(u32 *)devp->regs + 209);  return count;
}

static ssize_t register210_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register210, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register210_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register210 = tempValue;
  iowrite32(devp->register210,(u32 *)devp->regs + 210);  return count;
}

static ssize_t register211_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register211, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register211_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register211 = tempValue;
  iowrite32(devp->register211,(u32 *)devp->regs + 211);  return count;
}

static ssize_t register212_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register212, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register212_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register212 = tempValue;
  iowrite32(devp->register212,(u32 *)devp->regs + 212);  return count;
}

static ssize_t register213_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register213, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register213_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register213 = tempValue;
  iowrite32(devp->register213,(u32 *)devp->regs + 213);  return count;
}

static ssize_t register214_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register214, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register214_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register214 = tempValue;
  iowrite32(devp->register214,(u32 *)devp->regs + 214);  return count;
}

static ssize_t register215_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register215, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register215_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register215 = tempValue;
  iowrite32(devp->register215,(u32 *)devp->regs + 215);  return count;
}

static ssize_t register216_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register216, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register216_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register216 = tempValue;
  iowrite32(devp->register216,(u32 *)devp->regs + 216);  return count;
}

static ssize_t register217_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register217, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register217_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register217 = tempValue;
  iowrite32(devp->register217,(u32 *)devp->regs + 217);  return count;
}

static ssize_t register218_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register218, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register218_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register218 = tempValue;
  iowrite32(devp->register218,(u32 *)devp->regs + 218);  return count;
}

static ssize_t register219_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register219, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register219_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register219 = tempValue;
  iowrite32(devp->register219,(u32 *)devp->regs + 219);  return count;
}

static ssize_t register220_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register220, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register220_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register220 = tempValue;
  iowrite32(devp->register220,(u32 *)devp->regs + 220);  return count;
}

static ssize_t register221_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register221, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register221_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register221 = tempValue;
  iowrite32(devp->register221,(u32 *)devp->regs + 221);  return count;
}

static ssize_t register222_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register222, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register222_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register222 = tempValue;
  iowrite32(devp->register222,(u32 *)devp->regs + 222);  return count;
}

static ssize_t register223_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register223, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register223_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register223 = tempValue;
  iowrite32(devp->register223,(u32 *)devp->regs + 223);  return count;
}

static ssize_t register224_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register224, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register224_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register224 = tempValue;
  iowrite32(devp->register224,(u32 *)devp->regs + 224);  return count;
}

static ssize_t register225_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register225, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register225_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register225 = tempValue;
  iowrite32(devp->register225,(u32 *)devp->regs + 225);  return count;
}

static ssize_t register226_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register226, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register226_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register226 = tempValue;
  iowrite32(devp->register226,(u32 *)devp->regs + 226);  return count;
}

static ssize_t register227_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register227, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register227_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register227 = tempValue;
  iowrite32(devp->register227,(u32 *)devp->regs + 227);  return count;
}

static ssize_t register228_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register228, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register228_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register228 = tempValue;
  iowrite32(devp->register228,(u32 *)devp->regs + 228);  return count;
}

static ssize_t register229_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register229, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register229_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register229 = tempValue;
  iowrite32(devp->register229,(u32 *)devp->regs + 229);  return count;
}

static ssize_t register230_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register230, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register230_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register230 = tempValue;
  iowrite32(devp->register230,(u32 *)devp->regs + 230);  return count;
}

static ssize_t register231_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register231, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register231_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register231 = tempValue;
  iowrite32(devp->register231,(u32 *)devp->regs + 231);  return count;
}

static ssize_t register232_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register232, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register232_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register232 = tempValue;
  iowrite32(devp->register232,(u32 *)devp->regs + 232);  return count;
}

static ssize_t register233_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register233, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register233_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register233 = tempValue;
  iowrite32(devp->register233,(u32 *)devp->regs + 233);  return count;
}

static ssize_t register234_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register234, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register234_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register234 = tempValue;
  iowrite32(devp->register234,(u32 *)devp->regs + 234);  return count;
}

static ssize_t register235_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register235, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register235_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register235 = tempValue;
  iowrite32(devp->register235,(u32 *)devp->regs + 235);  return count;
}

static ssize_t register236_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register236, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register236_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register236 = tempValue;
  iowrite32(devp->register236,(u32 *)devp->regs + 236);  return count;
}

static ssize_t register237_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register237, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register237_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register237 = tempValue;
  iowrite32(devp->register237,(u32 *)devp->regs + 237);  return count;
}

static ssize_t register238_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register238, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register238_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register238 = tempValue;
  iowrite32(devp->register238,(u32 *)devp->regs + 238);  return count;
}

static ssize_t register239_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register239, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register239_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register239 = tempValue;
  iowrite32(devp->register239,(u32 *)devp->regs + 239);  return count;
}

static ssize_t register240_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register240, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register240_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register240 = tempValue;
  iowrite32(devp->register240,(u32 *)devp->regs + 240);  return count;
}

static ssize_t register241_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register241, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register241_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register241 = tempValue;
  iowrite32(devp->register241,(u32 *)devp->regs + 241);  return count;
}

static ssize_t register242_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register242, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register242_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register242 = tempValue;
  iowrite32(devp->register242,(u32 *)devp->regs + 242);  return count;
}

static ssize_t register243_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register243, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register243_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register243 = tempValue;
  iowrite32(devp->register243,(u32 *)devp->regs + 243);  return count;
}

static ssize_t register244_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register244, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register244_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register244 = tempValue;
  iowrite32(devp->register244,(u32 *)devp->regs + 244);  return count;
}

static ssize_t register245_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register245, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register245_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register245 = tempValue;
  iowrite32(devp->register245,(u32 *)devp->regs + 245);  return count;
}

static ssize_t register246_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register246, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register246_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register246 = tempValue;
  iowrite32(devp->register246,(u32 *)devp->regs + 246);  return count;
}

static ssize_t register247_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register247, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register247_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register247 = tempValue;
  iowrite32(devp->register247,(u32 *)devp->regs + 247);  return count;
}

static ssize_t register248_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register248, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register248_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register248 = tempValue;
  iowrite32(devp->register248,(u32 *)devp->regs + 248);  return count;
}

static ssize_t register249_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register249, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register249_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register249 = tempValue;
  iowrite32(devp->register249,(u32 *)devp->regs + 249);  return count;
}

static ssize_t register250_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register250, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register250_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register250 = tempValue;
  iowrite32(devp->register250,(u32 *)devp->regs + 250);  return count;
}

static ssize_t register251_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register251, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register251_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register251 = tempValue;
  iowrite32(devp->register251,(u32 *)devp->regs + 251);  return count;
}

static ssize_t register252_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register252, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register252_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register252 = tempValue;
  iowrite32(devp->register252,(u32 *)devp->regs + 252);  return count;
}

static ssize_t register253_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register253, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register253_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register253 = tempValue;
  iowrite32(devp->register253,(u32 *)devp->regs + 253);  return count;
}

static ssize_t register254_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register254, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register254_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register254 = tempValue;
  iowrite32(devp->register254,(u32 *)devp->regs + 254);  return count;
}

static ssize_t register255_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register255, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register255_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register255 = tempValue;
  iowrite32(devp->register255,(u32 *)devp->regs + 255);  return count;
}

static ssize_t register256_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register256, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register256_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register256 = tempValue;
  iowrite32(devp->register256,(u32 *)devp->regs + 256);  return count;
}

static ssize_t register257_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register257, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register257_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register257 = tempValue;
  iowrite32(devp->register257,(u32 *)devp->regs + 257);  return count;
}

static ssize_t register258_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register258, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register258_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register258 = tempValue;
  iowrite32(devp->register258,(u32 *)devp->regs + 258);  return count;
}

static ssize_t register259_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register259, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register259_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register259 = tempValue;
  iowrite32(devp->register259,(u32 *)devp->regs + 259);  return count;
}

static ssize_t register260_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register260, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register260_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register260 = tempValue;
  iowrite32(devp->register260,(u32 *)devp->regs + 260);  return count;
}

static ssize_t register261_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register261, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register261_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register261 = tempValue;
  iowrite32(devp->register261,(u32 *)devp->regs + 261);  return count;
}

static ssize_t register262_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register262, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register262_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register262 = tempValue;
  iowrite32(devp->register262,(u32 *)devp->regs + 262);  return count;
}

static ssize_t register263_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register263, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register263_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register263 = tempValue;
  iowrite32(devp->register263,(u32 *)devp->regs + 263);  return count;
}

static ssize_t register264_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register264, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register264_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register264 = tempValue;
  iowrite32(devp->register264,(u32 *)devp->regs + 264);  return count;
}

static ssize_t register265_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register265, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register265_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register265 = tempValue;
  iowrite32(devp->register265,(u32 *)devp->regs + 265);  return count;
}

static ssize_t register266_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register266, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register266_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register266 = tempValue;
  iowrite32(devp->register266,(u32 *)devp->regs + 266);  return count;
}

static ssize_t register267_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register267, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register267_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register267 = tempValue;
  iowrite32(devp->register267,(u32 *)devp->regs + 267);  return count;
}

static ssize_t register268_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register268, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register268_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register268 = tempValue;
  iowrite32(devp->register268,(u32 *)devp->regs + 268);  return count;
}

static ssize_t register269_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register269, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register269_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register269 = tempValue;
  iowrite32(devp->register269,(u32 *)devp->regs + 269);  return count;
}

static ssize_t register270_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register270, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register270_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register270 = tempValue;
  iowrite32(devp->register270,(u32 *)devp->regs + 270);  return count;
}

static ssize_t register271_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register271, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register271_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register271 = tempValue;
  iowrite32(devp->register271,(u32 *)devp->regs + 271);  return count;
}

static ssize_t register272_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register272, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register272_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register272 = tempValue;
  iowrite32(devp->register272,(u32 *)devp->regs + 272);  return count;
}

static ssize_t register273_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register273, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register273_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register273 = tempValue;
  iowrite32(devp->register273,(u32 *)devp->regs + 273);  return count;
}

static ssize_t register274_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register274, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register274_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register274 = tempValue;
  iowrite32(devp->register274,(u32 *)devp->regs + 274);  return count;
}

static ssize_t register275_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register275, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register275_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register275 = tempValue;
  iowrite32(devp->register275,(u32 *)devp->regs + 275);  return count;
}

static ssize_t register276_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register276, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register276_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register276 = tempValue;
  iowrite32(devp->register276,(u32 *)devp->regs + 276);  return count;
}

static ssize_t register277_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register277, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register277_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register277 = tempValue;
  iowrite32(devp->register277,(u32 *)devp->regs + 277);  return count;
}

static ssize_t register278_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register278, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register278_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register278 = tempValue;
  iowrite32(devp->register278,(u32 *)devp->regs + 278);  return count;
}

static ssize_t register279_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register279, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register279_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register279 = tempValue;
  iowrite32(devp->register279,(u32 *)devp->regs + 279);  return count;
}

static ssize_t register280_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register280, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register280_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register280 = tempValue;
  iowrite32(devp->register280,(u32 *)devp->regs + 280);  return count;
}

static ssize_t register281_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register281, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register281_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register281 = tempValue;
  iowrite32(devp->register281,(u32 *)devp->regs + 281);  return count;
}

static ssize_t register282_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register282, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register282_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register282 = tempValue;
  iowrite32(devp->register282,(u32 *)devp->regs + 282);  return count;
}

static ssize_t register283_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register283, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register283_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register283 = tempValue;
  iowrite32(devp->register283,(u32 *)devp->regs + 283);  return count;
}

static ssize_t register284_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register284, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register284_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register284 = tempValue;
  iowrite32(devp->register284,(u32 *)devp->regs + 284);  return count;
}

static ssize_t register285_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register285, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register285_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register285 = tempValue;
  iowrite32(devp->register285,(u32 *)devp->regs + 285);  return count;
}

static ssize_t register286_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register286, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register286_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register286 = tempValue;
  iowrite32(devp->register286,(u32 *)devp->regs + 286);  return count;
}

static ssize_t register287_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register287, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register287_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register287 = tempValue;
  iowrite32(devp->register287,(u32 *)devp->regs + 287);  return count;
}

static ssize_t register288_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register288, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register288_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register288 = tempValue;
  iowrite32(devp->register288,(u32 *)devp->regs + 288);  return count;
}

static ssize_t register289_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register289, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register289_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register289 = tempValue;
  iowrite32(devp->register289,(u32 *)devp->regs + 289);  return count;
}

static ssize_t register290_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register290, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register290_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register290 = tempValue;
  iowrite32(devp->register290,(u32 *)devp->regs + 290);  return count;
}

static ssize_t register291_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register291, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register291_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register291 = tempValue;
  iowrite32(devp->register291,(u32 *)devp->regs + 291);  return count;
}

static ssize_t register292_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register292, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register292_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register292 = tempValue;
  iowrite32(devp->register292,(u32 *)devp->regs + 292);  return count;
}

static ssize_t register293_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register293, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register293_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register293 = tempValue;
  iowrite32(devp->register293,(u32 *)devp->regs + 293);  return count;
}

static ssize_t register294_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register294, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register294_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register294 = tempValue;
  iowrite32(devp->register294,(u32 *)devp->regs + 294);  return count;
}

static ssize_t register295_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register295, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register295_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register295 = tempValue;
  iowrite32(devp->register295,(u32 *)devp->regs + 295);  return count;
}

static ssize_t register296_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register296, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register296_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register296 = tempValue;
  iowrite32(devp->register296,(u32 *)devp->regs + 296);  return count;
}

static ssize_t register297_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register297, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register297_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register297 = tempValue;
  iowrite32(devp->register297,(u32 *)devp->regs + 297);  return count;
}

static ssize_t register298_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register298, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register298_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register298 = tempValue;
  iowrite32(devp->register298,(u32 *)devp->regs + 298);  return count;
}

static ssize_t register299_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register299, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register299_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register299 = tempValue;
  iowrite32(devp->register299,(u32 *)devp->regs + 299);  return count;
}

static ssize_t register300_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register300, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register300_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register300 = tempValue;
  iowrite32(devp->register300,(u32 *)devp->regs + 300);  return count;
}

static ssize_t register301_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register301, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register301_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register301 = tempValue;
  iowrite32(devp->register301,(u32 *)devp->regs + 301);  return count;
}

static ssize_t register302_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register302, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register302_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register302 = tempValue;
  iowrite32(devp->register302,(u32 *)devp->regs + 302);  return count;
}

static ssize_t register303_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register303, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register303_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register303 = tempValue;
  iowrite32(devp->register303,(u32 *)devp->regs + 303);  return count;
}

static ssize_t register304_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register304, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register304_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register304 = tempValue;
  iowrite32(devp->register304,(u32 *)devp->regs + 304);  return count;
}

static ssize_t register305_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register305, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register305_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register305 = tempValue;
  iowrite32(devp->register305,(u32 *)devp->regs + 305);  return count;
}

static ssize_t register306_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register306, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register306_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register306 = tempValue;
  iowrite32(devp->register306,(u32 *)devp->regs + 306);  return count;
}

static ssize_t register307_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register307, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register307_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register307 = tempValue;
  iowrite32(devp->register307,(u32 *)devp->regs + 307);  return count;
}

static ssize_t register308_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register308, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register308_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register308 = tempValue;
  iowrite32(devp->register308,(u32 *)devp->regs + 308);  return count;
}

static ssize_t register309_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register309, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register309_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register309 = tempValue;
  iowrite32(devp->register309,(u32 *)devp->regs + 309);  return count;
}

static ssize_t register310_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register310, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register310_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register310 = tempValue;
  iowrite32(devp->register310,(u32 *)devp->regs + 310);  return count;
}

static ssize_t register311_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register311, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register311_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register311 = tempValue;
  iowrite32(devp->register311,(u32 *)devp->regs + 311);  return count;
}

static ssize_t register312_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register312, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register312_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register312 = tempValue;
  iowrite32(devp->register312,(u32 *)devp->regs + 312);  return count;
}

static ssize_t register313_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register313, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register313_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register313 = tempValue;
  iowrite32(devp->register313,(u32 *)devp->regs + 313);  return count;
}

static ssize_t register314_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register314, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register314_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register314 = tempValue;
  iowrite32(devp->register314,(u32 *)devp->regs + 314);  return count;
}

static ssize_t register315_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register315, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register315_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register315 = tempValue;
  iowrite32(devp->register315,(u32 *)devp->regs + 315);  return count;
}

static ssize_t register316_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register316, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register316_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register316 = tempValue;
  iowrite32(devp->register316,(u32 *)devp->regs + 316);  return count;
}

static ssize_t register317_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register317, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register317_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register317 = tempValue;
  iowrite32(devp->register317,(u32 *)devp->regs + 317);  return count;
}

static ssize_t register318_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register318, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register318_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register318 = tempValue;
  iowrite32(devp->register318,(u32 *)devp->regs + 318);  return count;
}

static ssize_t register319_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register319, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register319_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register319 = tempValue;
  iowrite32(devp->register319,(u32 *)devp->regs + 319);  return count;
}

static ssize_t register320_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register320, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register320_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register320 = tempValue;
  iowrite32(devp->register320,(u32 *)devp->regs + 320);  return count;
}

static ssize_t register321_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register321, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register321_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register321 = tempValue;
  iowrite32(devp->register321,(u32 *)devp->regs + 321);  return count;
}

static ssize_t register322_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register322, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register322_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register322 = tempValue;
  iowrite32(devp->register322,(u32 *)devp->regs + 322);  return count;
}

static ssize_t register323_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register323, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register323_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register323 = tempValue;
  iowrite32(devp->register323,(u32 *)devp->regs + 323);  return count;
}

static ssize_t register324_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register324, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register324_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register324 = tempValue;
  iowrite32(devp->register324,(u32 *)devp->regs + 324);  return count;
}

static ssize_t register325_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register325, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register325_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register325 = tempValue;
  iowrite32(devp->register325,(u32 *)devp->regs + 325);  return count;
}

static ssize_t register326_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register326, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register326_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register326 = tempValue;
  iowrite32(devp->register326,(u32 *)devp->regs + 326);  return count;
}

static ssize_t register327_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register327, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register327_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register327 = tempValue;
  iowrite32(devp->register327,(u32 *)devp->regs + 327);  return count;
}

static ssize_t register328_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register328, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register328_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register328 = tempValue;
  iowrite32(devp->register328,(u32 *)devp->regs + 328);  return count;
}

static ssize_t register329_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register329, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register329_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register329 = tempValue;
  iowrite32(devp->register329,(u32 *)devp->regs + 329);  return count;
}

static ssize_t register330_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register330, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register330_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register330 = tempValue;
  iowrite32(devp->register330,(u32 *)devp->regs + 330);  return count;
}

static ssize_t register331_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register331, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register331_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register331 = tempValue;
  iowrite32(devp->register331,(u32 *)devp->regs + 331);  return count;
}

static ssize_t register332_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register332, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register332_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register332 = tempValue;
  iowrite32(devp->register332,(u32 *)devp->regs + 332);  return count;
}

static ssize_t register333_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register333, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register333_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register333 = tempValue;
  iowrite32(devp->register333,(u32 *)devp->regs + 333);  return count;
}

static ssize_t register334_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register334, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register334_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register334 = tempValue;
  iowrite32(devp->register334,(u32 *)devp->regs + 334);  return count;
}

static ssize_t register335_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register335, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register335_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register335 = tempValue;
  iowrite32(devp->register335,(u32 *)devp->regs + 335);  return count;
}

static ssize_t register336_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register336, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register336_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register336 = tempValue;
  iowrite32(devp->register336,(u32 *)devp->regs + 336);  return count;
}

static ssize_t register337_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register337, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register337_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register337 = tempValue;
  iowrite32(devp->register337,(u32 *)devp->regs + 337);  return count;
}

static ssize_t register338_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register338, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register338_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register338 = tempValue;
  iowrite32(devp->register338,(u32 *)devp->regs + 338);  return count;
}

static ssize_t register339_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register339, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register339_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register339 = tempValue;
  iowrite32(devp->register339,(u32 *)devp->regs + 339);  return count;
}

static ssize_t register340_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register340, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register340_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register340 = tempValue;
  iowrite32(devp->register340,(u32 *)devp->regs + 340);  return count;
}

static ssize_t register341_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register341, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register341_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register341 = tempValue;
  iowrite32(devp->register341,(u32 *)devp->regs + 341);  return count;
}

static ssize_t register342_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register342, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register342_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register342 = tempValue;
  iowrite32(devp->register342,(u32 *)devp->regs + 342);  return count;
}

static ssize_t register343_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register343, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register343_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register343 = tempValue;
  iowrite32(devp->register343,(u32 *)devp->regs + 343);  return count;
}

static ssize_t register344_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register344, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register344_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register344 = tempValue;
  iowrite32(devp->register344,(u32 *)devp->regs + 344);  return count;
}

static ssize_t register345_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register345, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register345_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register345 = tempValue;
  iowrite32(devp->register345,(u32 *)devp->regs + 345);  return count;
}

static ssize_t register346_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register346, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register346_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register346 = tempValue;
  iowrite32(devp->register346,(u32 *)devp->regs + 346);  return count;
}

static ssize_t register347_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register347, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register347_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register347 = tempValue;
  iowrite32(devp->register347,(u32 *)devp->regs + 347);  return count;
}

static ssize_t register348_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register348, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register348_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register348 = tempValue;
  iowrite32(devp->register348,(u32 *)devp->regs + 348);  return count;
}

static ssize_t register349_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register349, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register349_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register349 = tempValue;
  iowrite32(devp->register349,(u32 *)devp->regs + 349);  return count;
}

static ssize_t register350_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register350, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register350_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register350 = tempValue;
  iowrite32(devp->register350,(u32 *)devp->regs + 350);  return count;
}

static ssize_t register351_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register351, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register351_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register351 = tempValue;
  iowrite32(devp->register351,(u32 *)devp->regs + 351);  return count;
}

static ssize_t register352_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register352, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register352_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register352 = tempValue;
  iowrite32(devp->register352,(u32 *)devp->regs + 352);  return count;
}

static ssize_t register353_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register353, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register353_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register353 = tempValue;
  iowrite32(devp->register353,(u32 *)devp->regs + 353);  return count;
}

static ssize_t register354_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register354, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register354_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register354 = tempValue;
  iowrite32(devp->register354,(u32 *)devp->regs + 354);  return count;
}

static ssize_t register355_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register355, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register355_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register355 = tempValue;
  iowrite32(devp->register355,(u32 *)devp->regs + 355);  return count;
}

static ssize_t register356_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register356, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register356_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register356 = tempValue;
  iowrite32(devp->register356,(u32 *)devp->regs + 356);  return count;
}

static ssize_t register357_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register357, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register357_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register357 = tempValue;
  iowrite32(devp->register357,(u32 *)devp->regs + 357);  return count;
}

static ssize_t register358_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register358, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register358_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register358 = tempValue;
  iowrite32(devp->register358,(u32 *)devp->regs + 358);  return count;
}

static ssize_t register359_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register359, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register359_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register359 = tempValue;
  iowrite32(devp->register359,(u32 *)devp->regs + 359);  return count;
}

static ssize_t register360_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register360, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register360_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register360 = tempValue;
  iowrite32(devp->register360,(u32 *)devp->regs + 360);  return count;
}

static ssize_t register361_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register361, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register361_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register361 = tempValue;
  iowrite32(devp->register361,(u32 *)devp->regs + 361);  return count;
}

static ssize_t register362_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register362, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register362_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register362 = tempValue;
  iowrite32(devp->register362,(u32 *)devp->regs + 362);  return count;
}

static ssize_t register363_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register363, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register363_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register363 = tempValue;
  iowrite32(devp->register363,(u32 *)devp->regs + 363);  return count;
}

static ssize_t register364_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register364, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register364_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register364 = tempValue;
  iowrite32(devp->register364,(u32 *)devp->regs + 364);  return count;
}

static ssize_t register365_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register365, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register365_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register365 = tempValue;
  iowrite32(devp->register365,(u32 *)devp->regs + 365);  return count;
}

static ssize_t register366_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register366, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register366_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register366 = tempValue;
  iowrite32(devp->register366,(u32 *)devp->regs + 366);  return count;
}

static ssize_t register367_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register367, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register367_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register367 = tempValue;
  iowrite32(devp->register367,(u32 *)devp->regs + 367);  return count;
}

static ssize_t register368_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register368, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register368_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register368 = tempValue;
  iowrite32(devp->register368,(u32 *)devp->regs + 368);  return count;
}

static ssize_t register369_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register369, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register369_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register369 = tempValue;
  iowrite32(devp->register369,(u32 *)devp->regs + 369);  return count;
}

static ssize_t register370_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register370, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register370_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register370 = tempValue;
  iowrite32(devp->register370,(u32 *)devp->regs + 370);  return count;
}

static ssize_t register371_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register371, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register371_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register371 = tempValue;
  iowrite32(devp->register371,(u32 *)devp->regs + 371);  return count;
}

static ssize_t register372_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register372, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register372_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register372 = tempValue;
  iowrite32(devp->register372,(u32 *)devp->regs + 372);  return count;
}

static ssize_t register373_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register373, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register373_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register373 = tempValue;
  iowrite32(devp->register373,(u32 *)devp->regs + 373);  return count;
}

static ssize_t register374_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register374, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register374_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register374 = tempValue;
  iowrite32(devp->register374,(u32 *)devp->regs + 374);  return count;
}

static ssize_t register375_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register375, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register375_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register375 = tempValue;
  iowrite32(devp->register375,(u32 *)devp->regs + 375);  return count;
}

static ssize_t register376_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register376, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register376_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register376 = tempValue;
  iowrite32(devp->register376,(u32 *)devp->regs + 376);  return count;
}

static ssize_t register377_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register377, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register377_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register377 = tempValue;
  iowrite32(devp->register377,(u32 *)devp->regs + 377);  return count;
}

static ssize_t register378_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register378, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register378_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register378 = tempValue;
  iowrite32(devp->register378,(u32 *)devp->regs + 378);  return count;
}

static ssize_t register379_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register379, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register379_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register379 = tempValue;
  iowrite32(devp->register379,(u32 *)devp->regs + 379);  return count;
}

static ssize_t register380_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register380, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register380_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register380 = tempValue;
  iowrite32(devp->register380,(u32 *)devp->regs + 380);  return count;
}

static ssize_t register381_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register381, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register381_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register381 = tempValue;
  iowrite32(devp->register381,(u32 *)devp->regs + 381);  return count;
}

static ssize_t register382_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register382, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register382_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register382 = tempValue;
  iowrite32(devp->register382,(u32 *)devp->regs + 382);  return count;
}

static ssize_t register383_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register383, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register383_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register383 = tempValue;
  iowrite32(devp->register383,(u32 *)devp->regs + 383);  return count;
}

static ssize_t register384_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register384, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register384_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register384 = tempValue;
  iowrite32(devp->register384,(u32 *)devp->regs + 384);  return count;
}

static ssize_t register385_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register385, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register385_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register385 = tempValue;
  iowrite32(devp->register385,(u32 *)devp->regs + 385);  return count;
}

static ssize_t register386_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register386, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register386_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register386 = tempValue;
  iowrite32(devp->register386,(u32 *)devp->regs + 386);  return count;
}

static ssize_t register387_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register387, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register387_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register387 = tempValue;
  iowrite32(devp->register387,(u32 *)devp->regs + 387);  return count;
}

static ssize_t register388_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register388, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register388_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register388 = tempValue;
  iowrite32(devp->register388,(u32 *)devp->regs + 388);  return count;
}

static ssize_t register389_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register389, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register389_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register389 = tempValue;
  iowrite32(devp->register389,(u32 *)devp->regs + 389);  return count;
}

static ssize_t register390_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register390, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register390_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register390 = tempValue;
  iowrite32(devp->register390,(u32 *)devp->regs + 390);  return count;
}

static ssize_t register391_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register391, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register391_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register391 = tempValue;
  iowrite32(devp->register391,(u32 *)devp->regs + 391);  return count;
}

static ssize_t register392_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register392, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register392_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register392 = tempValue;
  iowrite32(devp->register392,(u32 *)devp->regs + 392);  return count;
}

static ssize_t register393_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register393, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register393_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register393 = tempValue;
  iowrite32(devp->register393,(u32 *)devp->regs + 393);  return count;
}

static ssize_t register394_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register394, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register394_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register394 = tempValue;
  iowrite32(devp->register394,(u32 *)devp->regs + 394);  return count;
}

static ssize_t register395_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register395, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register395_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register395 = tempValue;
  iowrite32(devp->register395,(u32 *)devp->regs + 395);  return count;
}

static ssize_t register396_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register396, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register396_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register396 = tempValue;
  iowrite32(devp->register396,(u32 *)devp->regs + 396);  return count;
}

static ssize_t register397_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register397, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register397_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register397 = tempValue;
  iowrite32(devp->register397,(u32 *)devp->regs + 397);  return count;
}

static ssize_t register398_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register398, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register398_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register398 = tempValue;
  iowrite32(devp->register398,(u32 *)devp->regs + 398);  return count;
}

static ssize_t register399_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register399, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register399_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register399 = tempValue;
  iowrite32(devp->register399,(u32 *)devp->regs + 399);  return count;
}

static ssize_t register400_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register400, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register400_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register400 = tempValue;
  iowrite32(devp->register400,(u32 *)devp->regs + 400);  return count;
}

static ssize_t register401_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register401, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register401_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register401 = tempValue;
  iowrite32(devp->register401,(u32 *)devp->regs + 401);  return count;
}

static ssize_t register402_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register402, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register402_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register402 = tempValue;
  iowrite32(devp->register402,(u32 *)devp->regs + 402);  return count;
}

static ssize_t register403_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register403, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register403_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register403 = tempValue;
  iowrite32(devp->register403,(u32 *)devp->regs + 403);  return count;
}

static ssize_t register404_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register404, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register404_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register404 = tempValue;
  iowrite32(devp->register404,(u32 *)devp->regs + 404);  return count;
}

static ssize_t register405_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register405, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register405_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register405 = tempValue;
  iowrite32(devp->register405,(u32 *)devp->regs + 405);  return count;
}

static ssize_t register406_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register406, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register406_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register406 = tempValue;
  iowrite32(devp->register406,(u32 *)devp->regs + 406);  return count;
}

static ssize_t register407_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register407, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register407_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register407 = tempValue;
  iowrite32(devp->register407,(u32 *)devp->regs + 407);  return count;
}

static ssize_t register408_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register408, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register408_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register408 = tempValue;
  iowrite32(devp->register408,(u32 *)devp->regs + 408);  return count;
}

static ssize_t register409_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register409, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register409_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register409 = tempValue;
  iowrite32(devp->register409,(u32 *)devp->regs + 409);  return count;
}

static ssize_t register410_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register410, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register410_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register410 = tempValue;
  iowrite32(devp->register410,(u32 *)devp->regs + 410);  return count;
}

static ssize_t register411_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register411, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register411_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register411 = tempValue;
  iowrite32(devp->register411,(u32 *)devp->regs + 411);  return count;
}

static ssize_t register412_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register412, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register412_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register412 = tempValue;
  iowrite32(devp->register412,(u32 *)devp->regs + 412);  return count;
}

static ssize_t register413_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register413, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register413_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register413 = tempValue;
  iowrite32(devp->register413,(u32 *)devp->regs + 413);  return count;
}

static ssize_t register414_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register414, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register414_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register414 = tempValue;
  iowrite32(devp->register414,(u32 *)devp->regs + 414);  return count;
}

static ssize_t register415_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register415, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register415_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register415 = tempValue;
  iowrite32(devp->register415,(u32 *)devp->regs + 415);  return count;
}

static ssize_t register416_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register416, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register416_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register416 = tempValue;
  iowrite32(devp->register416,(u32 *)devp->regs + 416);  return count;
}

static ssize_t register417_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register417, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register417_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register417 = tempValue;
  iowrite32(devp->register417,(u32 *)devp->regs + 417);  return count;
}

static ssize_t register418_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register418, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register418_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register418 = tempValue;
  iowrite32(devp->register418,(u32 *)devp->regs + 418);  return count;
}

static ssize_t register419_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register419, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register419_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register419 = tempValue;
  iowrite32(devp->register419,(u32 *)devp->regs + 419);  return count;
}

static ssize_t register420_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register420, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register420_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register420 = tempValue;
  iowrite32(devp->register420,(u32 *)devp->regs + 420);  return count;
}

static ssize_t register421_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register421, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register421_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register421 = tempValue;
  iowrite32(devp->register421,(u32 *)devp->regs + 421);  return count;
}

static ssize_t register422_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register422, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register422_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register422 = tempValue;
  iowrite32(devp->register422,(u32 *)devp->regs + 422);  return count;
}

static ssize_t register423_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register423, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register423_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register423 = tempValue;
  iowrite32(devp->register423,(u32 *)devp->regs + 423);  return count;
}

static ssize_t register424_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register424, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register424_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register424 = tempValue;
  iowrite32(devp->register424,(u32 *)devp->regs + 424);  return count;
}

static ssize_t register425_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register425, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register425_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register425 = tempValue;
  iowrite32(devp->register425,(u32 *)devp->regs + 425);  return count;
}

static ssize_t register426_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register426, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register426_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register426 = tempValue;
  iowrite32(devp->register426,(u32 *)devp->regs + 426);  return count;
}

static ssize_t register427_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register427, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register427_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register427 = tempValue;
  iowrite32(devp->register427,(u32 *)devp->regs + 427);  return count;
}

static ssize_t register428_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register428, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register428_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register428 = tempValue;
  iowrite32(devp->register428,(u32 *)devp->regs + 428);  return count;
}

static ssize_t register429_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register429, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register429_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register429 = tempValue;
  iowrite32(devp->register429,(u32 *)devp->regs + 429);  return count;
}

static ssize_t register430_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register430, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register430_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register430 = tempValue;
  iowrite32(devp->register430,(u32 *)devp->regs + 430);  return count;
}

static ssize_t register431_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register431, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register431_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register431 = tempValue;
  iowrite32(devp->register431,(u32 *)devp->regs + 431);  return count;
}

static ssize_t register432_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register432, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register432_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register432 = tempValue;
  iowrite32(devp->register432,(u32 *)devp->regs + 432);  return count;
}

static ssize_t register433_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register433, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register433_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register433 = tempValue;
  iowrite32(devp->register433,(u32 *)devp->regs + 433);  return count;
}

static ssize_t register434_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register434, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register434_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register434 = tempValue;
  iowrite32(devp->register434,(u32 *)devp->regs + 434);  return count;
}

static ssize_t register435_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register435, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register435_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register435 = tempValue;
  iowrite32(devp->register435,(u32 *)devp->regs + 435);  return count;
}

static ssize_t register436_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register436, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register436_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register436 = tempValue;
  iowrite32(devp->register436,(u32 *)devp->regs + 436);  return count;
}

static ssize_t register437_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register437, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register437_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register437 = tempValue;
  iowrite32(devp->register437,(u32 *)devp->regs + 437);  return count;
}

static ssize_t register438_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register438, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register438_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register438 = tempValue;
  iowrite32(devp->register438,(u32 *)devp->regs + 438);  return count;
}

static ssize_t register439_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register439, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register439_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register439 = tempValue;
  iowrite32(devp->register439,(u32 *)devp->regs + 439);  return count;
}

static ssize_t register440_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register440, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register440_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register440 = tempValue;
  iowrite32(devp->register440,(u32 *)devp->regs + 440);  return count;
}

static ssize_t register441_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register441, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register441_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register441 = tempValue;
  iowrite32(devp->register441,(u32 *)devp->regs + 441);  return count;
}

static ssize_t register442_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register442, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register442_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register442 = tempValue;
  iowrite32(devp->register442,(u32 *)devp->regs + 442);  return count;
}

static ssize_t register443_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register443, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register443_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register443 = tempValue;
  iowrite32(devp->register443,(u32 *)devp->regs + 443);  return count;
}

static ssize_t register444_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register444, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register444_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register444 = tempValue;
  iowrite32(devp->register444,(u32 *)devp->regs + 444);  return count;
}

static ssize_t register445_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register445, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register445_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register445 = tempValue;
  iowrite32(devp->register445,(u32 *)devp->regs + 445);  return count;
}

static ssize_t register446_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register446, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register446_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register446 = tempValue;
  iowrite32(devp->register446,(u32 *)devp->regs + 446);  return count;
}

static ssize_t register447_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register447, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register447_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register447 = tempValue;
  iowrite32(devp->register447,(u32 *)devp->regs + 447);  return count;
}

static ssize_t register448_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register448, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register448_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register448 = tempValue;
  iowrite32(devp->register448,(u32 *)devp->regs + 448);  return count;
}

static ssize_t register449_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register449, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register449_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register449 = tempValue;
  iowrite32(devp->register449,(u32 *)devp->regs + 449);  return count;
}

static ssize_t register450_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register450, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register450_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register450 = tempValue;
  iowrite32(devp->register450,(u32 *)devp->regs + 450);  return count;
}

static ssize_t register451_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register451, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register451_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register451 = tempValue;
  iowrite32(devp->register451,(u32 *)devp->regs + 451);  return count;
}

static ssize_t register452_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register452, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register452_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register452 = tempValue;
  iowrite32(devp->register452,(u32 *)devp->regs + 452);  return count;
}

static ssize_t register453_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register453, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register453_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register453 = tempValue;
  iowrite32(devp->register453,(u32 *)devp->regs + 453);  return count;
}

static ssize_t register454_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register454, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register454_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register454 = tempValue;
  iowrite32(devp->register454,(u32 *)devp->regs + 454);  return count;
}

static ssize_t register455_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register455, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register455_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register455 = tempValue;
  iowrite32(devp->register455,(u32 *)devp->regs + 455);  return count;
}

static ssize_t register456_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register456, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register456_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register456 = tempValue;
  iowrite32(devp->register456,(u32 *)devp->regs + 456);  return count;
}

static ssize_t register457_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register457, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register457_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register457 = tempValue;
  iowrite32(devp->register457,(u32 *)devp->regs + 457);  return count;
}

static ssize_t register458_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register458, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register458_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register458 = tempValue;
  iowrite32(devp->register458,(u32 *)devp->regs + 458);  return count;
}

static ssize_t register459_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register459, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register459_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register459 = tempValue;
  iowrite32(devp->register459,(u32 *)devp->regs + 459);  return count;
}

static ssize_t register460_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register460, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register460_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register460 = tempValue;
  iowrite32(devp->register460,(u32 *)devp->regs + 460);  return count;
}

static ssize_t register461_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register461, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register461_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register461 = tempValue;
  iowrite32(devp->register461,(u32 *)devp->regs + 461);  return count;
}

static ssize_t register462_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register462, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register462_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register462 = tempValue;
  iowrite32(devp->register462,(u32 *)devp->regs + 462);  return count;
}

static ssize_t register463_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register463, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register463_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register463 = tempValue;
  iowrite32(devp->register463,(u32 *)devp->regs + 463);  return count;
}

static ssize_t register464_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register464, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register464_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register464 = tempValue;
  iowrite32(devp->register464,(u32 *)devp->regs + 464);  return count;
}

static ssize_t register465_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register465, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register465_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register465 = tempValue;
  iowrite32(devp->register465,(u32 *)devp->regs + 465);  return count;
}

static ssize_t register466_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register466, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register466_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register466 = tempValue;
  iowrite32(devp->register466,(u32 *)devp->regs + 466);  return count;
}

static ssize_t register467_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register467, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register467_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register467 = tempValue;
  iowrite32(devp->register467,(u32 *)devp->regs + 467);  return count;
}

static ssize_t register468_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register468, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register468_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register468 = tempValue;
  iowrite32(devp->register468,(u32 *)devp->regs + 468);  return count;
}

static ssize_t register469_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register469, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register469_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register469 = tempValue;
  iowrite32(devp->register469,(u32 *)devp->regs + 469);  return count;
}

static ssize_t register470_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register470, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register470_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register470 = tempValue;
  iowrite32(devp->register470,(u32 *)devp->regs + 470);  return count;
}

static ssize_t register471_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register471, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register471_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register471 = tempValue;
  iowrite32(devp->register471,(u32 *)devp->regs + 471);  return count;
}

static ssize_t register472_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register472, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register472_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register472 = tempValue;
  iowrite32(devp->register472,(u32 *)devp->regs + 472);  return count;
}

static ssize_t register473_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register473, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register473_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register473 = tempValue;
  iowrite32(devp->register473,(u32 *)devp->regs + 473);  return count;
}

static ssize_t register474_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register474, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register474_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register474 = tempValue;
  iowrite32(devp->register474,(u32 *)devp->regs + 474);  return count;
}

static ssize_t register475_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register475, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register475_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register475 = tempValue;
  iowrite32(devp->register475,(u32 *)devp->regs + 475);  return count;
}

static ssize_t register476_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register476, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register476_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register476 = tempValue;
  iowrite32(devp->register476,(u32 *)devp->regs + 476);  return count;
}

static ssize_t register477_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register477, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register477_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register477 = tempValue;
  iowrite32(devp->register477,(u32 *)devp->regs + 477);  return count;
}

static ssize_t register478_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register478, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register478_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register478 = tempValue;
  iowrite32(devp->register478,(u32 *)devp->regs + 478);  return count;
}

static ssize_t register479_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register479, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register479_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register479 = tempValue;
  iowrite32(devp->register479,(u32 *)devp->regs + 479);  return count;
}

static ssize_t register480_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register480, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register480_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register480 = tempValue;
  iowrite32(devp->register480,(u32 *)devp->regs + 480);  return count;
}

static ssize_t register481_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register481, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register481_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register481 = tempValue;
  iowrite32(devp->register481,(u32 *)devp->regs + 481);  return count;
}

static ssize_t register482_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register482, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register482_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register482 = tempValue;
  iowrite32(devp->register482,(u32 *)devp->regs + 482);  return count;
}

static ssize_t register483_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register483, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register483_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register483 = tempValue;
  iowrite32(devp->register483,(u32 *)devp->regs + 483);  return count;
}

static ssize_t register484_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register484, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register484_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register484 = tempValue;
  iowrite32(devp->register484,(u32 *)devp->regs + 484);  return count;
}

static ssize_t register485_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register485, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register485_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register485 = tempValue;
  iowrite32(devp->register485,(u32 *)devp->regs + 485);  return count;
}

static ssize_t register486_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register486, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register486_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register486 = tempValue;
  iowrite32(devp->register486,(u32 *)devp->regs + 486);  return count;
}

static ssize_t register487_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register487, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register487_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register487 = tempValue;
  iowrite32(devp->register487,(u32 *)devp->regs + 487);  return count;
}

static ssize_t register488_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register488, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register488_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register488 = tempValue;
  iowrite32(devp->register488,(u32 *)devp->regs + 488);  return count;
}

static ssize_t register489_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register489, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register489_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register489 = tempValue;
  iowrite32(devp->register489,(u32 *)devp->regs + 489);  return count;
}

static ssize_t register490_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register490, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register490_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register490 = tempValue;
  iowrite32(devp->register490,(u32 *)devp->regs + 490);  return count;
}

static ssize_t register491_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register491, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register491_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register491 = tempValue;
  iowrite32(devp->register491,(u32 *)devp->regs + 491);  return count;
}

static ssize_t register492_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register492, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register492_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register492 = tempValue;
  iowrite32(devp->register492,(u32 *)devp->regs + 492);  return count;
}

static ssize_t register493_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register493, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register493_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register493 = tempValue;
  iowrite32(devp->register493,(u32 *)devp->regs + 493);  return count;
}

static ssize_t register494_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register494, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register494_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register494 = tempValue;
  iowrite32(devp->register494,(u32 *)devp->regs + 494);  return count;
}

static ssize_t register495_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register495, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register495_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register495 = tempValue;
  iowrite32(devp->register495,(u32 *)devp->regs + 495);  return count;
}

static ssize_t register496_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register496, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register496_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register496 = tempValue;
  iowrite32(devp->register496,(u32 *)devp->regs + 496);  return count;
}

static ssize_t register497_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register497, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register497_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register497 = tempValue;
  iowrite32(devp->register497,(u32 *)devp->regs + 497);  return count;
}

static ssize_t register498_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register498, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register498_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register498 = tempValue;
  iowrite32(devp->register498,(u32 *)devp->regs + 498);  return count;
}

static ssize_t register499_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register499, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register499_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register499 = tempValue;
  iowrite32(devp->register499,(u32 *)devp->regs + 499);  return count;
}

static ssize_t register500_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register500, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register500_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register500 = tempValue;
  iowrite32(devp->register500,(u32 *)devp->regs + 500);  return count;
}

static ssize_t register501_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register501, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register501_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register501 = tempValue;
  iowrite32(devp->register501,(u32 *)devp->regs + 501);  return count;
}

static ssize_t register502_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register502, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register502_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register502 = tempValue;
  iowrite32(devp->register502,(u32 *)devp->regs + 502);  return count;
}

static ssize_t register503_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register503, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register503_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register503 = tempValue;
  iowrite32(devp->register503,(u32 *)devp->regs + 503);  return count;
}

static ssize_t register504_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register504, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register504_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register504 = tempValue;
  iowrite32(devp->register504,(u32 *)devp->regs + 504);  return count;
}

static ssize_t register505_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register505, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register505_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register505 = tempValue;
  iowrite32(devp->register505,(u32 *)devp->regs + 505);  return count;
}

static ssize_t register506_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register506, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register506_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register506 = tempValue;
  iowrite32(devp->register506,(u32 *)devp->regs + 506);  return count;
}

static ssize_t register507_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register507, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register507_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register507 = tempValue;
  iowrite32(devp->register507,(u32 *)devp->regs + 507);  return count;
}

static ssize_t register508_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register508, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register508_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register508 = tempValue;
  iowrite32(devp->register508,(u32 *)devp->regs + 508);  return count;
}

static ssize_t register509_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register509, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register509_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register509 = tempValue;
  iowrite32(devp->register509,(u32 *)devp->regs + 509);  return count;
}

static ssize_t register510_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register510, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register510_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register510 = tempValue;
  iowrite32(devp->register510,(u32 *)devp->regs + 510);  return count;
}

static ssize_t register511_read(struct device *dev, struct device_attribute *attr, char *buf) { 
  fe_DPRAM_dev_t * devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  fp_to_string(buf, devp->register511, 0, true, 9);
  strcat2(buf, "\n");
  return strlen(buf);
}

static ssize_t register511_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { 
  uint32_t tempValue = 0;
  char substring[80];
  int substring_count = 0;
  int i;
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  for (i = 0; i < count; i++) {
    if((buf[i] !=',') && (buf[i] != ' ') && (buf[i] !='\0') && (buf[i] != '\r') && (buf[i] != '\n')) {
      substring[substring_count] = buf[i];
      substring_count++;
     }
   }
  substring[substring_count] = 0;
  tempValue = set_fixed_num(substring, 0, false);
  devp->register511 = tempValue;
  iowrite32(devp->register511,(u32 *)devp->regs + 511);  return count;
}

static ssize_t name_read(struct device *dev, struct device_attribute *attr, char *buf) {
  fe_DPRAM_dev_t *devp = (fe_DPRAM_dev_t *)dev_get_drvdata(dev);
  sprintf(buf, "%s\n", devp->name);
  return strlen(buf);
}

/* Device driver functions */
static int DPRAM_open(struct inode *inode, struct file *file) {
  pr_info("DPRAM : Device OPEN\n"); // Test
  return 0;
}

static int DPRAM_release(struct inode *inode, struct file *file) {
  pr_info("DPRAM : Device RELEASE\n"); // Test
  return 0;
}

static ssize_t DPRAM_read(struct file *filp, char *buffer, size_t len, loff_t *offset) {
  pr_info("DPRAM : Device READ\n"); // Test
  return 0;
}    

static ssize_t DPRAM_write(struct file *filp, const char *buffer, size_t len, loff_t *offset) {
  pr_info("DPRAM : Device WRITE\n"); // Test
  return 0;
}

static int DPRAM_remove(struct platform_device *pdev) {
 
  fe_DPRAM_dev_t *dev = (fe_DPRAM_dev_t *)platform_get_drvdata(pdev);
  
  pr_info("DPRAM_remove enter\n");
  
  cdev_del(&dev->cdev);
  unregister_chrdev_region(dev_num, 2);
  iounmap(dev->regs);
  
  pr_info("DPRAM_remove exit\n");
  
  return 0;
}

static void DPRAM_exit(void) {
  pr_info("Flat Earth Dual Port RAM module exit\n");
  platform_driver_unregister(&DPRAM_platform);
  pr_info("Flat Earth Dual Port RAM module successfully unregistered\n");
}

/* End of file */
module_init(DPRAM_init);
module_exit(DPRAM_exit);