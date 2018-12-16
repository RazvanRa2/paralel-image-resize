#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "homework.h"
int num_threads;
int resize_factor;

int outSize;
int outWidth;
unsigned char** bwInData;
unsigned char** bwOutData;
unsigned char** redInData;
unsigned char** redOutData;
unsigned char** greenInData;
unsigned char** greenOutData;
unsigned char** blueInData;
unsigned char** blueOutData;

int mulFactor;
int gaussianKernel[3][3] = {{1,2,1}, {2,4,2}, {1,2,1}};

void readInput(const char * fileName, image *img) {
    // open input file for reading
    FILE *filePointer = fopen(fileName, "rb");
    if (filePointer == NULL) {
        perror("error opening input file");
        exit(1);
    }

    // read image type in tempImageTypeBuffer
    char tempImageType[3] = {'\0'};
    fread(tempImageType, 1, 2,filePointer);
    // read \n in junk
    char junk[1];
    fread(junk, 1, 1, filePointer);

    // read image width in width buffer, then convert to integer widthAsNum
    char width[10] = {'\0'};
    int widthAsNum = -1;
    int i = 0;
    fread(junk, 1, 1, filePointer);
    while(junk[0] != ' ') {
        width[i] = junk[0];
        i++;
        fread(junk, 1, 1, filePointer);
    }
    widthAsNum = atoi(width);

    // read image height in height buffer, then convert to integer heightAsNum
    char height[10] = {'\0'};
    int heightAsNum = -1;
    i = 0;
    fread(junk, 1, 1, filePointer);
    while(junk[0] != '\n') {
        height[i] = junk[0];
        i++;
        fread(junk, 1, 1, filePointer);
    }
    heightAsNum = atoi(height);

    //read maxval in maxval buffer, then convert to integer maxvalAsNum
    char maxval[10] = {'\0'};
    int maxvalAsNum = -1;
    i = 0;
    fread(junk, 1, 1, filePointer);
    while(junk[0] != '\n') {
        maxval[i] = junk[0];
        i++;
        fread(junk, 1, 1, filePointer);
    }
    maxvalAsNum = atoi(maxval);

    // read image data
    if (tempImageType[1] == '5') {  // image is b&w
        img->bwData = (unsigned char **) malloc(heightAsNum * sizeof(unsigned char *));
        for (int i = 0; i < heightAsNum; i++) {
            img->bwData[i] = (unsigned char *) malloc(widthAsNum * sizeof(unsigned char *));
        }

        unsigned char *buffer = (unsigned char *)malloc(widthAsNum * heightAsNum);
    	fread(buffer, sizeof(unsigned char), widthAsNum * heightAsNum, filePointer);

    	int k = 0;
    	for (int i = 0; i < heightAsNum; i++) {
    		for (int j = 0; j < widthAsNum; j++) {
    			img->bwData[i][j] = buffer[k++];
    		}
    	}
    	free(buffer);
        img->redData = NULL;
        img->greenData = NULL;
        img->blueData = NULL;
    } else {  // image is in color
        img->redData = (unsigned char **) malloc(heightAsNum * sizeof(unsigned char *));
        img->greenData = (unsigned char **) malloc(heightAsNum * sizeof(unsigned char *));
        img->blueData = (unsigned char **) malloc(heightAsNum * sizeof(unsigned char *));

        for (int i = 0; i < heightAsNum; i++) {
            img->redData[i] = (unsigned char *)malloc(widthAsNum * sizeof(unsigned char));
            img->greenData[i] = (unsigned char *)malloc(widthAsNum * sizeof(unsigned char));
            img->blueData[i] = (unsigned char *)malloc(widthAsNum * sizeof(unsigned char));
        }

        unsigned char *buffer = (unsigned char *)malloc(heightAsNum * widthAsNum * 3);
        fread(buffer, 1, heightAsNum * widthAsNum * 3, filePointer);

        int k = 0;
        for (int i = 0; i < heightAsNum; i++) {
            for (int j = 0; j < widthAsNum; j++) {
                img->redData[i][j] = buffer[k++];
                img->greenData[i][j] = buffer[k++];
                img->blueData[i][j] = buffer[k++];
            }
        }
        free(buffer);
        img->bwData = NULL;
    }
    // close input file
    fclose(filePointer);

    // put read image metadata inside the actual data structure
    if (tempImageType[1] == '5') {
        img->type = BW;
    } else {
        img->type = COLOR;
    }
    img->width = widthAsNum;
    img->height = heightAsNum;
    img->maxval = maxvalAsNum;
}

