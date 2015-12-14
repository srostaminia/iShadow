#include "eye_models.h"
#include "stonyman.h"
#include "diskio.h"
#include "stm32l152d_eval_sdio_sd.h"
#include "math.h"

extern unsigned short val[112*112];
int8_t pred[2];
float pred_radius;

unsigned short num_subsample = 0;
unsigned short num_hidden = 0;
unsigned int bh_offset = 0;
unsigned int bo_offset = 0;
unsigned int mask_offset = 0;
unsigned int who_offset = 0;
unsigned int wih_offset = 0;
unsigned int fpn_offset = 0;
unsigned int col_fpn_offset = 0;

extern uint16_t model_data[];
extern uint32_t sd_ptr;

// FIXME: REMOVE THIS!!
extern uint16_t subsampled[2273];

//uint16_t test_data[] = { 358 , 392 , 338 , 426 , 349 , 329 , 330 , 435 , 483 , 376 , 362 , 488 , 463 , 318 , 481 , 434 , 482 , 441 , 345 , 422 , 374 , 469 , 400 , 383 , 433 , 435 , 462 , 319 , 433 , 370 , 346 , 432 , 341 , 334 , 347 , 340 , 383 , 369 , 493 , 484 , 322 , 477 , 333 , 429 , 495 , 306 , 327 , 487 , 387 , 313 , 405 , 379 , 300 , 428 , 470 , 463 , 441 , 419 , 327 , 406 , 494 , 475 , 434 , 437 , 327 , 418 , 449 , 303 , 389 , 421 , 470 , 495 , 482 , 477 , 332 , 411 , 448 , 328 , 405 , 316 , 445 , 385 , 306 , 374 , 370 , 436 , 334 , 338 , 488 , 353 , 441 , 330 , 481 , 471 , 421 , 394 , 396 , 432 , 467 , 373 , 486 , 363 , 445 , 400 , 339 , 463 , 498 , 312 , 367 , 376 , 366 , 359 , 314 , 428 , 383 , 369 , 473 , 482 , 425 , 308 , 479 , 392 , 427 , 312 , 392 , 323 , 498 , 404 , 428 , 494 , 494 , 489 , 307 , 494 , 366 , 472 , 326 , 422 , 355 , 307 , 310 , 378 , 486 , 321 , 471 , 418 , 438 , 332 , 395 , 483 , 387 , 405 , 436 , 333 , 376 , 428 , 355 , 427 , 424 , 497 , 484 , 433 , 366 , 494 , 474 , 333 , 493 , 411 , 306 , 332 , 382 , 475 , 390 , 468 , 329 , 306 , 489 , 483 , 374 , 369 , 460 , 383 , 421 , 413 , 465 , 371 , 474 , 398 , 343 , 307 , 451 , 366 , 430 , 410 , 451 , 403 , 364 , 391 , 451 , 479 , 447 , 369 , 419 , 383 , 349 , 438 , 453 , 300 , 352 , 374 , 414 , 493 , 407 , 354 , 457 , 359 , 336 , 439 , 412 , 444 , 311 , 309 , 328 , 305 , 413 , 339 , 486 , 451 , 459 , 467 , 451 , 488 , 493 , 336 , 336 , 428 , 346 , 389 , 413 , 491 , 335 , 326 , 335 , 408 , 453 , 371 , 378 , 498 , 484 , 458 , 496 , 473 , 461 , 303 , 429 , 378 , 382 , 419 , 422 , 457 , 374 , 385 , 431 , 307 , 391 , 445 , 317 , 333 , 413 , 474 , 304 , 383 , 395 , 451 , 438 , 346 , 450 , 403 , 448 , 387 , 384 , 423 , 405 , 372 , 372 , 356 , 441 , 426 , 378 , 410 , 481 , 358 , 500 , 320 , 416 , 364 , 392 , 405 , 362 , 363 };

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

