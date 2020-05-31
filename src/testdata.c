//
// Created by David Heddle on 5/27/20.
//

#include "testdata.h"
double torusNN[33][6] = {
        {496.225, 8.5994, 264.179,   3.13083e-18, 0.00496269, 1.6076e-17},
        {35.7457, 496.431, 499.971,   0.000185855, -0.00197376, -0.000705167},
        {159.022, 388.459, 306.278,   -0.0174959, 0.0128095, 0.000542118},
        {341.313, 171.239, 275.274,   -0.0139845, -0.0666581, 0.012125},
        {140.612, 58.5985, 368.63,   -0.342639, 0.686831, -0.0648507},
        {334.661, 239.04, 252.056,   -0.0106836, -0.0256641, 0.00765755},
        {145.925, 251.197, 299.973,   7.19425e-18, 0.151853, -4.40537e-17},
        {431.29, 232.83, 527.776,   0.000536185, -0.0015445, -0.000317776},
        {53.312, 338.198, 138.065,   -0.00284981, -0.0125471, 0.0154393},
        {240.461, 376.897, 219.262,   -0.00204967, 0.00843449, 0.0008723},
        {362.697, 175.685, 585.088,   0.000476148, -0.00145595, -0.000737294},
        {84.6628, 469.569, 222.828,   -0.00473728, -0.00461568, 0.00268136},
        {54.9528, 395.486, 535.359,   -0.000321198, -0.00278942, -0.00299359},
        {32.779, 249.869, 546.559,   0.0023272, -0.00483037, -0.00654778},
        {67.6333, 290.384, 106.685,   -0.00138529, -0.000476482, 0.0143488},
        {134.167, 272.762, 193.851,   -0.0139352, 0.0525363, 0.0166309},
        {134.116, 197.083, 416.29,   -0.0277987, 0.140604, -0.0361505},
        {29.6759, 460.195, 238.084,   -0.000461795, -0.00956103, 0.00177496},
        {29.2697, 413.947, 521.362,   0.000754557, -0.00373464, -0.00171003},
        {80.4731, 234.619, 483.998,   -0.00900032, 0.0141529, -0.0357867},
        {190.112, 263.89, 557.21,   -0.000753205, 0.00352481, -0.00272293},
        {1.2454, 8.6383, 429.798,   0.0381581, -0.0137955, -0.00265154},
        {201.851, 418.178, 552.956,   -0.000461119, 0.0013911, -0.000604042},
        {36.1727, 179.856, 506.01,   0.00364127, -0.00705238, -0.0311974},
        {60.0228, 160.559, 297.228,   -0.111066, 0.530351, 0.0628344},
        {125.535, 223.825, 374.389,   -2.90434e-17, 0.186396, 1.93623e-17},
        {435.529, 240.783, 216.499,   0.00111699, -0.00436352, 0.000476382},
        {259.091, 118.064, 442.853,   0.00127204, -0.0661337, -0.071331},
        {114.992, 73.3266, 597.031,   0.000490728, -0.00069606, -0.000147822},
        {124.332, 172.3, 428.433,   -0.0403489, 0.150592, -0.0641344},
        {6.7452, 273.481, 435.941,   0.051118, -0.12767, -0.045554},
        {394.045, 14.1724, 179.829,   -0.00259152, 0.0131244, 0.00201429},
        {300.868, 383.929, 385.222,   -0.00464421, 0.00309322, -0.00148602}};

double solenoidNN[34][6] = { //GAUSS
		{21.0317, 244.232, 95.2729,   -1.21889,  -14.1544,  -1.41521},
		{174.432, 196.567, -263.224,   -3.20192,  -3.60823,  -3.27755},
		{288.243, 7.8778, -82.5142,   6.46423,  0.17667,  0.282259},
		{105.14, 181.542, -84.2208,   15.927,  27.5006,  -0.327652},
		{16.576, 148.955, 176.399,   4.42786,  39.7896,  -43.2307},
		{72.9677, 230.268, -244.82,   -1.95093,  -6.15664,  -5.06388},
		{246.484, 81.8586, -1.737,   0.96789,  0.321441,  9.91855},
		{251.295, 5.453, -245.891,   -5.41577,  -0.11752,  -4.66959},
		{210.474, 146.127, 275.894,   4.203,  2.91803,  -2.52403},
		{195.531, 213.615, 7.4519,   -0.236798,  -0.258699,  5.47413},
		{83.8357, 23.184, 123.292,   657.595,  181.852,  -439.556},
		{105.841, 134.876, -78.5556,   51.3912,  65.4891,  9.14706},
		{11.743, 291.14, -148.5,   0.132803,  3.29254,  -4.4603},
		{48.2771, 110.934, -114.821,   34.6511,  79.623,  -329.556},
		{131.855, 97.759, -49.7778,   46.9646,  34.8202,  59.0092},
		{220.196, 115.514, 266.187,   5.0157,  2.63122,  -3.17758},
    	{73.0324, 5.5785, 178.748,   263.64,  20.1379,  136.897},
		{256.495, 153.741, -156.435,   2.08414,  1.24922,  -4.16564},
		{36.172, 72.2379, -226.888,   -37.0557,  -74.0027,  64.6766},
		{39.39, 39.9497, 271.879,   20.1851,  20.4719,  51.8233},
		{24.1363, 138.98, 205.722,   8.55665,  49.2703,  -13.3516},
		{264.531, 31.4866, -270.127,   -4.63366,  -0.551535,  -2.84764},
		{54.0208, 55.563, -1.2278,   411.473,  423.22,  -23869.1},
		{158.14, 96.7593, 160.347,   0.0603408,  0.0369201,  -33.0466},
		{105.24, 100.995, 17.3683,   4.14226,  3.97518,  68.608},
		{152.66, 164.654, 186.291,   1.51438,  1.63335,  -13.0138},
		{117.994, 168.875, -88.3551,   19.7803,  28.3099,  -3.28007},
		{272.064, 35.7897, -146.729,   4.35926,  0.573456,  -5.91188},
		{152.263, 152.57, 285.83,   6.24651,  6.25913,  -1.27165},
		{63.5133, 192.669, 298.619,   3.00884,  9.12737,  0.240413},
		{141.172, 16.2907, 220.072,   42.9365,  4.9547,  -4.74594},
		{177.99, 38.3277, 23.1713,   -13.176,  -2.83727,  43.3687},
		{5.788, 178.128, 259.632,   0.561831,  17.2906,  -1.51619},
		{241.583, 155.961, -204.312,   -0.754936,  -0.48737,  -4.91092}};