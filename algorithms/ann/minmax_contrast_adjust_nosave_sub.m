function [X] = minmax_contrast_adjust_nosave(X,ind)

	% for i = 1:size(X,1)
	% 	X(i,:) -= min(X(i,:));
	% 	X(i,:) /= max(X(i,:));
	% end

	X=bsxfun(@minus,X,min(X(:,ind),[],2));
	X=bsxfun(@rdivide,X,max(X(:,ind),[],2));
