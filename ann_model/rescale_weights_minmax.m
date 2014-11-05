function [X] = rescale_weights_mean(X,W_groupSparse,params)

	nHidden = params.nHidden;
	HiddenShape = params.hiddenShape;
	% nVars = 12545;
	nVars = 12433;
	nTargets = 2;

	%Extract parameters
	count = nVars*nHidden;
	Wih = reshape(W_groupSparse(1:count),[nVars,nHidden]);
	mask = reshape(sum(abs(Wih(1:end-1,:)),2)>0,[111,112]);
	nnz(mask)

	%[col,row,val]=find(mask);
	%maskL   = [row col];
	%player1 = Wih(2:end, 1:end-1);
	%player1 = player1(:,mask(:)==1); 

	[col,row,val]=find(mask');
	maskL   = [row,col];

	sub_pixels = [];
	for j=1:size(maskL,1)
		% pix_val = X(i,:)(sub2ind([111, 112], maskL(j,1), maskL(j, 2)));

		if mod(j,500) == 0
			disp(j)
		end

		sub_pixels(:,j) = X(:,sub2ind([111, 112], maskL(j,1), maskL(j, 2)));

		% img_data = [img_data, pix_val];
	end

	sub_pixels -= min(sub_pixels, 2);
	sub_pixels ./= max(sub_pixels, 0, 2);

	% X(i,1:end-1) = zeros(size(X(i,1:end-1)));
	X = zeros([size(X,1), size(X,2) - 1]);

	for j=1:size(maskL,1)
		% X(i,sub2ind([111, 112], maskL(j,1), maskL(j, 2))) = img_data(pix_idx);
		idx = sub2ind([111, 112], maskL(j,1), maskL(j, 2));
		X(:,idx) = sub_pixels(:,j);

		if mod(j,500) == 0
			disp(j)
		end
	end

	X = [X, ones(size(X,1),1)];
end