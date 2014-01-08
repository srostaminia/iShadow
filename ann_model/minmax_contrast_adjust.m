clear all; close all;
graphics_toolkit('fltk')

load('eye_data_addison2_auto')

new_X = []
for i = 1:size(X,1)
	img = reshape(X(i,:), [111,112]);

	img -= min(img(:));
	img /= max(img(:));

	img = reshape(img, [1, 111*112]);

	new_X = [new_X; img];

	if mod(i,100) == 0
		i
	end
end

X = new_X;

save('-V7', 'eye_data_addison2_auto_minmax.mat', 'Out', 'X', 'gout', 'template');