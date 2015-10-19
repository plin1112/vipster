#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

static float vert_off[8][3]={
    {0.0, 0.0, 0.0,},
    {0.0, 0.0, 1.0,},
    {0.0, 1.0, 0.0,},
    {0.0, 1.0, 1.0,},
    {1.0, 0.0, 0.0,},
    {1.0, 0.0, 1.0,},
    {1.0, 1.0, 0.0,},
    {1.0, 1.0, 1.0}};

static int nvert_lut[256]={
    0, 1, 1, 2, 1, 2, 2, 3,
    1, 2, 2, 3, 2, 3, 3, 2,
    1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 3,
    1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 3,
    2, 3, 3, 2, 3, 4, 4, 3,
    3, 4, 4, 3, 4, 3, 3, 2,
    1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 3,
    2, 3, 3, 4, 3, 2, 4, 3,
    3, 4, 4, 3, 4, 3, 3, 2,
    2, 3, 3, 4, 3, 4, 4, 3,
    3, 4, 4, 3, 4, 3, 3, 2,
    3, 4, 4, 3, 4, 3, 3, 2,
    4, 3, 3, 2, 3, 2, 2, 1,
    1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 3,
    2, 3, 3, 4, 3, 4, 4, 3,
    3, 4, 4, 3, 4, 3, 3, 2,
    2, 3, 3, 4, 3, 4, 4, 3,
    3, 4, 2, 3, 4, 3, 3, 2,
    3, 4, 4, 3, 4, 3, 3, 2,
    4, 3, 3, 2, 3, 2, 2, 1,
    2, 3, 3, 4, 3, 4, 4, 3,
    3, 4, 4, 3, 2, 3, 3, 2,
    3, 4, 4, 3, 4, 3, 3, 2,
    4, 3, 3, 2, 3, 2, 2, 1,
    3, 4, 4, 3, 4, 3, 3, 2,
    4, 3, 3, 2, 3, 2, 2, 1,
    2, 3, 3, 2, 3, 2, 2, 1,
    3, 2, 2, 1, 2, 1, 1, 0};

static int edge_lut[256]={
    0x000,  0x089,  0x013,  0x09A,  0x980,  0x909,  0x993,  0x91A,
    0x310,  0x399,  0x303,  0x38A,  0xA90,  0xA19,  0xA83,  0xA0A,
    0x04C,  0x0C5,  0x05F,  0x0D6,  0x9CC,  0x945,  0x9DF,  0x956,
    0x35C,  0x3D5,  0x34F,  0x3C6,  0xADC,  0xA55,  0xACF,  0xA46,
    0x026,  0x0AF,  0x035,  0x0BC,  0x9A6,  0x92F,  0x9B5,  0x93C,
    0x336,  0x3BF,  0x325,  0x3AC,  0xAB6,  0xA3F,  0xAA5,  0xA2C,
    0x06A,  0x0E3,  0x079,  0x0F0,  0x9EA,  0x963,  0x9F9,  0x970,
    0x37A,  0x3F3,  0x369,  0x3E0,  0xAFA,  0xA73,  0xAE9,  0xA60,
    0xC40,  0xCC9,  0xC53,  0xCDA,  0x5C0,  0x549,  0x5D3,  0x55A,
    0xF50,  0xFD9,  0xF43,  0xFCA,  0x6D0,  0x659,  0x6C3,  0x64A,
    0xC0C,  0xC85,  0xC1F,  0xC96,  0x58C,  0x505,  0x59F,  0x516,
    0xF1C,  0xF95,  0xF0F,  0xF86,  0x69C,  0x615,  0x68F,  0x606,
    0xC66,  0xCEF,  0xC75,  0xCFC,  0x5E6,  0x56F,  0x5F5,  0x57C,
    0xF76,  0xFFF,  0xF65,  0xFEC,  0x6F6,  0x67F,  0x6E5,  0x66C,
    0xC2A,  0xCA3,  0xC39,  0xCB0,  0x5AA,  0x523,  0x5B9,  0x530,
    0xF3A,  0xFB3,  0xF29,  0xFA0,  0x6BA,  0x633,  0x6A9,  0x620,
    0x620,  0x6A9,  0x633,  0x6BA,  0xFA0,  0xF29,  0xFB3,  0xF3A,
    0x530,  0x5B9,  0x523,  0x5AA,  0xCB0,  0xC39,  0xCA3,  0xC2A,
    0x66C,  0x6E5,  0x67F,  0x6F6,  0xFEC,  0xF65,  0xFFF,  0xF76,
    0x57C,  0x5F5,  0x56F,  0x5E6,  0xCFC,  0xC75,  0xCEF,  0xC66,
    0x606,  0x68F,  0x615,  0x69C,  0xF86,  0xF0F,  0xF95,  0xF1C,
    0x516,  0x59F,  0x505,  0x58C,  0xC96,  0xC1F,  0xC85,  0xC0C,
    0x64A,  0x6C3,  0x659,  0x6D0,  0xFCA,  0xF43,  0xFD9,  0xF50,
    0x65A,  0x5D3,  0x549,  0x5C0,  0xCDA,  0xC53,  0xCC9,  0xC40,
    0xA60,  0xAE9,  0xA73,  0xAFA,  0x3E0,  0x369,  0x3F3,  0x37A,
    0x970,  0x9F9,  0x963,  0x9EA,  0x0F0,  0x079,  0x0E3,  0x06A,
    0xA2C,  0xAA5,  0xA3F,  0xAB6,  0x3AC,  0x325,  0x3BF,  0x336,
    0x93C,  0x9B5,  0x92F,  0x9A6,  0x0BC,  0x035,  0x0AF,  0x026,
    0xA46,  0xACF,  0xA55,  0xADC,  0x3C6,  0x34F,  0x3D5,  0x35C,
    0x956,  0x9DF,  0x945,  0x9CC,  0x0D6,  0x05F,  0x0C5,  0x04C,
    0xA0A,  0xA83,  0xA19,  0xA90,  0x38A,  0x303,  0x399,  0x310,
    0x91A,  0x993,  0x909,  0x980,  0x09A,  0x013,  0x089,  0x000};

