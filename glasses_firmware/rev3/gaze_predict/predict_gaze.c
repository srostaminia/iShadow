#include "predict_gaze.h"

extern unsigned short val[112*112];
volatile unsigned short pred[2];

extern float bh[NUM_HIDDEN];
extern float bo[NUM_HIDDEN];
extern unsigned short mask[NUM_SUBSAMPLE][2];
extern float who[NUM_HIDDEN][2];
extern float wih[NUM_SUBSAMPLE][NUM_HIDDEN];

float tanh_values[] = {
-0.999909204263 ,
-0.999889102951 ,
-0.999864551701 ,
-0.999834565554 ,
-0.999797941612 ,
-0.999753210848 ,
-0.999698579284 ,
-0.99963185619 ,
-0.99955036646 ,
-0.999450843688 ,
-0.999329299739 ,
-0.99918086567 ,
-0.998999597786 ,
-0.998778241281 ,
-0.998507942332 ,
-0.998177897611 ,
-0.997774927934 ,
-0.997282960099 ,
-0.99668239784 ,
-0.995949359222 ,
-0.995054753687 ,
-0.993963167351 ,
-0.992631520201 ,
-0.991007453678 ,
-0.989027402201 ,
-0.986614298151 ,
-0.983674857694 ,
-0.980096396266 ,
-0.975743130031 ,
-0.970451936613 ,
-0.964027580076 ,
-0.956237458128 ,
-0.946806012846 ,
-0.935409070603 ,
-0.921668554406 ,
-0.905148253645 ,
-0.885351648202 ,
-0.861723159313 ,
-0.833654607012 ,
-0.800499021761 ,
-0.761594155956 ,
-0.716297870199 ,
-0.664036770268 ,
-0.604367777117 ,
-0.537049566998 ,
-0.46211715726 ,
-0.379948962255 ,
-0.291312612452 ,
-0.197375320225 ,
-0.099667994625 ,
0.0 ,
0.099667994625 ,
0.197375320225 ,
0.291312612452 ,
0.379948962255 ,
0.46211715726 ,
0.537049566998 ,
0.604367777117 ,
0.664036770268 ,
0.716297870199 ,
0.761594155956 ,
0.800499021761 ,
0.833654607012 ,
0.861723159313 ,
0.885351648202 ,
0.905148253645 ,
0.921668554406 ,
0.935409070603 ,
0.946806012846 ,
0.956237458128 ,
0.964027580076 ,
0.970451936613 ,
0.975743130031 ,
0.980096396266 ,
0.983674857694 ,
0.986614298151 ,
0.989027402201 ,
0.991007453678 ,
0.992631520201 ,
0.993963167351 ,
0.995054753687 ,
0.995949359222 ,
0.99668239784 ,
0.997282960099 ,
0.997774927934 ,
0.998177897611 ,
0.998507942332 ,
0.998778241281 ,
0.998999597786 ,
0.99918086567 ,
0.999329299739 ,
0.999450843688 ,
0.99955036646 ,
0.99963185619 ,
0.999698579284 ,
0.999753210848 ,
0.999797941612 ,
0.999834565554 ,
0.999864551701 ,
0.999889102951 ,
0.999909204263 ,
};

void predict_gaze()
{
    int i, j;
    float ah[6];
    float x, x_val, y_val;

    for (i = 0; i < NUM_HIDDEN; i++)  {
        ah[i] = bh[i] / 255;
    }

    for (i = 0; i < NUM_SUBSAMPLE; i++) {
        // val = global containing subsampled pixel data
        x = (float)(val[i]) / 255;

        for (j = 0; j < NUM_HIDDEN; j++) {
            ah[j] += x * wih[i][j];
        }
    }

    x_val = bo[0];
    y_val = bo[1];

    for (i = 0; i < NUM_HIDDEN; i++) {
        x_val += who[i][0] * tanh_approx(ah[i]);
        y_val += who[i][1] * tanh_approx(ah[i]);
    }

    // pred = global for storing prediction values
    pred[0] = (unsigned short)((x_val * 112) + 0.5);
    pred[1] = (unsigned short)((y_val * 112) + 0.5);
}

void finish_predict(float ah[6])
{
//    float ah[6];
    float x_val, y_val;

//    for (i = 0; i < NUM_HIDDEN; i++)  {
//        ah[i] = bh[i] / 255;
//    }

//    for (i = 0; i < NUM_SUBSAMPLE; i++) {
//        // val = global containing subsampled pixel data
//        x = (float)(val[i]) / 255;
//
//        for (j = 0; j < NUM_HIDDEN; j++) {
//            ah[j] += x * wih[i][j];
//        }
//    }

    x_val = bo[0];
    y_val = bo[1];

    for (int i = 0; i < NUM_HIDDEN; i++) {
        x_val += who[i][0] * tanh_approx(ah[i]);
        y_val += who[i][1] * tanh_approx(ah[i]);
    }

    // pred = global for storing prediction values
    pred[0] = (unsigned short)((x_val * 112) + 0.5);
    pred[1] = (unsigned short)((y_val * 112) + 0.5);
}

float tanh_approx(float input)
{
    if (input <= -5)
        return -1;
    else if (input >= 5)
        return 1;

    // Offset input value
    input += 5;

    // Each index corresponds to 0.1 in the range from -5 to 5 (0 to 10 after offset)
    int lower_index = (int)(input * 10);
    int upper_index = lower_index + 1;

    // Just in case precision or rounding issues push us into the clipped range
    if (lower_index < 0)
        return -1;
    if (lower_index >= 99)
        return 1;

    float x0 = (float)(lower_index) / 10, x1 = (float)(upper_index) / 10;
    float y0 = tanh_values[lower_index], y1 = tanh_values[upper_index];

    float approx = y0 + (y1 - y0) * ((input - x0) / (x1 - x0));

    return approx;
}
