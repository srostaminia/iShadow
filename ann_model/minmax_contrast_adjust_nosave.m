function [X] = minmax_contrast_adjust_nosave(X)

	% for i = 1:size(X,1)
	% 	X(i,:) -= min(X(i,:));
	% 	X(i,:) /= max(X(i,:));
	% end

	X=bsxfun(@Minus,X,min(X,[],2));
	X ./= max(X,[],2);
