clear all; close all;
graphics_toolkit('fltk')

gaze=load('first_pred.txt');

i = 0
for i = 1:size(gaze,1)
	fname = sprintf('first_img_%06d.png',i)
	if(exist(fname))
		img = imread(fname);

		figure();
		hold off;
		imagesc(img);
		hold on;
		plot(gaze(i,1),gaze(i,2),'r+','markersize',20,'linewidth',3);
		axis off;
		colormap('gray');

		print(fname,'-dpng');

		close;

		i = i + 1
	end
end