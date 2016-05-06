function [X] = mean_contrast_adjust(X)

	% for i = 1:size(X,1)
	% 	X(i,:) -= mean(X(i,:));
	% 	X(i,:) /= std(X(i,:));
	% end

	X=bsxfun(@minus,X,mean(X,2));
	X=bsxfun(@rdivide,X,std(X,0,2));