void writeData(const char * fileName, image *img) {
    // open output file for writing
    FILE *filePointer;
    filePointer = fopen(fileName, "wb");
    if (filePointer == NULL) {
        perror("error opening input file");
        exit(1);
    }

    if (img->type == BW)
    {
        char imgTypeLine[] = "P5\n";
        fwrite(&imgTypeLine, sizeof(char), 3 * sizeof(char), filePointer);
    } else {
        char imgTypeLine[] = "P6\n";
        fwrite(&imgTypeLine, sizeof(char), 3 * sizeof(char), filePointer);
    }

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
    char maxvalArray[100];
    sprintf(maxvalArray, "%d", img->maxval);
    fwrite(&maxvalArray, sizeof(char), strlen(maxvalArray), filePointer);
    fwrite(&newLine, sizeof(char), 1 * sizeof(char), filePointer);

    // write image data
    if (img->type == BW || img->type == COLOR) {
        if (img->type == BW) {
            for (int i = 0; i < img->height; i++) {
                fwrite(img->bwData[i], sizeof(unsigned char), img->width, filePointer);
            }
        } else {
            for (int i = 0; i < img->height; i++) {
                int k = 0;
                for (int j = 0; j < img->width * 3; j++) {
                    if (j % 3 == 0) {
                        fwrite(&img->redData[i][k], 1, sizeof(unsigned char), filePointer);
                    }
                    if (j % 3 == 1) {
                        fwrite(&img->greenData[i][k], 1, sizeof(unsigned char), filePointer);
                    }
                    if (j % 3 == 2) {
                        fwrite(&img->blueData[i][k], 1, sizeof(unsigned char), filePointer);
                        k++;
                    }
                }
            }
        }
    }
    // safety check
    // do not free unallocated memory
    if (img->type == BW || img->type == COLOR) {
        if (img->type == BW) {
            for (int i = 0; i < img->height; i++) {
                free(img->bwData[i]);
            }
            free(img->bwData);
        }
    }
    fclose(filePointer);
}

void* ssaaBW2(void *var) { // bw image with even resize factor
    int thread_id = *(int*)var;
    // split image in chuncks, one chunk for each thread, each chunk corespons to
    // one pixel from final image
    int lowBound = mulFactor * thread_id;
    int highBound = (int)fmin(mulFactor * (thread_id + 1), outSize);

    for (int i = lowBound; i < highBound; i++) {
        int outI = i / outWidth;
        int outJ = i % outWidth;
        int startLine = outI * resize_factor;  // determine start position for chunk
        int startCol = outJ * resize_factor;  // and end position
        int colorSum  = 0;
        // determine color
        for (int j = startLine; j < startLine + resize_factor; j++) {
            for (int k = startCol; k < startCol + resize_factor; k++) {
                colorSum += bwInData[j][k];
            }
        }
        bwOutData[outI][outJ] = (unsigned char) (colorSum / (resize_factor * resize_factor));
    }
    return NULL;
}

void* ssaaBW3(void *var) {  // litreally the same as the function above
    int thread_id = *(int*)var;

    int lowBound = mulFactor * thread_id;
    int highBound = (int)fmin(mulFactor * (thread_id + 1), outSize);
    for (int i = lowBound; i < highBound; i++) {
        int outI = i / outWidth;
        int outJ = i % outWidth;
        int startLine = outI * resize_factor;
        int startCol = outJ * resize_factor;
        int colorSum  = 0;
        for (int j = startLine; j < startLine + resize_factor; j++) {
            for (int k = startCol; k < startCol + resize_factor; k++) {
                // except this guy gauss had a few words to say
                colorSum += bwInData[j][k] * gaussianKernel[j % 3][k % 3];
            }
        }
        bwOutData[outI][outJ] = (unsigned char) (colorSum / 16);
    }
    return NULL;
}  // yes, this is duplicate code, but i want to finish this assignment fast

void* ssaaCOL2(void *var) {  // color image with even resize factor
    int thread_id = *(int*)var;
    // same as ssaaBW2, except 3 colors instead of one
    int lowBound = mulFactor * thread_id;
    int highBound = (int)fmin(mulFactor * (thread_id + 1), outSize);

    for (int i = lowBound; i < highBound; i++) {
        int outI = i / outWidth;
        int outJ = i % outWidth;
        int startLine = outI * resize_factor;
        int startCol = outJ * resize_factor;
        int redSum  = 0;
        int greenSum = 0;
        int blueSum = 0;
        for (int j = startLine; j < startLine + resize_factor; j++) {
            for (int k = startCol; k < startCol + resize_factor; k++) {
                redSum += redInData[j][k];
                greenSum += greenInData[j][k];
                blueSum += blueInData[j][k];
            }
        }
        redOutData[outI][outJ] = (unsigned char) (redSum / (resize_factor * resize_factor));
        greenOutData[outI][outJ] = (unsigned char) (greenSum / (resize_factor * resize_factor));
        blueOutData[outI][outJ] = (unsigned char) (blueSum / (resize_factor * resize_factor));
    }
    return NULL;
}

