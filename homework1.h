#ifndef HOMEWORK_H1
#define HOMEWORK_H1

typedef struct {
    int width;  // only need to store this data for the 2nd part
    int height;  // image is obv. b&w and maxval 255 
    unsigned char** data;
}image;

void initialize(image *im);
void render(image *im);
void writeData(const char * fileName, image *img);

#endif /* HOMEWORK_H1 */
