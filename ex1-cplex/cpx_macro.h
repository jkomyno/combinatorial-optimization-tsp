#pragma once

#include <ilcplex/cplex.h>
#include <cstring>
#include <stdexcept>
#include <string>

#define STRINGIZE(something) STRINGIZE_HELPER(something)
#define STRINGIZE_HELPER(something) #something

/**
 * typedefs of basic Callable Library entities,
 * i.e. environment (Env) and problem pointers (Prob).
 */

using Env = CPXENVptr;
using CEnv = CPXCENVptr;
using Prob = CPXLPptr;
using CProb = CPXCLPptr;

/* Cplex Error Status and Message Buffer */

extern int status;

const unsigned int BUF_SIZE = 4096;

extern char errmsg[BUF_SIZE];

/* Shortcut for declaring a Cplex Env */
#define DECL_ENV(name)                                                                      \
    name = CPXopenCPLEX(&status);                                                           \
    if (status) {                                                                           \
        CPXgeterrorstring(NULL, status, errmsg);                                            \
        int trailer = std::strlen(errmsg) - 1;                                              \
        if (trailer >= 0)                                                                   \
            errmsg[trailer] = '\0';                                                         \
        throw std::runtime_error(std::string(__FILE__) + ":" + STRINGIZE(__LINE__) + ": " + \
                                 errmsg);                                                   \
    }

/* Shortcut for declaring a Cplex Problem */
#define DECL_PROB(env, name)                                                                \
    name = CPXcreateprob(env, &status, "");                                                 \
    if (status) {                                                                           \
        CPXgeterrorstring(NULL, status, errmsg);                                            \
        int trailer = std::strlen(errmsg) - 1;                                              \
        if (trailer >= 0)                                                                   \
            errmsg[trailer] = '\0';                                                         \
        throw std::runtime_error(std::string(__FILE__) + ":" + STRINGIZE(__LINE__) + ": " + \
                                 errmsg);                                                   \
    }

/* Make a checked call to a Cplex API function */
#define CHECKED_CPX_CALL(func, env, ...)                                                        \
    do {                                                                                        \
        status = func(env, __VA_ARGS__);                                                        \
        if (status) {                                                                           \
            CPXgeterrorstring(env, status, errmsg);                                             \
            int trailer = std::strlen(errmsg) - 1;                                              \
            if (trailer >= 0)                                                                   \
                errmsg[trailer] = '\0';                                                         \
            throw std::runtime_error(std::string(__FILE__) + ":" + STRINGIZE(__LINE__) + ": " + \
                                     errmsg);                                                   \
        }                                                                                       \
    } while (false)
