/**
 * This is only a demo to show the usage of tcas_framer
 * milkyjing <milkyjing@gmail.com>
 * 2011-12-19
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>        /* we need the GetTickCount() and system() function, and also the console functions */

#include "tcas_framer.h"
#pragma comment(lib, "tcas_framer.lib")


int main(int argc, char **argv) {
    char szFilename[256];       /* filename of the TCAS */
    char szErr[256];
    TcasPtr tcas;
    int width, height;
    long minFrame, maxFrame;
    double fps;
    unsigned char *frameBuf;    /* a width * height RGBA bitmap */
    int i, len, frameCount;
    DWORD t1, t2;
    /* utility for showing the progress */
    HANDLE hStdout;
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    /* get the filename */
    if (argc > 1) {
        strcpy(szFilename, argv[1]);
        printf("filename: %s\n", szFilename);
    } else {
        printf("filename: ");
        fgets(szFilename, 256, stdin);
        szFilename[strlen(szFilename) - 1] = 0;
    }
    len = strlen(szFilename);
    if (stricmp(szFilename + len - 5, ".tcas") != 0) {
        printf("Error: invalid TCAS file name.\n");
        system("PAUSE");
        return 0;
    }
    /* openning the TCAS file */
    printf("Initializing...\n");
    t1 = GetTickCount();    /* get the starting time of the process */
    tcas = tcas_open(szFilename, szErr);        /* open the TCAS file, may take a while to generate the index */
    if (!tcas) {
        printf(szErr);
        system("PAUSE");
        return 0;
    }
    t2 = GetTickCount();    /* get the end time */
    printf("initializing takes %i mm\n", t2 - t1);
    /* show the basic information of the TCAS file */
    width = tcas_get_width(tcas);
    height = tcas_get_height(tcas);
    minFrame = tcas_get_min_frame(tcas);
    maxFrame = tcas_get_max_frame(tcas);
    fps = tcas_get_fps(tcas);
    printf("\nwidth=%i height=%i minFrame=%i maxFrame=%i frame rate=%.3f\n", width, height, minFrame, maxFrame, fps);
    /* get the current cursor position */
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdout, &csbi);
    coord.X = 0;
    coord.Y = csbi.dwCursorPosition.Y + 1;
    /* here goes the main process */
    frameCount = maxFrame - minFrame + 1;
    t1 = GetTickCount();
    for (i = minFrame; i <= maxFrame; i++) {
        frameBuf = tcas_get_frame(tcas, i);     /* get the ith frame, it's a width * height RGBA bitmap */
        /* TODO: play with the frameBuf */
        SetConsoleCursorPosition(hStdout, coord);   /* a little trick to keep the cursor steady */
        printf("progress: %.2f%%\n", 100 * (i - minFrame + 1) / (double)frameCount);    /* for not boring you during the processing period */
        free(frameBuf);     /* you should free it manually after finishing it */
    }
    t2 = GetTickCount();
    printf("\nviewed %i frames at the frame rate of %.3f frames per second.\n", frameCount, 1000 * frameCount / (double)(t2 - t1));
    tcas_close(tcas);       /* close the TCAS file and release the resource */
    system("PAUSE");
    return 0;
}

