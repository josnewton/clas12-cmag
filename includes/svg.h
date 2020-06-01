//
// This mini svg creation package was  adopted from onewritten by Chris Webb
// which is available at https://github.com/CodeDrome/svg-library-c
// We did modify it to write to the file as it goes, which made it way faster.
//

#ifndef CMAGDEVEL_SVG_H
#define CMAGDEVEL_SVG_H

#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>
#include<math.h>

typedef struct svg {
    FILE *fd;
    int height;
    int width;
    bool finalized;
} svg;

//external prototypes
svg* svgStart(char *path, int width, int height);
void svgEnd(svg*);

void svgCircle(svg* psvg, char* stroke, int strokewidth, char* fill, int r, int cx, int cy);
void svgLine(svg* psvg, char* stroke, int strokewidth, int x1, int y1, int x2, int y2);
void svgRectangle(svg* psvg, int width, int height, int x, int y, char* fill, char* stroke, int strokewidth, int radiusx, int radiusy);
void svgFill(svg* psvg, char* fill);
void svgText(svg* psvg, int x, int y, char* fontfamily, int fontsize, char* fill, char* stroke, char* text);
void svgRotatedText(svg* psvg, int x, int y, char* fontfamily, int fontsize, char* fill, char* stroke, int angle, char* text);
void svgEllipse(svg* psvg, int cx, int cy, int rx, int ry, char* fill, char* stroke, int strokewidth);
#endif //CMAGDEVEL_SVG_H