static int tri_lut[256][4][3]={
    {{0, 0, 0},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 1, 4},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{3, 7, 4},{ 3, 4, 1},{ 0, 0, 0},{ 0, 0, 0}},
	{{7,11, 8},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3,11},{ 0,11, 8},{ 0, 0, 0},{ 0, 0, 0}},
	{{7,11, 8},{ 0, 1, 4},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 8, 1},{ 8,11, 1},{ 1,11, 3},{ 0, 0, 0}},
	{{4, 8, 9},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 8, 9},{ 0, 3, 7},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 1, 8},{ 1, 8, 9},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 3, 9},{ 3, 9, 8},{ 3, 7, 8},{ 0, 0, 0}},
	{{4, 7,11},{ 4, 9,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{9, 3,11},{ 0, 3, 9},{ 0, 9, 4},{ 0, 0, 0}},
	{{1, 9,11},{ 1, 0,11},{ 0, 7,11},{ 0, 0, 0}},
	{{1, 9,11},{ 1, 3,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{2, 3, 6},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 2, 7},{ 6, 2, 7},{ 0, 0, 0},{ 0, 0, 0}},
	{{2, 3, 6},{ 0, 1, 4},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 7, 6},{ 4, 6, 1},{ 1, 6, 2},{ 0, 0, 0}},
	{{2, 3, 6},{ 7, 8,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 2, 8},{ 2, 8, 6},{ 8, 6,11},{ 0, 0, 0}},
	{{2, 3, 6},{ 7,11, 8},{ 0, 1, 4},{ 0, 0, 0}},
	{{1, 2, 4},{ 2, 4, 6},{ 4, 6, 8},{ 6, 8,11}},
	{{2, 3, 6},{ 4, 8, 9},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 2, 7},{ 6, 2, 7},{ 4, 8, 9},{ 0, 0, 0}},
	{{0, 1, 8},{ 1, 8, 9},{ 2, 3, 6},{ 0, 0, 0}},
	{{2, 6, 7},{ 2, 7, 9},{ 1, 2, 9},{ 7, 8, 9}},
	{{2, 3, 6},{ 4, 7,11},{ 4, 9,11},{ 0, 0, 0}},
	{{2, 6, 0},{ 0, 6, 9},{ 0, 9, 4},{ 6, 9,11}},
	{{1, 9,11},{ 1, 0,11},{ 0, 7,11},{ 2, 3, 6}},
	{{1, 9,11},{ 1, 2,11},{ 2, 6,11},{ 0, 0, 0}},
	{{1, 2, 5},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 1, 2, 5},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 2, 5},{ 0, 4, 5},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 5, 7},{ 5, 7, 2},{ 2, 7, 3},{ 0, 0, 0}},
	{{1, 2, 5},{ 7,11, 8},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3,11},{ 0,11, 8},{ 1, 2, 5},{ 0, 0, 0}},
	{{0, 2, 5},{ 0, 4, 5},{ 7,11, 8},{ 0, 0, 0}},
	{{2, 3,11},{ 2,11, 4},{ 2, 4, 5},{ 4, 8,11}},
	{{1, 2, 5},{ 4, 8, 9},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 1, 2, 5},{ 4, 8, 9},{ 0, 0, 0}},
	{{0, 2, 8},{ 2, 8, 5},{ 5, 8, 9},{ 0, 0, 0}},
	{{3, 7, 2},{ 2, 7, 5},{ 5, 7, 8},{ 5, 8, 9}},
	{{4, 7,11},{ 4, 9,11},{ 1, 2, 5},{ 0, 0, 0}},
	{{1, 2, 5},{ 3, 9,11},{ 3, 9, 4},{ 3, 4, 0}},
	{{0, 2, 5},{ 0, 5,11},{ 5,11, 9},{ 0, 7,11}},
	{{3, 9,11},{ 3, 9, 5},{ 3, 5, 2},{ 0, 0, 0}},
	{{1, 3, 6},{ 1, 5, 6},{ 0, 0, 0},{ 0, 0, 0}},
	{{5, 6, 7},{ 5, 7, 0},{ 5, 0, 1},{ 0, 0, 0}},
	{{4, 5, 6},{ 4, 6, 3},{ 4, 3, 0},{ 0, 0, 0}},
	{{4, 5, 6},{ 4, 6, 7},{ 0, 0, 0},{ 0, 0, 0}},
	{{7, 8,11},{ 1, 3, 5},{ 3, 5, 6},{ 0, 0, 0}},
	{{1, 5, 6},{ 1, 6, 8},{ 1, 8, 0},{ 6, 8,11}},
	{{4, 5, 6},{ 4, 6, 3},{ 4, 3, 0},{ 7, 8,11}},
	{{4, 5, 6},{ 4, 6,11},{ 4, 8,11},{ 0, 0, 0}},
	{{4, 8, 9},{ 1, 3, 6},{ 1, 5, 6},{ 0, 0, 0}},
	{{5, 6, 7},{ 5, 7, 0},{ 5, 0, 1},{ 4, 8, 9}},
	{{0, 8, 9},{ 0, 6, 9},{ 9, 5, 6},{ 0, 6, 3}},
	{{5, 6, 7},{ 5, 7, 8},{ 5, 8, 9},{ 0, 0, 0}},
	{{1, 3, 5},{ 3, 5, 6},{ 4, 7, 9},{ 7, 9,11}},
	{{0, 1, 4},{ 5, 6, 9},{ 6, 9,11},{ 0, 0, 0}},
	{{0, 3, 7},{ 5, 6, 9},{ 6, 9,11},{ 0, 0, 0}},
	{{5, 6, 9},{ 6, 9,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{6,10,11},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 6,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 1, 4},{ 6,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 3, 4},{ 3, 4, 7},{ 6,10,11},{ 0, 0, 0}},
	{{6, 7, 8},{ 6, 8,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 8,10},{ 0,10, 6},{ 0, 6, 3},{ 0, 0, 0}},
	{{6, 7, 8},{ 6, 8,10},{ 0, 1, 4},{ 0, 0, 0}},
	{{1, 3, 4},{ 3, 4,10},{ 3,10, 6},{ 4, 8,10}},
	{{4, 8, 9},{ 6,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 4, 8, 9},{ 6,10,11},{ 0, 0, 0}},
	{{0, 1, 8},{ 1, 8, 9},{ 6,10,11},{ 0, 0, 0}},
	{{1, 3, 9},{ 3, 9, 8},{ 3, 8, 7},{ 6,10,11}},
	{{4, 6, 7},{ 4, 6,10},{ 4,10, 9},{ 0, 0, 0}},
	{{0, 3, 4},{ 3, 4, 9},{ 3, 9, 6},{ 6, 9,10}},
	{{0, 1, 9},{ 0, 9, 6},{ 0, 6, 7},{ 6, 9,10}},
	{{1, 3, 9},{ 3, 9, 6},{ 6, 9,10},{ 0, 0, 0}},
	{{2, 3,10},{ 3,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 2,10},{ 0,10, 7},{ 7,10,11},{ 0, 0, 0}},
	{{2, 3,10},{ 3,10,11},{ 0, 1, 4},{ 0, 0, 0}},
	{{1, 4, 7},{ 1, 7,10},{ 7,10,11},{ 1,10, 2}},
	{{2, 8,10},{ 2, 8, 7},{ 2, 7, 3},{ 0, 0, 0}},
	{{0, 2, 8},{ 2, 8,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{2, 8,10},{ 2, 8, 7},{ 2, 7, 3},{ 0, 1, 4}},
	{{2, 8,10},{ 2, 8, 4},{ 2, 4, 1},{ 0, 0, 0}},
	{{2, 3,10},{ 3,10,11},{ 4, 8, 9},{ 0, 0, 0}},
	{{0, 2,10},{ 0,10,11},{ 0,11, 7},{ 4, 8, 9}},
	{{2, 3,10},{ 3,10,11},{ 0, 1, 8},{ 1, 8, 9}},
	{{1, 2, 9},{ 2, 9,10},{ 7, 8,11},{ 0, 0, 0}},
	{{4, 9, 7},{ 7, 9, 2},{ 2, 9,10},{ 2, 3, 7}},
	{{0, 2,10},{ 0,10, 9},{ 0, 9, 4},{ 0, 0, 0}},
	{{0, 3, 7},{ 1, 2, 9},{ 2, 9,10},{ 0, 0, 0}},
	{{1, 2, 9},{ 2, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 2, 5},{ 6,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 2, 5},{ 6,10,11},{ 0, 3, 7},{ 0, 0, 0}},
	{{0, 4, 5},{ 0, 5, 2},{ 6,10,11},{ 0, 0, 0}},
	{{4, 5, 7},{ 5, 7, 3},{ 5, 3, 2},{ 6,10,11}},
	{{1, 2, 5},{ 6, 7, 8},{ 6, 8,10},{ 0, 0, 0}},
	{{1, 2, 5},{ 0, 8,10},{ 0,10, 3},{ 3,10, 6}},
	{{0, 4, 5},{ 0, 5, 2},{ 6, 7, 8},{ 6, 8,10}},
	{{2, 3, 6},{ 4, 5, 8},{ 5, 8,10},{ 0, 0, 0}},
	{{1, 2, 5},{ 4, 8, 9},{ 6,10,11},{ 0, 0, 0}},
	{{0, 3, 7},{ 1, 2, 5},{ 4, 8, 9},{ 6,10,11}},
	{{0, 2, 8},{ 2, 8, 5},{ 5, 8, 9},{ 6,10,11}},
	{{2, 3, 6},{ 7, 8,11},{ 5, 9,10},{ 0, 0, 0}},
	{{4, 6, 7},{ 4, 6, 9},{ 6, 9,10},{ 1, 2, 5}},
	{{0, 1, 4},{ 5, 9,10},{ 2, 3, 6},{ 0, 0, 0}},
	{{0, 2, 7},{ 2, 7, 6},{ 5, 9,10},{ 0, 0, 0}},
	{{2, 3, 6},{ 5, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 3,11},{ 1,11,10},{ 1,10, 5},{ 0, 0, 0}},
	{{0, 1, 7},{ 1, 7, 5},{ 7, 5,11},{ 5,10,11}},
	{{0, 4, 5},{ 0, 5,11},{ 0, 3,11},{ 5,11,10}},
	{{4, 5, 7},{ 5, 7,10},{ 7,10,11},{ 0, 0, 0}},
	{{1, 5, 3},{ 3, 5, 8},{ 3, 8, 7},{ 5, 8,10}},
	{{0, 8,10},{ 0,10, 5},{ 0, 5, 1},{ 0, 0, 0}},
	{{0, 3, 7},{ 4, 5, 8},{ 5, 8,10},{ 0, 0, 0}},
	{{4, 5, 8},{ 5, 8,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 8, 9},{ 1, 3,11},{ 1,11,10},{ 1,10, 5}},
	{{0, 1, 4},{ 7, 8,11},{ 5, 9,10},{ 0, 0, 0}},
	{{0, 3, 8},{ 3, 8,11},{ 5, 9,10},{ 0, 0, 0}},
	{{7, 8,11},{ 5, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 3, 4},{ 3, 4, 7},{ 5, 9,10},{ 0, 0, 0}},
	{{0, 1, 4},{ 5, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 5, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{5, 9,10},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{5, 9,10},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 5, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 1, 4},{ 5, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 3, 4},{ 3, 4, 7},{ 5, 9,10},{ 0, 0, 0}},
	{{7, 8,11},{ 5, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 8},{ 3, 8,11},{ 5, 9,10},{ 0, 0, 0}},
	{{0, 1, 4},{ 7, 8,11},{ 5, 9,10},{ 0, 0, 0}},
	{{4, 8, 9},{ 1, 3,11},{ 1,11,10},{ 1,10, 5}},
	{{4, 5, 8},{ 5, 8,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 4, 5, 8},{ 5, 8,10},{ 0, 0, 0}},
	{{0, 8,10},{ 0,10, 5},{ 0, 5, 1},{ 0, 0, 0}},
	{{1, 5, 3},{ 3, 5, 8},{ 3, 8, 7},{ 5, 8,10}},
	{{4, 5, 7},{ 5, 7,10},{ 7,10,11},{ 0, 0, 0}},
	{{0, 4, 5},{ 0, 5,11},{ 0, 3,11},{ 5,11,10}},
	{{0, 1, 7},{ 1, 7, 5},{ 7, 5,11},{ 5,10,11}},
	{{1, 3,11},{ 1,11,10},{ 1,10, 5},{ 0, 0, 0}},
	{{2, 3, 6},{ 5, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 2, 7},{ 2, 7, 6},{ 5, 9,10},{ 0, 0, 0}},
	{{0, 1, 4},{ 5, 9,10},{ 2, 3, 6},{ 0, 0, 0}},
	{{4, 6, 7},{ 4, 6, 9},{ 6, 9,10},{ 1, 2, 5}},
	{{2, 3, 6},{ 7, 8,11},{ 5, 9,10},{ 0, 0, 0}},
	{{0, 2, 8},{ 2, 8, 5},{ 5, 8, 9},{ 6,10,11}},
	{{0, 3, 7},{ 1, 2, 5},{ 4, 8, 9},{ 6,10,11}},
	{{1, 2, 5},{ 4, 8, 9},{ 6,10,11},{ 0, 0, 0}},
	{{2, 3, 6},{ 4, 5, 8},{ 5, 8,10},{ 0, 0, 0}},
	{{0, 4, 5},{ 0, 5, 2},{ 6, 7, 8},{ 6, 8,10}},
	{{1, 2, 5},{ 0, 8,10},{ 0,10, 3},{ 3,10, 6}},
	{{1, 2, 5},{ 6, 7, 8},{ 6, 8,10},{ 0, 0, 0}},
	{{4, 5, 7},{ 5, 7, 3},{ 5, 3, 2},{ 6,10,11}},
	{{0, 4, 5},{ 0, 5, 2},{ 6,10,11},{ 0, 0, 0}},
	{{1, 2, 5},{ 6,10,11},{ 0, 3, 7},{ 0, 0, 0}},
	{{1, 2, 5},{ 6,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 2, 9},{ 2, 9,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 1, 2, 9},{ 2, 9,10},{ 0, 0, 0}},
	{{0, 2,10},{ 0,10, 9},{ 0, 9, 4},{ 0, 0, 0}},
	{{4, 9, 7},{ 7, 9, 2},{ 2, 9,10},{ 2, 3, 7}},
	{{1, 2, 9},{ 2, 9,10},{ 7, 8,11},{ 0, 0, 0}},
	{{2, 3,10},{ 3,10,11},{ 0, 1, 8},{ 1, 8, 9}},
	{{0, 2,10},{ 0,10,11},{ 0,11, 7},{ 4, 8, 9}},
	{{2, 3,10},{ 3,10,11},{ 4, 8, 9},{ 0, 0, 0}},
	{{2, 8,10},{ 2, 8, 4},{ 2, 4, 1},{ 0, 0, 0}},
	{{2, 8,10},{ 2, 8, 7},{ 2, 7, 3},{ 0, 1, 4}},
	{{0, 2, 8},{ 2, 8,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{2, 8,10},{ 2, 8, 7},{ 2, 7, 3},{ 0, 0, 0}},
	{{1, 4, 7},{ 1, 7,10},{ 7,10,11},{ 1,10, 2}},
	{{2, 3,10},{ 3,10,11},{ 0, 1, 4},{ 0, 0, 0}},
	{{0, 2,10},{ 0,10, 7},{ 7,10,11},{ 0, 0, 0}},
	{{2, 3,10},{ 3,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 3, 9},{ 3, 9, 6},{ 6, 9,10},{ 0, 0, 0}},
	{{0, 1, 9},{ 0, 9, 6},{ 0, 6, 7},{ 6, 9,10}},
	{{0, 3, 4},{ 3, 4, 9},{ 3, 9, 6},{ 6, 9,10}},
	{{4, 6, 7},{ 4, 6,10},{ 4,10, 9},{ 0, 0, 0}},
	{{1, 3, 9},{ 3, 9, 8},{ 3, 8, 7},{ 6,10,11}},
	{{0, 1, 8},{ 1, 8, 9},{ 6,10,11},{ 0, 0, 0}},
	{{0, 3, 7},{ 4, 8, 9},{ 6,10,11},{ 0, 0, 0}},
	{{4, 8, 9},{ 6,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 3, 4},{ 3, 4,10},{ 3,10, 6},{ 4, 9,10}},
	{{6, 7, 8},{ 6, 8,10},{ 0, 1, 4},{ 0, 0, 0}},
	{{0, 8,10},{ 0,10, 6},{ 0, 6, 3},{ 0, 0, 0}},
	{{6, 7, 8},{ 6, 8,10},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 3, 4},{ 3, 4, 7},{ 6,10,11},{ 0, 0, 0}},
	{{0, 1, 4},{ 6,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 6,10,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{6,10,11},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{5, 6, 9},{ 6, 9,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 5, 6, 9},{ 6, 9,11},{ 0, 0, 0}},
	{{0, 1, 4},{ 5, 6, 9},{ 6, 9,11},{ 0, 0, 0}},
	{{1, 3, 5},{ 3, 5, 6},{ 4, 7, 9},{ 7, 9,11}},
	{{5, 6, 7},{ 5, 7, 8},{ 5, 8, 9},{ 0, 0, 0}},
	{{0, 8, 9},{ 0, 6, 9},{ 9, 5, 6},{ 0, 6, 3}},
	{{5, 6, 7},{ 5, 7, 0},{ 5, 0, 1},{ 4, 8, 9}},
	{{4, 8, 9},{ 1, 3, 6},{ 1, 5, 6},{ 0, 0, 0}},
	{{4, 5, 6},{ 4, 6,11},{ 4, 8,11},{ 0, 0, 0}},
	{{4, 5, 6},{ 4, 6, 3},{ 4, 3, 0},{ 7, 8,11}},
	{{1, 5, 6},{ 1, 6, 8},{ 1, 8, 0},{ 6, 8,11}},
	{{7, 8,11},{ 1, 3, 5},{ 3, 5, 6},{ 0, 0, 0}},
	{{4, 5, 6},{ 4, 6, 7},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 5, 6},{ 4, 6, 3},{ 4, 3, 0},{ 0, 0, 0}},
	{{5, 6, 7},{ 5, 7, 0},{ 5, 0, 1},{ 0, 0, 0}},
	{{1, 3, 6},{ 1, 5, 6},{ 0, 0, 0},{ 0, 0, 0}},
	{{3, 9,11},{ 3, 9, 5},{ 3, 5, 2},{ 0, 0, 0}},
	{{0, 2, 5},{ 0, 5,11},{ 5,11, 9},{ 0, 7,11}},
	{{1, 2, 5},{ 3, 9,11},{ 3, 9, 4},{ 3, 4, 0}},
	{{4, 7,11},{ 4, 9,11},{ 1, 2, 5},{ 0, 0, 0}},
	{{3, 7, 2},{ 2, 7, 5},{ 5, 7, 8},{ 5, 8, 9}},
	{{0, 2, 8},{ 2, 8, 5},{ 5, 8, 9},{ 0, 0, 0}},
	{{0, 3, 7},{ 1, 2, 5},{ 4, 8, 9},{ 0, 0, 0}},
	{{1, 2, 5},{ 4, 8, 9},{ 0, 0, 0},{ 0, 0, 0}},
	{{2, 3,11},{ 2,11, 4},{ 2, 4, 5},{ 4, 8,11}},
	{{0, 2, 5},{ 0, 4, 5},{ 7,11, 8},{ 0, 0, 0}},
	{{0, 3,11},{ 0,11, 8},{ 1, 2, 5},{ 0, 0, 0}},
	{{1, 2, 5},{ 7,11, 8},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 5, 7},{ 5, 7, 2},{ 2, 7, 3},{ 0, 0, 0}},
	{{0, 2, 5},{ 0, 4, 5},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 1, 2, 5},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 2, 5},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 9,11},{ 1, 2,11},{ 2, 6,11},{ 0, 0, 0}},
	{{1, 9,11},{ 1, 0,11},{ 0, 7,11},{ 2, 3, 6}},
	{{2, 6, 0},{ 0, 6, 9},{ 0, 9, 4},{ 6, 9,11}},
	{{2, 3, 6},{ 4, 7,11},{ 4, 9,11},{ 0, 0, 0}},
	{{2, 6, 7},{ 2, 7, 9},{ 1, 2, 9},{ 7, 8, 9}},
	{{0, 1, 8},{ 1, 8, 9},{ 2, 3, 6},{ 0, 0, 0}},
	{{0, 2, 7},{ 6, 2, 7},{ 4, 8, 9},{ 0, 0, 0}},
	{{2, 3, 6},{ 4, 8, 9},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 2, 4},{ 2, 4, 6},{ 4, 6, 8},{ 6, 8,11}},
	{{2, 3, 6},{ 7,11, 8},{ 0, 1, 4},{ 0, 0, 0}},
	{{0, 2, 8},{ 2, 8, 6},{ 8, 6,11},{ 0, 0, 0}},
	{{2, 3, 6},{ 7, 8,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 7, 6},{ 4, 6, 1},{ 1, 6, 2},{ 0, 0, 0}},
	{{2, 3, 6},{ 0, 1, 4},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 2, 7},{ 6, 2, 7},{ 0, 0, 0},{ 0, 0, 0}},
	{{2, 3, 6},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 9,11},{ 1, 3,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 9,11},{ 1, 0,11},{ 0, 7,11},{ 0, 0, 0}},
	{{9, 3,11},{ 0, 3, 9},{ 0, 9, 4},{ 0, 0, 0}},
	{{4, 7,11},{ 4, 9,11},{ 0, 0, 0},{ 0, 0, 0}},
	{{1, 3, 9},{ 3, 9, 8},{ 3, 7, 8},{ 0, 0, 0}},
	{{0, 1, 8},{ 1, 8, 9},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 8, 9},{ 0, 3, 7},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 8, 9},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{4, 8, 1},{ 8,11, 1},{ 1,11, 3},{ 0, 0, 0}},
	{{7,11, 8},{ 0, 1, 4},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3,11},{ 0,11, 8},{ 0, 0, 0},{ 0, 0, 0}},
	{{7,11, 8},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{3, 7, 4},{ 3, 4, 1},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 1, 4},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 3, 7},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}},
	{{0, 0, 0},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}}};

