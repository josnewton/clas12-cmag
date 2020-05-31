//
// Created by David Heddle on 5/30/20.
// Used only to make the svg picture of the map
//

#include "mapcolor.h"
#include "magfieldutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * Get a color from a color map.
 * @param cmapPtr a valid pointer to a color map.
 * @param value the value to convert into a color.
 * @return the color in "#rrggbb" format.
 */
char *getColor(ColorMapPtr cmapPtr, double value) {

    //the values are in descending order

    if (value > cmapPtr->values[0]) {
        return cmapPtr->tooBigColor;
    }
    if (value <= cmapPtr->values[cmapPtr->num]) {
        return cmapPtr->tooSmallColor;
    }

    int index;
    for (index = 0; index < cmapPtr->num; index++) {
        if (cmapPtr->values[index+1] < value) {
            break;
        }
    }
    return cmapPtr->colors[72-index];
}


/**
 * Get the default color map optimized for displaying torus and solenoid
 * @return he default color map.
 */
ColorMapPtr defaultColorMap() {

    ColorMapPtr cmapPtr = (ColorMapPtr) malloc(sizeof(ColorMap));

    int colorlen = 73;
    cmapPtr->num = colorlen;
    stringCopy(&(cmapPtr->tooSmallColor), "#ffffff");
    stringCopy(&(cmapPtr->tooBigColor), "#000000");

    char* colors[] = {
            "#fef4f2", "#fdf1e4", "#fbf1e4", "#f9f1e4", "#f7f1e4", "#f5f1e4",
            "#f3f1e4", "#f1f1e4", "#dff1e4", "#ddf1e4", "#dbf1e4", "#d9f1e4",
            "#dcf1e4",
            "#d7f1e4", "#d4eee4", "#cbeae5", "#c2e7e5", "#b9e4e5",
            "#b0e0e6", "#a1dad2", "#92d3bd", "#82cca8", "#73c593", "#63be7e",
            "#54b769", "#44b054", "#35a93f", "#25a22a", "#2fa22b", "#3aa12c",
            "#44a02d", "#4f9f2e", "#599f2f", "#649e30", "#6e9d30", "#799c32",
            "#849b33", "#91a62e", "#9fb128", "#adbc22", "#bac71d", "#c8d217",
            "#d6dd11", "#e3e80c", "#f1f306", "#ffff00", "#fff100", "#ffe300",
            "#ffd500", "#ffc700", "#ffb900", "#ffab00", "#ff9d00", "#ff8f00",
            "#ff8000", "#ff7200", "#ff6400", "#ff5600", "#ff4800", "#ff3900",
            "#ff2b00", "#ff1d00", "#ff0f00", "#ff0000", "#f1000e", "#e3001c",
            "#d5002a", "#c70038", "#b80046", "#aa0054", "#9c0062", "#8e0070",
            "#7f007f"};



    double values[] = {
            66.00000, 61.63079, 57.55083, 53.74095, 50.18330, 46.86116,
            43.75894, 40.86209, 38.15702, 35.63102, 33.27224, 31.06962,
            29.01280, 27.09215, 25.29865, 23.62388, 22.05997, 20.59960,
            19.23591, 17.96249, 16.77337, 15.66297, 14.62608, 13.65783,
            12.75368, 11.90939, 11.12098, 10.38477, 9.69730, 9.05534,
            8.45588, 7.89610, 7.37337, 6.88526, 6.42945, 6.00382,
            5.60637, 5.23523, 4.88866, 4.56503, 4.26282, 3.98062,
            3.71711, 3.47103, 3.24125, 3.02668, 2.82631, 2.63921,
            2.46450, 2.30135, 2.14900, 2.00674, 1.87389, 1.74984,
            1.63400, 1.52583, 1.42482, 1.33050, 1.24242, 1.16017,
            1.08337, 1.01165, 0.94468, 0.88214, 0.82375, 0.76921,
            0.71829, 0.67074, 0.62634, 0.58488, 0.54616, 0.51000,
            0.47624, 0.1};

    for (int i = 0; i < colorlen; i++) {
        cmapPtr->colors[i] = colors[i];
    }

    for (int i = 0; i <= colorlen; i++) {
        cmapPtr->values[i] = values[i];
    }

    return cmapPtr;
}


/**
 * Get the hex color string from color components
 * @param colorStr must be at last 8 characters
 * @param r the red component [0..255]
 * @param g the green component [0..255]
 * @param b the blue component [0..255]
 */
void colorToHex(char * colorStr, int r, int g, int b) {
    sprintf(colorStr, "#%02x%02x%02x", r, g, b);
}