// Commenting this out until we need it for something
//static float calc_std(uint16_t img[])
//{
//    float M = 0.0;
//    float S = 0.0;
//    int k = 1;
//    for (int i = 0; i < (112 * 112); i++) {
//      float value = (float)img[i];
//      float tmpM = M;
//      M += (value - tmpM) / k;
//      S += (value - tmpM) * (value - M);
//      k++;
//    }
//    float temp_res = sqrt(S / (k-1));
//    return sqrt(S / (k-1));
//}

static float tanh_approx(float input)
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

#ifdef USE_PARAM_FILE
void read_cider_params()
{
    num_subsample = model_data[PARAMS_START];
    num_hidden = model_data[PARAMS_START + 1];

    if (num_subsample == 0 || num_hidden == 0)      return;

    bh_offset = PARAMS_START + 2;
    bo_offset = bh_offset + num_hidden * 2;
    mask_offset = bo_offset + 4;
    who_offset = mask_offset + num_subsample * 2;
    wih_offset = who_offset + num_hidden * 2 * 2;
}
#endif // USE_PARAM_FILE

inline void init_streamstats(StreamStats *ss)
{
    ss->pixel_sum = 0;
    ss->M = 0.0;
    ss->S = 0.0;
    ss->k = 1;
    ss->current_subsample = 0;
}

inline void update_streamstats(StreamStats *ss, uint16_t *pixels, uint16_t this_pixel, int i_major, int j_minor)
{
    float value, tmpM;

    // Streaming mean / stdev computation on subsampled pixels
    if (MASK(ss->current_subsample, MASK_MAJOR) == i_major && 
        MASK(ss->current_subsample, MASK_MINOR) == j_minor)
    {
        pixels[ss->current_subsample] = this_pixel;
        
        if (ss->current_subsample == 0)
          tmpM = 0;

        ss->pixel_sum += this_pixel;

        // Standard deviation computation
        value = (float)this_pixel;
        tmpM = ss->M;
        ss->M += (value - tmpM) / ss->k;
        ss->S += (value - tmpM) * (value - ss->M);
        ss->k++;
        ss->current_subsample++;
    }
}

inline void update_streamstats2(StreamStats *ss, uint16_t *pixels, uint16_t this_pixel)
{
    float value, tmpM;

    // Streaming mean / stdev computation on subsampled pixels
    pixels[ss->current_subsample] = this_pixel;

    ss->pixel_sum += this_pixel;

    // Standard deviation computation
    value = (float)this_pixel;
    tmpM = ss->M;
    ss->M += (value - tmpM) / ss->k;
    ss->S += (value - tmpM) * (value - ss->M);
    ss->k++;
    ss->current_subsample++;
}

void ann_predict(uint16_t *pixels, StreamStats *ss)
{
    int i, j;
    float ah[6];
    float x, x_val, y_val;

    //TODO FIXME: UNCOMMENT!
    float mean = (float)ss->pixel_sum / num_subsample;
//    float mean = (float)ss->pixel_sum / 300;
    float std = sqrt(ss->S / (ss->k-1));

    for (i = 0; i < num_hidden; i++)  {
        ah[i] = BH(i);
    }

    //    float std2 = calc_std(img);
    //    float mean = ((float)sum) / (112*112);

    uint16_t subsample;
    for (i = 0; i < num_subsample; i++) {
//    for (i = 0; i < 300; i++) {
        subsample = pixels[i];
        x = (float)(subsample - mean) / std;

        for (j = 0; j < num_hidden; j++) {
            float wih_tmp = WIH(i, j);
            ah[j] += x * wih_tmp;
        }
    }

    x_val = BO(0);
    y_val = BO(1);

    for (i = 0; i < num_hidden; i++) {
        x_val += WHO(i, 0) * tanh_approx(ah[i]);
        y_val += WHO(i, 1) * tanh_approx(ah[i]);
    }

    // pred = global for storing prediction values
    pred[PRED_X] = (unsigned short)((x_val * 112) + 0.5);
    pred[PRED_Y] = (unsigned short)((y_val * 111) + 0.5);

    return;
}