static PyObject* make_iso_surf(PyObject *self, PyObject *args)
{
    PyArrayObject *volume_py, *gradient_py;
    float isoval;

    if (!PyArg_ParseTuple(args,"OfO",&volume_py,&isoval,&gradient_py)) return NULL;
    if (!PyArray_Check(volume_py)) return NULL;
    if (!PyArray_Check(gradient_py)) return NULL;

    int i2,j2,k2,vert_sum,nvert=0;
    float ratio;
    float tempvert[12][3];
    float tempnorm[12][3];

    int x = PyArray_DIM(volume_py,0);
    int y = PyArray_DIM(volume_py,1);
    int z = PyArray_DIM(volume_py,2);
    float (*volume)[y][z] = PyArray_GETPTR3(volume_py,0,0,0);
    float (*gradient)[x][y][z] = PyArray_GETPTR4(gradient_py,0,0,0,0);
    typedef struct{
        float v1[3];
        float n1[3];
        float c1[3];
        float v2[3];
        float n2[3];
        float c2[3];
        float v3[3];
        float n3[3];
        float c3[3];
    }face;
    face *faces = malloc(4*x*y*z*sizeof(face));

    for (int i=0;i<x;i++) {
        if(i==x-1){i2=0;}else{i2=i+1;}
        for (int j=0;j<y;j++) {
            if(j==y-1){j2=0;}else{j2=j+1;}
            for (int k=0;k<z;k++) {
                if(k==z-1){k2=0;}else{k2=k+1;}

                /*determine the cube-vertices inside of volume*/
                vert_sum=0;
                if (volume[i ][j ][k ]<isoval) vert_sum|=0x01;
                if (volume[i ][j ][k2]<isoval) vert_sum|=0x02;
                if (volume[i ][j2][k ]<isoval) vert_sum|=0x04;
                if (volume[i ][j2][k2]<isoval) vert_sum|=0x08;
                if (volume[i2][j ][k ]<isoval) vert_sum|=0x10;
                if (volume[i2][j ][k2]<isoval) vert_sum|=0x20;
                if (volume[i2][j2][k ]<isoval) vert_sum|=0x40;
                if (volume[i2][j2][k2]<isoval) vert_sum|=0x80;

                if (0<vert_sum&&vert_sum<255){
                    /* determine interpolated edge intersections and normals */
#define interpol(n_edge,n_edge2,n_edge3,i_1,j_1,k_1,i_2,j_2,k_2)\
ratio=(isoval-volume[i_1][j_1][k_1])/(volume[i_2][j_2][k_2]-volume[i_1][j_1][k_1]);\
tempvert[n_edge][0]=vert_off[n_edge2][0]+ratio*(vert_off[n_edge3][0]-vert_off[n_edge2][0]);\
tempvert[n_edge][1]=vert_off[n_edge2][1]+ratio*(vert_off[n_edge3][1]-vert_off[n_edge2][1]);\
tempvert[n_edge][2]=vert_off[n_edge2][2]+ratio*(vert_off[n_edge3][2]-vert_off[n_edge2][2]);\
tempnorm[n_edge][0]=gradient[0][i_1][j_1][k_1]+ratio*\
                        (gradient[0][i_2][j_2][k_2]-gradient[0][i_1][j_1][k_1]);\
tempnorm[n_edge][1]=gradient[1][i_1][j_1][k_1]+ratio*\
                        (gradient[1][i_2][j_2][k_2]-gradient[1][i_1][j_1][k_1]);\
tempnorm[n_edge][2]=gradient[2][i_1][j_1][k_1]+ratio*\
                        (gradient[2][i_2][j_2][k_2]-gradient[2][i_1][j_1][k_1]);
                    if (edge_lut[vert_sum]&&0x001) interpol( 0, 0, 1,i ,j ,k ,i ,j ,k2);
                    if (edge_lut[vert_sum]&&0x002) interpol( 1, 1, 5,i ,j ,k2,i2,j ,k2);
                    if (edge_lut[vert_sum]&&0x004) interpol( 2, 5, 4,i2,j ,k2,i2,j ,k );
                    if (edge_lut[vert_sum]&&0x008) interpol( 3, 0, 4,i ,j ,k ,i2,j ,k );
                    if (edge_lut[vert_sum]&&0x010) interpol( 4, 1, 3,i ,j ,k2,i ,j2,k2);
                    if (edge_lut[vert_sum]&&0x020) interpol( 5, 5, 7,i2,j ,k2,i2,j2,k2);
                    if (edge_lut[vert_sum]&&0x040) interpol( 6, 4, 6,i2,j ,k ,i2,j2,k );
                    if (edge_lut[vert_sum]&&0x080) interpol( 7, 0, 2,i ,j ,k ,i ,j2,k );
                    if (edge_lut[vert_sum]&&0x100) interpol( 8, 2, 3,i ,j2,k ,i ,j2,k2);
                    if (edge_lut[vert_sum]&&0x200) interpol( 9, 3, 7,i ,j2,k2,i2,j2,k2);
                    if (edge_lut[vert_sum]&&0x400) interpol(10, 6, 7,i2,j2,k ,i2,j2,k2);
                    if (edge_lut[vert_sum]&&0x800) interpol(11, 2, 6,i ,j2,k ,i2,j2,k );

                    for(int l=0;l<nvert_lut[vert_sum];l++){
                        faces[nvert].v1[0]=(i+tempvert[tri_lut[vert_sum][l][0]][0])/x;
                        faces[nvert].v1[1]=(j+tempvert[tri_lut[vert_sum][l][0]][1])/y;
                        faces[nvert].v1[2]=(k+tempvert[tri_lut[vert_sum][l][0]][2])/z;
                        faces[nvert].v2[0]=(i+tempvert[tri_lut[vert_sum][l][1]][0])/x;
                        faces[nvert].v2[1]=(j+tempvert[tri_lut[vert_sum][l][1]][1])/y;
                        faces[nvert].v2[2]=(k+tempvert[tri_lut[vert_sum][l][1]][2])/z;
                        faces[nvert].v3[0]=(i+tempvert[tri_lut[vert_sum][l][2]][0])/x;
                        faces[nvert].v3[1]=(j+tempvert[tri_lut[vert_sum][l][2]][1])/y;
                        faces[nvert].v3[2]=(k+tempvert[tri_lut[vert_sum][l][2]][2])/z;
                        if(isoval>0){
                            faces[nvert].n1[0]=tempnorm[tri_lut[vert_sum][l][0]][0];
                            faces[nvert].n1[1]=tempnorm[tri_lut[vert_sum][l][0]][1];
                            faces[nvert].n1[2]=tempnorm[tri_lut[vert_sum][l][0]][2];
                            faces[nvert].c1[0]=0.8;
                            faces[nvert].c1[1]=0.1;
                            faces[nvert].c1[2]=0.1;
                            faces[nvert].n2[0]=tempnorm[tri_lut[vert_sum][l][1]][0];
                            faces[nvert].n2[1]=tempnorm[tri_lut[vert_sum][l][1]][1];
                            faces[nvert].n2[2]=tempnorm[tri_lut[vert_sum][l][1]][2];
                            faces[nvert].c2[0]=0.8;
                            faces[nvert].c2[1]=0.1;
                            faces[nvert].c2[2]=0.1;
                            faces[nvert].n3[0]=tempnorm[tri_lut[vert_sum][l][2]][0];
                            faces[nvert].n3[1]=tempnorm[tri_lut[vert_sum][l][2]][1];
                            faces[nvert].n3[2]=tempnorm[tri_lut[vert_sum][l][2]][2];
                            faces[nvert].c3[0]=0.8;
                            faces[nvert].c3[1]=0.1;
                            faces[nvert].c3[2]=0.1;
                        }else{
                            faces[nvert].n1[0]=-tempnorm[tri_lut[vert_sum][l][0]][0];
                            faces[nvert].n1[1]=-tempnorm[tri_lut[vert_sum][l][0]][1];
                            faces[nvert].n1[2]=-tempnorm[tri_lut[vert_sum][l][0]][2];
                            faces[nvert].c1[0]=0.1;
                            faces[nvert].c1[1]=0.1;
                            faces[nvert].c1[2]=0.8;
                            faces[nvert].n2[0]=-tempnorm[tri_lut[vert_sum][l][1]][0];
                            faces[nvert].n2[1]=-tempnorm[tri_lut[vert_sum][l][1]][1];
                            faces[nvert].n2[2]=-tempnorm[tri_lut[vert_sum][l][1]][2];
                            faces[nvert].c2[0]=0.1;
                            faces[nvert].c2[1]=0.1;
                            faces[nvert].c2[2]=0.8;
                            faces[nvert].n3[0]=-tempnorm[tri_lut[vert_sum][l][2]][0];
                            faces[nvert].n3[1]=-tempnorm[tri_lut[vert_sum][l][2]][1];
                            faces[nvert].n3[2]=-tempnorm[tri_lut[vert_sum][l][2]][2];
                            faces[nvert].c3[0]=0.1;
                            faces[nvert].c3[1]=0.1;
                            faces[nvert].c3[2]=0.8;
                        }
                        nvert+=1;
                    }
                }
            }
        }
    }
    if (!realloc(faces,nvert*sizeof(face))) return NULL;
    npy_intp dims[]={nvert*27};
    PyObject *faces_py = PyArray_SimpleNewFromData(1,dims,NPY_FLOAT,faces);
    PyArray_ENABLEFLAGS((PyArrayObject*)faces_py,NPY_ARRAY_OWNDATA);
    return faces_py;
}

static PyMethodDef GuiMethods[] = {
    {"make_iso_surf",make_iso_surf,METH_VARARGS,"Construct isosurface"},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >=3
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "gui_c",
    NULL,
    -1,
    MolMethods
};

PyObject* PyInit_gui_c(void)
#else
PyMODINIT_FUNC initgui_c(void)
#endif
{
    import_array();
#if PY_MAJOR_VERSION >=3
    PyObject *module = PyModule_Create(&moduledef);
    if (module == NULL) return NULL;
    return module;
#else
    Py_InitModule("gui_c", GuiMethods);
#endif
}
