function [X] = mean_contrast_adjust_nosave(X)

	for i = 1:size(X,1)
		X(i,:) -= mean(X(i,:));
		X(i,:) /= std(X(i,:));
	end