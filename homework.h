#ifndef HOMEWORK_H
#define HOMEWORK_H
#define IMAGE_SIZE_BW 1
#define IMAGE_SIZE_COL 3

#define BW 5
#define COLOR 6

#define UNDEFINED -1

typedef struct {
    int type;  // store all data for any type of image
    int width;
    int height;
    int maxval;
    unsigned char** bwData;  // bw image case
    unsigned char** redData;  // color image case
    unsigned char** greenData;
    unsigned char** blueData;
}image;

void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image * out);

#endif /* HOMEWORK_H */
