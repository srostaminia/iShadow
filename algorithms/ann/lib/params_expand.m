function Wfull = params_expand(W,Mask,nVars,nHidden)

  nVars_sub = length(Mask);
  count     = nVars_sub*nHidden;
  Wih       = reshape(W(1:count),[nVars_sub,nHidden]);
  Who       = reshape(W((count+1):end),[nHidden,2]);

  Wihfull   = zeros(nVars,nHidden);
  Wihfull(Mask,:) = Wih;

  Wfull = [Wihfull(:);Who(:)];
