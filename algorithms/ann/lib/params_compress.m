function Wcomp = params_compress(W,Mask,nVars,nHidden)

    Wihfull = reshape(W(1:nVars*nHidden),[nVars,nHidden]);
    Wih = Wihfull(Mask,:);

    Who = W(nVars*nHidden+1:end);
    Wcomp = [Wih(:); Who];