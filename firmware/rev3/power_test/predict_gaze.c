#include "predict_gaze.h"
#include "diskio.h"
#include "stm32l152d_eval_sdio_sd.h"
#include "predict_gaze.h"
#include "math.h"

extern unsigned short val[112*112];
int8_t pred[2];
float pred_radius;

unsigned short num_subsample;
unsigned short num_hidden;
unsigned int bh_offset;
unsigned int bo_offset;
unsigned int mask_offset;
unsigned int who_offset;
unsigned int wih_offset;
unsigned int fpn_offset;
unsigned int col_fpn_offset;

extern unsigned short model_data[];
extern uint32_t sd_ptr;

//extern float bh[NUM_HIDDEN];
//extern float bo[2];
//extern unsigned short mask[NUM_SUBSAMPLE][2];
//extern float who[NUM_HIDDEN][2];
//extern float wih[NUM_SUBSAMPLE][NUM_HIDDEN];

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

void predict_gaze(uint16_t subsamples[], uint16_t min, uint16_t max)
{
    int i, j;
    float ah[6];
    float x, x_val, y_val;

    for (i = 0; i < NUM_HIDDEN; i++)  {
        ah[i] = BH(i);
    }

    for (i = 0; i < NUM_SUBSAMPLE; i++) {
        // val = global containing subsampled pixel data
//        int temp = subsamples[i];
        x = (float)(subsamples[i] - min) / max;

        for (j = 0; j < NUM_HIDDEN; j++) {
            ah[j] += x * WIH(i, j);
        }
    }
    
    x_val = BO(0);
    y_val = BO(1);

    for (i = 0; i < NUM_HIDDEN; i++) {
        x_val += WHO(i, 0) * tanh_approx(ah[i]);
        y_val += WHO(i, 1) * tanh_approx(ah[i]);
    }

    // pred = global for storing prediction values
    pred[0] = (unsigned short)((x_val * 112) + 0.5);
    pred[1] = (unsigned short)((y_val * 111) + 0.5);
    
    //    f_finish_write();
//    if (disk_write_fast(0, (uint8_t *)pred, sd_ptr, 1) != RES_OK)
//      while(1);
//    sd_ptr += 1;
//    f_finish_write();
    
    return;
}

void predict_gaze_fullimg(uint16_t img[], uint16_t min, uint16_t max)
{
    int i, j;
    float ah[6];
    float x, x_val, y_val;

    for (i = 0; i < NUM_HIDDEN; i++)  {
        ah[i] = BH(i);
    }

    uint16_t subsample;
    for (i = 0; i < NUM_SUBSAMPLE; i++) {
        // val = global containing subsampled pixel data
//        int temp = subsamples[i];
        subsample = img[(MASK(i, 0) * 112) + MASK(i, 1)];
//        x = (float)(subsample - min) / (max - min);
        x = (float)(subsample - min) / (max - min);

        for (j = 0; j < NUM_HIDDEN; j++) {
            ah[j] += x * WIH(i, j);
        }
    }
    
    x_val = BO(0);
    y_val = BO(1);

    for (i = 0; i < NUM_HIDDEN; i++) {
        x_val += WHO(i, 0) * tanh_approx(ah[i]);
        y_val += WHO(i, 1) * tanh_approx(ah[i]);
    }
    
    // pred = global for storing prediction values
    pred[0] = (unsigned short)((x_val * 112) + 0.5);
    pred[1] = (unsigned short)((y_val * 111) + 0.5);
    
    //    f_finish_write();
//    if (disk_write_fast(0, (uint8_t *)pred, sd_ptr, 1) != RES_OK)
//      while(1);
//    sd_ptr += 1;
//    f_finish_write();
    
    return;
}

void predict_gaze_fullmean(uint16_t img[], float mean, float std)
{
    int i, j;
    float ah[6];
    float x, x_val, y_val;

    for (i = 0; i < NUM_HIDDEN; i++)  {
        ah[i] = BH(i);
    }
    
//    float std2 = calc_std(img);
//    float mean = ((float)sum) / (112*112);

    uint16_t subsample;
    for (i = 0; i < NUM_SUBSAMPLE; i++) {
        // val = global containing subsampled pixel data
//        int temp = subsamples[i];
        subsample = img[(MASK(i, 0) * 112) + MASK(i, 1)];
        x = (float)(subsample - mean) / std;

        for (j = 0; j < NUM_HIDDEN; j++) {
            ah[j] += x * WIH(i, j);
        }
    }
    
    x_val = BO(0);
    y_val = BO(1);

    for (i = 0; i < NUM_HIDDEN; i++) {
        x_val += WHO(i, 0) * tanh_approx(ah[i]);
        y_val += WHO(i, 1) * tanh_approx(ah[i]);
    }
    
    // pred = global for storing prediction values
    pred[0] = (unsigned short)((x_val * 112) + 0.5);
    pred[1] = (unsigned short)((y_val * 111) + 0.5);
    
    //    f_finish_write();
//    if (disk_write_fast(0, (uint8_t *)pred, sd_ptr, 1) != RES_OK)
//      while(1);
//    sd_ptr += 1;
//    f_finish_write();
    
    return;
}

float calc_std(uint16_t img[])
{
    float M = 0.0;
    float S = 0.0;
    int k = 1;
    for (int i = 0; i < (112 * 112); i++) {
      float value = (float)img[i];
      float tmpM = M;
      M += (value - tmpM) / k;
      S += (value - tmpM) * (value - M);
      k++;
    }
    float temp_res = sqrt(S / (k-1));
    return sqrt(S / (k-1));
}

int finish_predict(float ah[6])
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

    x_val = BO(0);
    y_val = BO(1);

    for (int i = 0; i < NUM_HIDDEN; i++) {
        x_val += WHO(i, 0) * tanh_approx(ah[i]);
        y_val += WHO(i, 1) * tanh_approx(ah[i]);
    }
    
    // pred = global for storing prediction values
    pred[0] = (unsigned short)((x_val * 112) + 0.5);
    pred[1] = (unsigned short)((y_val * 111) + 0.5);
      
//    f_finish_write();
//    if (disk_write_fast(0, (uint8_t*)pred, sd_ptr, 1) != RES_OK)      return -1;
//    sd_ptr += 1;
//    f_finish_write();
    
    return 0;
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
