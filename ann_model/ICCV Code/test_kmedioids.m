graphics_toolkit('fltk')

N     = 112*112;
[x,y] = meshgrid(1:112,1:112);
D     = [x(:);y(:)];
D     = reshape(D,[N,1,2]);
D     = sqrt(sum(bsxfun(@minus,D,reshape(D,[1,N,2])).^2,3));

best_energy = inf;
for i=1:100
  [inds,cidx,energy] = kmedioids(D,500);
  if(energy<best_energy)
    bestMask = Mask;
    Mask = zeros(N,1);
    Mask(cidx)=1;
    Mask = reshape(Mask,112,112);
    imagesc(Mask);colormap gray;
    drawnow;
  end  
end