//
// Created by David Heddle on 6/1/20.
//

#include "magfielddraw.h"
#include "svg.h"
#include "mapcolor.h"
#include "magfieldutil.h"

/**
 * Create an SVG image of the fields at a fixed value of z.
 * @param path the path to the svg file.
 * @param z the fixed value of z in cm.
 * @param fieldPtr torus the torus field (can be NULL).
 * @param fieldPtr torus the solenoid field (can be NULL).
 */
void createSVGImageFixedZ(char *path, double z, MagneticFieldPtr torus, MagneticFieldPtr solenoid) {

    ColorMapPtr colorMap = defaultColorMap();

    double rhomax = (z > 99) ? 360 : 120;
    int xmin = -rhomax;
    int xmax = rhomax;
    int ymin = -rhomax;
    int ymax = rhomax;

    int del = 2;

    int marginLeft =  50;
    int marginRight =  90;
    int marginTop =  50;
    int marginBottom =  50;

    int imageWidth = xmax - xmin;
    int imageHeight = ymax - ymin;
    int width = imageWidth + marginLeft + marginRight;
    int height = imageHeight + marginTop + marginBottom;

    svg* psvg;
    psvg = svgStart(path, width, height);

    psvg = svgStart(path, width, height);

    svgFill(psvg, "#f0f0f0");

    fprintf(stdout, "\nStarting svg image creation for: [%s]", path);

    FieldValuePtr fieldValuePtr = (FieldValuePtr) malloc(sizeof (FieldValue));

    int y = ymin+del;
    while (y < ymax+del) {
        if ((y % 50) == 0) {
            fprintf(stdout, ".");
        }
        int yPic = y - ymin + marginTop; //x is vertical
        int x = xmin;
        while (x < xmax) {

            int xPic = x - xmin + marginLeft;

            getCompositeFieldValue(fieldValuePtr, x, y, z, torus, solenoid);
            double magnitude = fieldMagnitude(fieldValuePtr);

            char *color = getColor(colorMap, magnitude);
            svgRectangle(psvg, del, del, xPic, yPic, color, "none", 0, 0, 0);

            x += del;
        }
        y += del;
    }

    free(fieldValuePtr);

    //border
    svgRectangle(psvg, imageWidth, imageHeight, marginLeft, marginTop, "none", "black", 1, 0, 0);

    char *label = (char *) malloc(255);


    int x = marginLeft - 6;
    y = ymin;
    while (y <= ymax) {
        int yPic = y - ymin + marginTop;

        sprintf(label, "%d", y);
        svgRotatedText(psvg, x, yPic+8, "times", 12, "black", "none", -90, label);
        svgLine(psvg, "#cccccc", 1, marginLeft, yPic, marginLeft+imageWidth, yPic);
        y += 120;
    }




    x = xmin;
    y = marginTop + imageHeight + 20;
    while (x <= xmax) {

        int xPic = x - xmin + marginLeft;
        sprintf(label, "%d", x);
        svgText(psvg, xPic-12, y, "times", 12, "black", "none", label);
        svgLine(psvg, "#cccccc", 1, xPic, marginTop, xPic, marginTop+imageHeight);

        x += 120;
    }


    //title and axes labels
    sprintf(label, "Magnetic field magnitude for constant z = %-4.1f cm", z);
    svgText(psvg, marginLeft + 100, 30, "times", 16, "black", "none", label);
    svgText(psvg, marginLeft + imageWidth/2, height-15, "times", 14, "black", "none", "x (cm)");
    svgRotatedText(psvg, 20, marginTop + imageHeight/2, "times", 14, "black", "none", -90, "y (cm)");


    //the gradient
    x = width - 75;
    y = marginTop + 40;
    int nc = colorMap->numColors;
    int gw = 20;
    int gh = 4;
    for (int i = 0; i < colorMap->numColors; i++) {
        svgRectangle(psvg, gw, gh, x, y, colorMap->colors[i], "none", 0, 0, 0);
        bool lab = ((i == 0)  || (i == nc/4) || (i == nc/2) || (i == 3*nc/4));

        if (lab) {
            sprintf(label, " %-3.1f kG", colorMap->values[i]);
            svgText(psvg, x+gw+4, y+5, "times", 11, "black", "none", label);
        }
        y+=gh;
    }

    sprintf(label, " %-3.1f kG", colorMap->values[nc+1]);
    svgText(psvg, x+gw+4, y+5, "times", 11, "black", "none", label);

    free(label);
    svgEnd(psvg);

    fprintf(stdout, "done.\n");
}

