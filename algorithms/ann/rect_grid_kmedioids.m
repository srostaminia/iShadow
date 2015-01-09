function ind = rect_grid_kmedioids(dims,K)

S     = prod(dims);
[x,y] = meshgrid(1:dims(1),1:dims(2));
D     = [x(:);y(:)];
D     = reshape(D,[S,1,2]);
D     = sqrt(sum(bsxfun(@minus,D,reshape(D,[1,S,2])).^2,3));

[foo,ind,energy] = kmedioids(D,K);
ind = [ind(:);prod(dims)+1];

%Mask = zeros(S,1);
%Mask(cidx)=1;
%Mask = reshape(Mask,N,N);
