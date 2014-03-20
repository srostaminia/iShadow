clear all; close all;
graphics_toolkit('fltk')
addpath('/Users/ammayber/senseye/ann_model');

load('/Users/ammayber/senseye/ann_model/results_mean/subset_l1_init_strips_k7_lambda0.010000/rep1.mat')

pred_list = []
% for i=1
for i=1:50
	eye_img=double(load(sprintf('test_eye_%06d.txt',i-1)));
	out_img=double(imread(sprintf('test_img_%06d.png',i-1)));

	eye_img=reshape(eye_img,[1 111*112]);

	% eye_img=double(load(sprintf('test_eye.txt')));
	% out_img=double(imread(sprintf('test_out.png')));

	% eye_img=reshape(eye_img(2:end,:),[1 111*112]);

	pred=[112,111].*logisticmlp_prediction(these_results.W, [reshape(eye_img,[1, 112*111]),1], 7, 2);
	pred_list = [pred_list; pred];

	imagesc(out_img);
	colormap('gray');
	hold on;
	plot(pred(1),pred(2),'r+','markersize',20,'linewidth',3);
	axis off;
	fname = sprintf('prediction_images/prediction_%06d.png',i-1);
    print(fname,'-dpng');
    hold off;
    close all;
end

pred_list