void* ssaaCOL3(void *var) {
    int thread_id = *(int*)var;
    // same as ssaaBW3 except 3 colors instead of one
    // if these comments sound passive aggresive to you
    // it's 1:18am and i have to go to work tomorrow morning
    // sorry, this assignment was really fun
    int lowBound = mulFactor * thread_id;
    int highBound = (int)fmin(mulFactor * (thread_id + 1), outSize);
    for (int i = lowBound; i < highBound; i++) {
        int outI = i / outWidth;
        int outJ = i % outWidth;
        int startLine = outI * resize_factor;
        int startCol = outJ * resize_factor;
        int redSum  = 0;
        int greenSum = 0;
        int blueSum = 0;
        for (int j = startLine; j < startLine + resize_factor; j++) {
            for (int k = startCol; k < startCol + resize_factor; k++) {
                redSum += redInData[j][k] * gaussianKernel[j % 3][k % 3];
                greenSum += greenInData[j][k] * gaussianKernel[j % 3][k % 3];
                blueSum += blueInData[j][k] * gaussianKernel[j % 3][k % 3];
            }
        }
        redOutData[outI][outJ] = (unsigned char) (redSum / 16);
        greenOutData[outI][outJ] = (unsigned char) (greenSum / 16);
        blueOutData[outI][outJ] = (unsigned char) (blueSum / 16);
    }
    return NULL;
}

void resize(image *in, image * out) {
    // initialize image metadata
    out->type = in->type;
    out->width = in->width / resize_factor;
    out->height = in->height / resize_factor;
    out->maxval = in->maxval;  // TODO check if this is true or matters

    // allocate memory for image
    if (out->type == BW) {
        out->bwData = (unsigned char **) malloc(out->height * sizeof(unsigned char *));
        for (int i = 0; i < out->height; i++) {
            out->bwData[i] = (unsigned char *) malloc(out->width * sizeof(unsigned char *));
        }
        out->redData = NULL;
        out->greenData = NULL;
        out->blueData = NULL;
    } else {
        out->redData = (unsigned char **) malloc(out->height * sizeof(unsigned char *));
        out->greenData = (unsigned char **) malloc(out->height * sizeof(unsigned char *));
        out->blueData = (unsigned char **) malloc(out->height * sizeof(unsigned char *));

        for (int i = 0; i < out->height; i++) {
            out->redData[i] = (unsigned char *)malloc(out->width * sizeof(unsigned char));
            out->greenData[i] = (unsigned char *)malloc(out->width * sizeof(unsigned char));
            out->blueData[i] = (unsigned char *)malloc(out->width * sizeof(unsigned char));
        }
        out->bwData = NULL;
    }

    // initialize auxiliary global vars
    outSize = out->width * out->height;

    outWidth = out->width;

    bwInData = in->bwData;
    redInData = in->redData;
    greenInData = in->greenData;
    blueInData = in->blueData;

    bwOutData = out->bwData;
    redOutData = out->redData;
    greenOutData = out->greenData;
    blueOutData = out->blueData;

    // initialize thread partition mulFactor
    mulFactor = (int)ceil(1.0f * outSize / num_threads);

    pthread_t tid[num_threads];
    int thread_id[num_threads];
    for (int i = 0; i < num_threads; i++) {  // start makina
        thread_id[i] = i;
    }
    if (out->type == BW && resize_factor % 2 == 0)  // bw image and even factor
        for(int i = 0; i < num_threads; i++) {
            pthread_create(&(tid[i]), NULL, ssaaBW2, &(thread_id[i]));
        }
    if (out->type == BW && resize_factor == 3)
        for(int i = 0; i < num_threads; i++) {
            pthread_create(&(tid[i]), NULL, ssaaBW3, &(thread_id[i]));  // ... and odd factor
        }
    if (out->type == COLOR && resize_factor % 2 == 0)
        for(int i = 0; i < num_threads; i++) {
            pthread_create(&(tid[i]), NULL, ssaaCOL2, &(thread_id[i]));  // color image and even factor
        }

    if (out->type == COLOR && resize_factor == 3)
        for(int i = 0; i < num_threads; i++) {
            pthread_create(&(tid[i]), NULL, ssaaCOL3, &(thread_id[i]));  // ... and odd factor
        }

    for(int i = 0; i < num_threads; i++) {
        pthread_join(tid[i], NULL);  // stop makina
    }
}
