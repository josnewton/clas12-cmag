//
// This mini svg creation package was written by Chris Webb
// It is available at https://github.com/CodeDrome/svg-library-c
//

#include"svg.h"
#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<time.h>

//these prototypes are from the main.c
//now used for test
static void drawrectangles(void);
static void drawallshapes(void);
static void iwanttobelieve(void);
static void mondrian(void);
static void stochasticpi(void);

// --------------------------------------------------------
// STATIC FUNCTION appendstringtosvg
// --------------------------------------------------------
static void appendstringtosvg(svg* psvg, char* text)
{
    int l = strlen(psvg->svg) + strlen(text) + 1;

    char* p = realloc(psvg->svg, l);

    if(p)
    {
        psvg->svg = p;
    }

    strcat(psvg->svg, text);
}

// --------------------------------------------------------
// STATIC FUNCTION appendnumbertosvg
// --------------------------------------------------------
static void appendnumbertosvg(svg* psvg, int n)
{
    char sn[16];

    sprintf(sn, "%d", n);

    appendstringtosvg(psvg, sn);
}

// --------------------------------------------------------
// FUNCTION svg_create
// --------------------------------------------------------
svg* svg_create(int width, int height)
{
    svg* psvg = malloc(sizeof(svg));

    if(psvg != NULL)
    {
        *psvg = (svg){.svg = NULL, .width = width, .height = height, .finalized = false};

        psvg->svg = malloc(1);

        sprintf(psvg->svg, "%s", "\0");

        appendstringtosvg(psvg, "<svg width='");
        appendnumbertosvg(psvg, width);
        appendstringtosvg(psvg, "px' height='");
        appendnumbertosvg(psvg, height);
        appendstringtosvg(psvg, "px' xmlns='http://www.w3.org/2000/svg' version='1.1' xmlns:xlink='http://www.w3.org/1999/xlink'>\n");

        return psvg;
    }
    else
    {
        return NULL;
    }
}

// --------------------------------------------------------
// FUNCTION svg_finalize
// --------------------------------------------------------
void svg_finalize(svg* psvg)
{
    appendstringtosvg(psvg, "</svg>");

    psvg->finalized = true;
}

// --------------------------------------------------------
// FUNCTION svg_print
// --------------------------------------------------------
void svg_print(svg* psvg)
{
    printf("%s\n", psvg->svg);
}

// --------------------------------------------------------
// FUNCTION svg_save
// --------------------------------------------------------
void svg_save(svg* psvg, char* filepath)
{
    if(!psvg->finalized)
    {
        svg_finalize(psvg);
    }

    FILE* fp;

    fp = fopen(filepath, "w");

    if(fp != NULL)
    {
        fwrite(psvg->svg, 1, strlen(psvg->svg), fp);

        fclose(fp);
    }
}

//----------------------------------------------------------------
// FUNCTION svg_free
//----------------------------------------------------------------
void svg_free(svg* psvg)
{
    free(psvg->svg);

    free(psvg);
}

//----------------------------------------------------------------
// FUNCTION svg_circle
//----------------------------------------------------------------
void svg_circle(svg* psvg, char* stroke, int strokewidth, char* fill, int r, int cx, int cy)
{
    appendstringtosvg(psvg, "    <circle stroke='");
    appendstringtosvg(psvg, stroke);
    appendstringtosvg(psvg, "' stroke-width='");
    appendnumbertosvg(psvg, strokewidth);
    appendstringtosvg(psvg, "px' fill='");
    appendstringtosvg(psvg, fill);
    appendstringtosvg(psvg, "' r='");
    appendnumbertosvg(psvg, r);
    appendstringtosvg(psvg, "' cy='");
    appendnumbertosvg(psvg, cy);
    appendstringtosvg(psvg, "' cx='");
    appendnumbertosvg(psvg, cx);
    appendstringtosvg(psvg, "' />\n");
}

//----------------------------------------------------------------
// FUNCTION svg_line
//----------------------------------------------------------------
void svg_line(svg* psvg, char* stroke, int strokewidth, int x1, int y1, int x2, int y2)
{
    appendstringtosvg(psvg, "    <line stroke='");
    appendstringtosvg(psvg, stroke);
    appendstringtosvg(psvg, "' stroke-width='");
    appendnumbertosvg(psvg, strokewidth);
    appendstringtosvg(psvg, "px' y2='");
    appendnumbertosvg(psvg, y2);
    appendstringtosvg(psvg, "' x2='");
    appendnumbertosvg(psvg, x2);
    appendstringtosvg(psvg, "' y1='");
    appendnumbertosvg(psvg, y1);
    appendstringtosvg(psvg, "' x1='");
    appendnumbertosvg(psvg, x1);
    appendstringtosvg(psvg, "' />\n");
}

