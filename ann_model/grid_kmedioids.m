function ind = grid_kmedioids(N,K)

S     = N*N;
[x,y] = meshgrid(1:N,1:N);
D     = [x(:);y(:)];
D     = reshape(D,[S,1,2]);
D     = sqrt(sum(bsxfun(@minus,D,reshape(D,[1,S,2])).^2,3));

[foo,ind,energy] = kmedioids(D,K);
ind = [ind(:);N*N+1];

%Mask = zeros(S,1);
%Mask(cidx)=1;
%Mask = reshape(Mask,N,N);
