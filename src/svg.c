//
// This mini svg creation package was adopted from one
// written by Chris Webb which is available at https://github.com/CodeDrome/svg-library-c
//

#include"svg.h"
#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>

/**
 * Append a string to the open svg file.
 * @param psvg pointer to the svg information.
 * @param text the text to append to the svg file.
 */
static void svgAppendString(svg* psvg, char* text) {
    if (psvg->fd != NULL) {
        fprintf(psvg->fd, "%s", text);
    }
}

/**
 * Append an integer value to the open svg file.
 * @param psvg pointer to the svg information.
 * @param n the integer to append
 */
static void svgAppendInt(svg* psvg, int n) {
    if (psvg->fd != NULL) {
        fprintf(psvg->fd, "%d", n);
    }
}

/**
 * Initialize the svg file creation process.
 * @param path a path to the ouyput file.
 * @param width the width of the image in pixels.
 * @param height the height of the image in pixels.
 * @return a pointer to the svg object.
 */
svg* svgStart(char *path, int width, int height) {
    svg* psvg = malloc(sizeof(svg));
    FILE *fd = fopen(path, "w");
    *psvg = (svg){.fd = fd, .width = width, .height = height, .finalized = false};

    svgAppendString(psvg, "<svg width='");
    svgAppendInt(psvg, width);
    svgAppendString(psvg, "px' height='");
    svgAppendInt(psvg, height);
    svgAppendString(psvg,
                    "px' xmlns='http://www.w3.org/2000/svg' version='1.1' xmlns:xlink='http://www.w3.org/1999/xlink'>\n");

    return psvg;
}

/**
 * Finalize the svg file and free all space.
 * @param psvg pointer to the svg information.
 */
void svgEnd(svg* psvg) {
    if(psvg->fd != NULL) {
        svgAppendString(psvg, "</svg>");
        psvg->finalized = true;
        fclose(psvg->fd);
        free(psvg);
    }
}

/**
 * Draw a circle. All units are pixels.
 * @param psvg pointer to the svg information.
 * @param stroke the outline color, usually in "#rrggbb" format.
 * @param strokewidth border line width.
 * @param fill the fill color, usually in "#rrggbb" format.
 * @param r the radius.
 * @param cx the x center
 * @param cy  the y center.
 */
void svgCircle(svg* psvg, char* stroke, int strokewidth, char* fill, int r, int cx, int cy) {
    svgAppendString(psvg, "    <circle stroke='");
    svgAppendString(psvg, stroke);
    svgAppendString(psvg, "' stroke-width='");
    svgAppendInt(psvg, strokewidth);
    svgAppendString(psvg, "px' fill='");
    svgAppendString(psvg, fill);
    svgAppendString(psvg, "' r='");
    svgAppendInt(psvg, r);
    svgAppendString(psvg, "' cy='");
    svgAppendInt(psvg, cy);
    svgAppendString(psvg, "' cx='");
    svgAppendInt(psvg, cx);
    svgAppendString(psvg, "' />\n");
}

/**
 * Draw a line. All units are pixels.
 * @param psvg pointer to the svg information.
 * @param stroke the line color, usually in "#rrggbb" format.
 * @param strokewidth the width of the line.
 * @param x1 x coordinate of start.
 * @param y1 y coordinate of start.
 * @param x2 x coordinate of end.
 * @param y2 y coordinate of end.
 */
void svgLine(svg* psvg, char* stroke, int strokewidth, int x1, int y1, int x2, int y2) {
    svgAppendString(psvg, "    <line stroke='");
    svgAppendString(psvg, stroke);
    svgAppendString(psvg, "' stroke-width='");
    svgAppendInt(psvg, strokewidth);
    svgAppendString(psvg, "px' y2='");
    svgAppendInt(psvg, y2);
    svgAppendString(psvg, "' x2='");
    svgAppendInt(psvg, x2);
    svgAppendString(psvg, "' y1='");
    svgAppendInt(psvg, y1);
    svgAppendString(psvg, "' x1='");
    svgAppendInt(psvg, x1);
    svgAppendString(psvg, "' />\n");
}

/**
 * Draw a rectangle. All units are pixels.
 * @param psvg pointer to the svg information.
 * @param width the width of the rectangle.
 * @param height the height of the rectangle.
 * @param x the left of the rectangle.
 * @param y th top of the rectangle.
 * @param fill the fill color, usually in "#rrggbb" format.
 * @param stroke the outline color, usually in "#rrggbb" format.
 * @param strokewidth the width of the outline.
 * @param radiusx for rounding the corners.
 * @param radiusy for rounding the corners.
 */
