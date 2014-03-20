clear all; close all;
graphics_toolkit('fltk')

pred=load('test_pred.txt');

for i=1:size(pred,1)
	out_img=double(imread(sprintf('test_img_%06d.png',i-1)));

	imagesc(out_img);
	colormap('gray');
	hold on;
	plot(pred(i,1),pred(i,2),'r+','markersize',20,'linewidth',3);
	axis off;
	fname = sprintf('prediction_images/prediction_%06d.png',i);
    print(fname,'-dpng');
    hold off;
    close all;
end