//----------------------------------------------------------------
// FUNCTION svg_rectangle
//----------------------------------------------------------------
void svg_rectangle(svg* psvg, int width, int height, int x, int y, char* fill, char* stroke, int strokewidth, int radiusx, int radiusy)
{
    appendstringtosvg(psvg, "    <rect fill='");
    appendstringtosvg(psvg, fill);
    appendstringtosvg(psvg, "' stroke='");
    appendstringtosvg(psvg, stroke);
    appendstringtosvg(psvg, "' stroke-width='");
    appendnumbertosvg(psvg, strokewidth);
    appendstringtosvg(psvg, "px' width='");
    appendnumbertosvg(psvg, width);
    appendstringtosvg(psvg, "' height='");
    appendnumbertosvg(psvg, height);
    appendstringtosvg(psvg, "' y='");
    appendnumbertosvg(psvg, y);
    appendstringtosvg(psvg, "' x='");
    appendnumbertosvg(psvg, x);
    appendstringtosvg(psvg, "' ry='");
    appendnumbertosvg(psvg, radiusy);
    appendstringtosvg(psvg, "' rx='");
    appendnumbertosvg(psvg, radiusx);
    appendstringtosvg(psvg, "' />\n");
}

// --------------------------------------------------------
// FUNCTION svg_fill
// --------------------------------------------------------
void svg_fill(svg* psvg, char* Fill)
{
    svg_rectangle(psvg, psvg->width, psvg->height, 0, 0, Fill, Fill, 0, 0, 0);
}

//----------------------------------------------------------------
// FUNCTION svg_text
//----------------------------------------------------------------
void svg_text(svg* psvg, int x, int y, char* fontfamily, int fontsize, char* fill, char* stroke, char* text)
{
    appendstringtosvg(psvg, "    <text x='");
    appendnumbertosvg(psvg, x);
    appendstringtosvg(psvg, "' y = '");
    appendnumbertosvg(psvg, y);
    appendstringtosvg(psvg, "' font-family='");
    appendstringtosvg(psvg, fontfamily);
    appendstringtosvg(psvg, "' stroke='");
    appendstringtosvg(psvg, stroke);
    appendstringtosvg(psvg, "' fill='");
    appendstringtosvg(psvg, fill);
    appendstringtosvg(psvg, "' font-size='");
    appendnumbertosvg(psvg, fontsize);
    appendstringtosvg(psvg, "px'>");
    appendstringtosvg(psvg, text);
    appendstringtosvg(psvg, "</text>\n");
}

//----------------------------------------------------------------
// FUNCTION svg_ellipse
//----------------------------------------------------------------
void svg_ellipse(svg* psvg, int cx, int cy, int rx, int ry, char* fill, char* stroke, int strokewidth)
{
    appendstringtosvg(psvg, "    <ellipse cx='");
    appendnumbertosvg(psvg, cx);
    appendstringtosvg(psvg, "' cy='");
    appendnumbertosvg(psvg, cy);
    appendstringtosvg(psvg, "' rx='");
    appendnumbertosvg(psvg, rx);
    appendstringtosvg(psvg, "' ry='");
    appendnumbertosvg(psvg, ry);
    appendstringtosvg(psvg, "' fill='");
    appendstringtosvg(psvg, fill);
    appendstringtosvg(psvg, "' stroke='");
    appendstringtosvg(psvg, stroke);
    appendstringtosvg(psvg, "' stroke-width='");
    appendnumbertosvg(psvg, strokewidth);
    appendstringtosvg(psvg, "' />\n");
}

//this was the main program
// changed to a test function (dph)
void svgTest() {
    puts("-----------------");
    puts("| codedrome.com |");
    puts("| SVG Library   |");
    puts("-----------------\n");

    drawrectangles();

    drawallshapes();

    iwanttobelieve();

    mondrian();

}


// --------------------------------------------------------
// FUNCTION drawrectangles
// --------------------------------------------------------
void drawrectangles(void)
{
    svg* psvg;
    psvg = svg_create(512, 512);

    if(psvg == NULL)
    {
        puts("psvg is NULL");
    }
    else
    {
        svg_rectangle(psvg, 512, 512, 0, 0, "#C0C0FF", "black", 1, 0, 0);

        svg_rectangle(psvg, 384, 384, 64, 64, "#00FF00", "#000000", 4, 0, 0);
        svg_rectangle(psvg, 320, 320, 96, 96, "#FFFF00", "#000000", 2, 8, 8);
        svg_rectangle(psvg, 256, 256, 128, 128, "#00FFFF", "#000000", 2, 8, 8);
        svg_rectangle(psvg, 192, 192, 160, 160, "#FF80FF", "#000000", 2, 8, 8);

        svg_finalize(psvg);
        svg_save(psvg, "rectangles.svg");
        svg_free(psvg);
    }
}

