clear all; close all;
graphics_toolkit('fltk')
addpath('/Users/ammayber/senseye/ann_model');

load('/Users/ammayber/senseye/ann_model/results/subset_l1_init_strips_k7_lambda0.010000/rep1.mat')

eye_img=double(load(sprintf('single_test.txt')));
eye_img=eye_img(2:end,:);
eye_img=reshape(eye_img,[1 111*112]);

pred=[112,111].*logisticmlp_prediction(these_results.W, [reshape(eye_img,[1, 112*111]),1], 7, 2)