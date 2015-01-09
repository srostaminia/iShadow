clear all; close all;
graphics_toolkit('fltk')
addpath('/Users/ammayber/senseye/ann_model');

load('/Users/ammayber/senseye/ann_model/results_addison2/subset_l1_init_strips_k7_lambda0.010000/rep3.mat')

eye_img=double(load(sprintf('~/Desktop/eye_exper/usb_frame.txt')));

base_img=eye_img;
eye_min = min(eye_img(:));
eye_max = max(eye_img(:));

Mask = load('~/Desktop/eye_exper/rep3_0.01_addison2/nn_mask.txt');

eye_img -= min(eye_img(:));
eye_img /= max(eye_img(:));
eye_img=eye_img(2:end,:);
% eye_img=reshape(eye_img,[1 111*112]);
eye_img=fliplr(eye_img);

keyboard;

pred=[112,111].*logisticmlp_prediction(these_results.W, [reshape(eye_img,[1, 112*111]),1], 7, 2);
pred(1) = 112 - pred(1);
pred