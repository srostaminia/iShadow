
function [Wsub,ind] = params_mask_pupilrad(W,nVars,nHidden,ind)

  count     = nVars*nHidden;
  Wih       = reshape(W(1:count),[nVars,nHidden]);
  Who       = reshape(W((count+1):end),[nHidden,3]);

  if(nargin<4)
    Mask      = sum(abs(Wih),2)>0;
    ind       = find(Mask);
  end
  
  Wih  = Wih(ind,:);
  Wsub = [Wih(:);Who(:)]; 