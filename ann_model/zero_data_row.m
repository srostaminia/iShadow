clear all; close all;
graphics_toolkit('fltk')

load('eye_data_auto_rebuild')

new_Out = []
for i = 1:size(Out,1)
	img = reshape(Out(i,:), [111,112]);

	img = [zeros(1, 112); img];

	img = reshape(img, [1, 12544]);

	new_Out = [new_Out; img];

	if mod(i,100) == 0
		i
	end
end

Out = new_Out;

save('-V7', 'eye_data_auto_rebuild2.mat', 'Out', 'X', 'gout', 'template');