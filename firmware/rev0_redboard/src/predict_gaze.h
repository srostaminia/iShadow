/*
 * File:   gaze_predict.h
 * Author: Addison
 *
 * Created on April 9, 2013, 11:05 AM
 */

#ifndef GAZE_PREDICT_H
#define	GAZE_PREDICT_H

#ifdef	__cplusplus
extern "C" {
#endif

#define NUM_SUBSAMPLE 27
#define NUM_HIDDEN 6

void predict_gaze();

float tanh_approx(float input);

#ifdef	__cplusplus
}
#endif

#endif	/* GAZE_PREDICT_H */