// --------------------------------------------------------
// FUNCTION drawallshapes
// --------------------------------------------------------
static void drawallshapes(void)
{
    svg* psvg;
    psvg = svg_create(192, 320);

    if(psvg == NULL)
    {
        puts("psvg is NULL");
    }
    else
    {
        svg_fill(psvg, "#DADAFF");

        svg_text(psvg, 32, 32, "sans-serif", 16, "#000000", "#000000", "drawallshapes");
        svg_circle(psvg, "#000080", 4, "#0000FF", 32, 64, 96);
        svg_ellipse(psvg, 64, 160, 32, 16, "#FF0000", "#800000", 4);

        svg_line(psvg, "#000000", 2, 32, 192, 160, 192);

        svg_rectangle(psvg, 64, 64, 32, 224, "#00FF00", "#008000", 4, 4, 4);

        svg_finalize(psvg);
        svg_print(psvg);
        svg_save(psvg, "allshapes.svg");
        svg_free(psvg);
    }
}

// --------------------------------------------------------
// FUNCTION iwanttobelieve
// --------------------------------------------------------
static void iwanttobelieve(void)
{
    svg* psvg;
    psvg = svg_create(512, 768);

    if(psvg == NULL)
    {
        puts("psvg is NULL");
    }
    else
    {
        svg_fill(psvg, "#000010");

        srand(time(NULL));
        int x, y;
        for(int s = 0; s <= 512; s++)
        {
            x = (rand() % 512);
            y = (rand() % 768);

            svg_rectangle(psvg, 1, 1, x, y, "white", "white", 0, 0, 0);
        }

        svg_text(psvg, 96, 712, "sans-serif", 32, "#FFFFFF", "#FFFFFF", "I WANT TO BELIEVE");

        svg_circle(psvg, "silver", 1, "rgba(0,0,0,0)", 28, 256, 384);

        svg_ellipse(psvg, 256, 374, 8, 14, "#808080", "#808080", 0);
        svg_ellipse(psvg, 252, 372, 3, 2, "#000000", "#000000", 0);
        svg_ellipse(psvg, 260, 372, 3, 2, "#000000", "#000000", 0);
        svg_rectangle(psvg, 1, 1, 251, 371, "white", "white", 0, 0, 0);
        svg_rectangle(psvg, 1, 1, 259, 371, "white", "white", 0, 0, 0);
        svg_line(psvg, "black", 2, 254, 378, 258, 378);

        svg_line(psvg, "silver", 2, 234, 416, 226, 432);
        svg_line(psvg, "silver", 2, 278, 416, 286, 432);
        svg_ellipse(psvg, 256, 400, 64, 16, "silver", "silver", 4);

        svg_finalize(psvg);
        svg_save(psvg, "iwanttobelieve.svg");
        svg_free(psvg);
    }
}

// --------------------------------------------------------
// FUNCTION mondrian
// --------------------------------------------------------
static void mondrian(void)
{
    svg* psvg;
    psvg = svg_create(512, 512);

    if(psvg == NULL)
    {
        puts("psvg is NULL");
    }
    else
    {
        svg_fill(psvg, "white");

        svg_rectangle(psvg, 512, 512, 0, 0, "white", "black", 1, 0, 0);

        svg_rectangle(psvg, 256, 256, 64, 64, "red", "red", 0, 0, 0);
        svg_rectangle(psvg, 128, 128, 64, 320, "black", "black", 0, 0, 0);
        svg_rectangle(psvg, 64, 128, 0, 384, "orange", "orange", 0, 0, 0);
        svg_rectangle(psvg, 128, 192, 320, 0, "orange", "orange", 0, 0, 0);
        svg_rectangle(psvg, 128, 64, 320, 384, "navy", "navy", 0, 0, 0);
        svg_rectangle(psvg, 64, 128, 448, 384, "red", "red", 0, 0, 0);

        svg_line(psvg, "black", 8, 0, 64, 448, 64);
        svg_line(psvg, "black", 8, 64, 64, 64, 512);
        svg_line(psvg, "black", 8, 0, 192, 64, 192);
        svg_line(psvg, "black", 8, 0, 384, 512, 384);
        svg_line(psvg, "black", 8, 128, 0, 128, 64);
        svg_line(psvg, "black", 8, 320, 0, 320, 448);
        svg_line(psvg, "black", 8, 64, 320, 448, 320);
        svg_line(psvg, "black", 8, 320, 192, 448, 192);
        svg_line(psvg, "black", 8, 64, 448, 448, 448);
        svg_line(psvg, "black", 8, 448, 0, 448, 512);
        svg_line(psvg, "black", 8, 192, 320, 192, 512);
        svg_line(psvg, "black", 8, 384, 192, 384, 320);

        svg_finalize(psvg);
        svg_save(psvg, "mondrian.svg");
        svg_free(psvg);
    }
}




