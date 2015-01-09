function ind = grid_randsample(N,K)

S   = N*N;
ind = randperm(S);
ind = ind(1:K);
ind = [ind(:);N*N+1];
%Mask = zeros(S,1);
%Mask(ind)=1;
%Mask = reshape(Mask,N,N);
