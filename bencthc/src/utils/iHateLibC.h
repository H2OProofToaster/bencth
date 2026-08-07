//
// Created by nick on 8/5/26.
//

#ifndef BENCTH_IHATELIBC_H
#define BENCTH_IHATELIBC_H

typedef
unsigned long size_t;

typedef
long ssize_t;

//integers *are* 32 bit so should die on overflow
#define INT_TO_STRING_SIZE 32

#define NULL ((void*)0)

#endif //BENCTH_IHATELIBC_H