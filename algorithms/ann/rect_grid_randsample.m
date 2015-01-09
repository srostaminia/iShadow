function ind = rect_grid_randsample(dims,K)

S   = prod(dims);
ind = randperm(S);
ind = ind(1:K);
ind = [ind(:);prod(dims)+1];
%Mask = zeros(S,1);
%Mask(ind)=1;
%Mask = reshape(Mask,N,N);