/**
 * Create an SVG image of the fields at a fixed value of phi.
 * @param path the path to the svg file.
 * @param phi the fixed value of phi in degrees. For the canonical
 * sector 1 midplane, use phi = 0;
 * @param fieldPtr torus the torus field (can be NULL).
 * @param fieldPtr torus the solenoid field (can be NULL).
 */
void createSVGImageFixedPhi(char *path, double phi, MagneticFieldPtr torus, MagneticFieldPtr solenoid) {

    ColorMapPtr colorMap = defaultColorMap();

    double phiRad = toRadians(phi);
    double cosPhi = cos(phiRad);
    double sinPhi = sin(phiRad);

    int zmin = -100;
    int zmax = 500;
    int rmin = 0;
    int rmax = 360;

    int del = 2;

    int marginLeft =  50;
    int marginRight =  90;
    int marginTop =  50;
    int marginBottom =  50;

    int imageWidth = zmax - zmin;
    int imageHeight = rmax - rmin;
    int width = imageWidth + marginLeft + marginRight;
    int height = imageHeight + marginTop + marginBottom;

    svg* psvg;
    psvg = svgStart(path, width, height);

    svgFill(psvg, "#f0f0f0");



    fprintf(stdout, "\nStarting svg image creation for: [%s]", path);

    FieldValuePtr fieldValuePtr = (FieldValuePtr) malloc(sizeof (FieldValue));

    int rho = rmin + del;
    while (rho < rmax + del) {
        if ((rho % 50) == 0) {
            fprintf(stdout, ".");
        }
        int rhoPic = marginTop + imageHeight - rho; //rho is vertical


        int z = zmin;
        while (z < zmax) {

            int zPic = z - zmin + marginLeft;

            getCompositeFieldValue(fieldValuePtr, rho*cosPhi, rho*sinPhi, z, torus, solenoid);
            double magnitude = fieldMagnitude(fieldValuePtr);

            char *color = getColor(colorMap, magnitude);
            svgRectangle(psvg, del, del, zPic, rhoPic, color, "none", 0, 0, 0);

            z += del;
        }
        rho += del;
    }

    free(fieldValuePtr);

    //border
    svgRectangle(psvg, imageWidth, imageHeight, marginLeft, marginTop, "none", "black", 1, 0, 0);

    char *label = (char *) malloc(255);


    int z = marginLeft - 6;
    rho = rmin;
    while (rho <= rmax) {
        int xPic = rho - rmin + marginTop;

        sprintf(label, "%d", rmax - rho);
        svgRotatedText(psvg, z, xPic+8, "times", 12, "black", "none", -90, label);
        svgLine(psvg, "#cccccc", 1, marginLeft, xPic, marginLeft+imageWidth, xPic);
        rho += 60;
    }


    z = zmin;
    rho = marginTop + imageHeight + 20;
    while (z <= zmax) {

        int zPic = z - zmin + marginLeft;
        sprintf(label, "%d", z);
        svgText(psvg, zPic-12, rho, "times", 12, "black", "none", label);
        svgLine(psvg, "#cccccc", 1, zPic, marginTop, zPic, marginTop+imageHeight);

        z += 100;
    }


    //title and axes labels
    sprintf(label, "Magnetic field Magnitude for constant phi = %-4.1f degrees", phi);
    svgText(psvg, marginLeft + 100, 30, "times", 16, "black", "none", label);
    svgText(psvg, marginLeft + imageWidth/2, height-15, "times", 14, "black", "none", "z (cm)");
    svgRotatedText(psvg, 20, marginTop + imageHeight/2, "times", 14, "black", "none", -90, "rho (cm)");

    //the gradient
    int x = width - 75;
    int y = marginTop + 40;
    int gw = 20;
    int gh = 4;
    int nc = colorMap->numColors;

    for (int i = 0; i < nc; i++) {
        svgRectangle(psvg, gw, gh, x, y, colorMap->colors[i], "none", 0, 0, 0);

        bool lab = ((i == 0) ||  (i == nc/4) || (i == nc/2) || (i == 3*nc/4));

        if (lab) {
            sprintf(label, " %-3.1f kG", colorMap->values[i]);
            svgText(psvg, x+gw+4, y+5, "times", 11, "black", "none", label);
        }
        y+=gh;
    }

    sprintf(label, " %-3.1f kG", colorMap->values[nc+1]);
    svgText(psvg, x+gw+4, y+5, "times", 11, "black", "none", label);

    free(label);
    svgEnd(psvg);

    fprintf(stdout, "done.\n");
}


