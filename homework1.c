#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "homework1.h"

int num_threads;
int resolution;
int mulFactor;

// Line ecuation is ax + by = 0 (c = 0)
int a = -1;
int b = 2;

unsigned char** imgData;

void initialize(image *im) {
    im->width = resolution;  // init metadata for image
    im->height = resolution;  // and allocate space
    im->data = (unsigned char **) malloc(im->height * sizeof(unsigned char *));
    for (int i = 0; i < im->height; i++) {
        im->data[i] = (unsigned char *) calloc(im->width * sizeof(unsigned char *),
         sizeof(unsigned char *));
    }
}
// function does rendering for a specific chunk of the image
void* threadRender(void *var) {
    int thread_id = *(int*)var;
    // calculating thread boundaries
    int lowBound = mulFactor * thread_id;
    int highBound = (int)fmin(mulFactor * (thread_id + 1), resolution * resolution);

    for (int cnt = lowBound; cnt < highBound; cnt++) {
        int i = cnt / resolution;
        int j = cnt % resolution;
        // taking into account x axis flip and half pixel distance
        float x = (j + 0.5f) * 100 / resolution;
        float y = (abs(i - resolution) - 0.5f) * 100 / resolution;
        // calculate distance in cm rather then pixels
        // and compare distances
        if ((abs(b * y + a * x)) / sqrt(a * a + b * b) >= 3) {  // all pixels are already black
            imgData[i][j] = 255; // just make the ones far away white
        }
    }
    return NULL;
}

void render(image *im) {
    pthread_t tid[num_threads];
    int thread_id[num_threads];

    // initialize auxilary vars
    imgData = im->data;
    mulFactor = (int)ceil(1.0f * resolution * resolution / num_threads);

    for (int i = 0; i < num_threads; i++) {  // start makina
        thread_id[i] = i;
    }
    for(int i = 0; i < num_threads; i++) {  // go makina
        pthread_create(&(tid[i]), NULL, threadRender, &(thread_id[i]));
    }

    for(int i = 0; i < num_threads; i++) {  // end makina
        pthread_join(tid[i], NULL);
    }
    // done makina
}

void writeData(const char * fileName, image *img) {
    // open output file for writing
    FILE *filePointer;
    filePointer = fopen(fileName, "wb");
    if (filePointer == NULL) {
        perror("error opening input file");
        exit(1);
    }

    char imgTypeLine[] = "P5\n";
    fwrite(&imgTypeLine, sizeof(char), 3 * sizeof(char), filePointer);
    //write image width and height line
    char widthArray[100];
    char heightArray[100];
    sprintf(widthArray, "%d", img->width);
    sprintf(heightArray, "%d", img->height);
    char whiteSpace[] = " ";
    char newLine[] = "\n";
    fwrite(&widthArray, sizeof(char), strlen(widthArray), filePointer);
    fwrite(&whiteSpace, sizeof(char), 1 * sizeof(char), filePointer);
    fwrite(&heightArray, sizeof(char), strlen(heightArray), filePointer);
    fwrite(&newLine, sizeof(char), 1 * sizeof(char), filePointer);
    // write maxval line
    char maxvalArray[] = "255\n";  // maxval will always be 255
    fwrite(&maxvalArray, sizeof(char), strlen(maxvalArray), filePointer);

    // write image data
    for (int i = 0; i < img->height; i++) {
        fwrite(img->data[i], sizeof(unsigned char), img->width, filePointer);
    }
    // done writing data to file
    fclose(filePointer);

    for (int i = 0; i < img->height; i++) {
        free(img->data[i]);
    }
    free(img->data);
}
