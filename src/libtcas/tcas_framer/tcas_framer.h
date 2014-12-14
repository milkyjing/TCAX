/* 
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
 *  beta version: 2011-12-18
 *
 */

#ifndef TCAS_FRAMER_H
#define TCAS_FRAMER_H

#ifdef TCAS_FRAMER_INTERNAL
#define TCEXPORT __declspec(dllexport)
#else
#define TCEXPORT __declspec(dllimport)
#endif    /* TCAS_FRAMER_INTERNAL */

typedef struct _tcas_st *TcasPtr;

/* Inhibit C++ name-mangling for tcas functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Open a TCAS file.
 * @param filename the filename of the TCAS file
 * @param szErr holds the error message in case of an error occurs, should be larger than 256 characters, or can be NULL if you don't need the error message
 * @return TcasPtr
 */
extern TCEXPORT TcasPtr tcas_open(const char *filename, char *szErr);

/**
 * Get the width of the TCAS FX.
 */
extern TCEXPORT int tcas_get_width(const TcasPtr tcas);

/**
 * Get the height of the TCAS FX.
 */
extern TCEXPORT int tcas_get_height(const TcasPtr tcas);

/**
 * Get the starting frame number of the TCAS FX.
 */
extern TCEXPORT long tcas_get_min_frame(const TcasPtr tcas);

/**
 * Get the last frame number of the TCAS FX.
 */
extern TCEXPORT long tcas_get_max_frame(const TcasPtr tcas);

/**
 * Get the frame rate of the TCAS FX.
 */
extern TCEXPORT double tcas_get_fps(const TcasPtr tcas);

/**
 * Get the nth frame, note that, you should free it manually.
 */
extern TCEXPORT unsigned char *tcas_get_frame(const TcasPtr tcas, long n);

/**
 * Close an opened TCAS file, and release all the resource.
 */
extern TCEXPORT void tcas_close(TcasPtr tcas);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* LIBTCC_TCC_H */