void svgRectangle(svg* psvg, int width, int height, int x, int y,
                  char* fill, char* stroke, int strokewidth, int radiusx, int radiusy) {
    svgAppendString(psvg, "    <rect fill='");
    svgAppendString(psvg, fill);
    svgAppendString(psvg, "' stroke='");
    svgAppendString(psvg, stroke);
    svgAppendString(psvg, "' stroke-width='");
    svgAppendInt(psvg, strokewidth);
    svgAppendString(psvg, "px' width='");
    svgAppendInt(psvg, width);
    svgAppendString(psvg, "' height='");
    svgAppendInt(psvg, height);
    svgAppendString(psvg, "' y='");
    svgAppendInt(psvg, y);
    svgAppendString(psvg, "' x='");
    svgAppendInt(psvg, x);
    svgAppendString(psvg, "' ry='");
    svgAppendInt(psvg, radiusy);
    svgAppendString(psvg, "' rx='");
    svgAppendInt(psvg, radiusx);
    svgAppendString(psvg, "' />\n");
}

/**
 * Fill the whole image area.
 * @param psvg pointer to the svg information.
 * @param fill the fill color, usually in "#rrggbb" format.
 */
void svgFill(svg* psvg, char* fill) {
    svgRectangle(psvg, psvg->width, psvg->height, 0, 0, fill, fill,
                 0, 0, 0);
}

/**
 * Draw some text. All units are pixels.
 * @param psvg pointer to the svg information.
 * @param x the baseline horizontal start
 * @param y the baseline vertical start
 * @param fontfamily the font family.
 * @param fontsize the font size.
 * @param fill the fill color, usually in "#rrggbb" format.
 * @param stroke the outline color, usually in "#rrggbb" format.
 * @param text the text to draw.
 */
void svgText(svg* psvg, int x, int y, char* fontfamily, int fontsize,
             char* fill, char* stroke, char* text) {
    svgAppendString(psvg, "    <text x='");
    svgAppendInt(psvg, x);
    svgAppendString(psvg, "' y = '");
    svgAppendInt(psvg, y);
    svgAppendString(psvg, "' font-family='");
    svgAppendString(psvg, fontfamily);
    svgAppendString(psvg, "' stroke='");
    svgAppendString(psvg, stroke);
    svgAppendString(psvg, "' fill='");
    svgAppendString(psvg, fill);
    svgAppendString(psvg, "' font-size='");
    svgAppendInt(psvg, fontsize);
    svgAppendString(psvg, "px'>");
    svgAppendString(psvg, text);
    svgAppendString(psvg, "</text>\n");
}

/**
 * Draw some text. All units are pixels.
 * @param psvg pointer to the svg information.
 * @param x the baseline horizontal start
 * @param y the baseline vertical start
 * @param fontfamily the font family.
 * @param fontsize the font size.
 * @param fill the fill color, usually in "#rrggbb" format.
 * @param stroke the outline color, usually in "#rrggbb" format.
 * @param angle the rotation angle in degrees.
 * @param text the text to draw.
 */
void svgRotatedText(svg* psvg, int x, int y, char* fontfamily, int fontsize,
             char* fill, char* stroke, int angle, char* text) {
    svgAppendString(psvg, "    <text x='");
    svgAppendInt(psvg, x);
    svgAppendString(psvg, "' y = '");
    svgAppendInt(psvg, y);
    //transform='rotate(-90)'
    svgAppendString(psvg, "' transform='rotate(");
    svgAppendInt(psvg, angle);
    svgAppendString(psvg, ",");
    svgAppendInt(psvg, x);
    svgAppendString(psvg, ",");
    svgAppendInt(psvg, y);
    svgAppendString(psvg, ")");
    svgAppendString(psvg, "' font-family='");
    svgAppendString(psvg, fontfamily);
    svgAppendString(psvg, "' stroke='");
    svgAppendString(psvg, stroke);
    svgAppendString(psvg, "' fill='");
    svgAppendString(psvg, fill);
    svgAppendString(psvg, "' font-size='");
    svgAppendInt(psvg, fontsize);
    svgAppendString(psvg, "px'>");


    svgAppendString(psvg, text);
    svgAppendString(psvg, "</text>\n");
}

/**
 * Draw an ellipse. All units are pixels.
 * @param psvg pointer to the svg information.
 * @param cx the horizontal center.
 * @param cy the vertical center.
 * @param rx the horizontal radius.
 * @param ry the vertical radius.
 * @param fill the fill color, usually in "#rrggbb" format.
 * @param stroke the outline color, usually in "#rrggbb" format.
 * @param strokewidth the width of the outline.
 */
void svgEllipse(svg* psvg, int cx, int cy, int rx, int ry,
                char* fill, char* stroke, int strokewidth) {
    svgAppendString(psvg, "    <ellipse cx='");
    svgAppendInt(psvg, cx);
    svgAppendString(psvg, "' cy='");
    svgAppendInt(psvg, cy);
    svgAppendString(psvg, "' rx='");
    svgAppendInt(psvg, rx);
    svgAppendString(psvg, "' ry='");
    svgAppendInt(psvg, ry);
    svgAppendString(psvg, "' fill='");
    svgAppendString(psvg, fill);
    svgAppendString(psvg, "' stroke='");
    svgAppendString(psvg, stroke);
    svgAppendString(psvg, "' stroke-width='");
    svgAppendInt(psvg, strokewidth);
    svgAppendString(psvg, "' />\n");
}





