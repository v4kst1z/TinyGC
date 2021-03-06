//
// Created by v4kst1z.
//
#pragma once
#include <iostream>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  TypeName& operator=(const TypeName&)

#ifndef DEBUG
#define DEBUG 1  // set debug mode
#endif

#define LOG(x)                     \
  do {                             \
    if (DEBUG) {                   \
      std::cerr << x << std::endl; \
    }                              \
  } while (0)