function [yhat] = MLPregressionLoss(w,X,y,nHidden)

%Get number of instances, variables, and layers
[nInstances,nVars] = size(X);
[nInstances,nTargets] = size(y);
NL = length(nHidden)+1;

if(isempty(nHidden))

  %Get weights
  outputWeights = reshape(w,nVars,nTargets);

  %Forward pass
  ip = X*outputWeights;
  yhat = tanh(ip);

else

  %Extract weights from weight vector
  if(~isempty(nHidden))
    inputWeights = reshape(w(1:nVars*nHidden(1)),nVars,nHidden(1));
    offset = nVars*nHidden(1);
    for h = 2:length(nHidden)
      hiddenWeights{h-1} = reshape(w(offset+1:offset+nHidden(h-1)*nHidden(h)),nHidden(h-1),nHidden(h));
      offset = offset+nHidden(h-1)*nHidden(h);
    end
  else
    offset = 0;
  end
  outputWeights = reshape(w(offset+1:offset+nHidden(end)*nTargets),nHidden(end),nTargets);

  % Compute Output
  if(~isempty(nHidden)) 
    ip{1} = X*inputWeights;
    fp{1} = tanh(ip{1});
    ip{1}(:,1) = -inf; %Correct bias unit
    fp{1}(:,1) = 1;    %Correct for bias unit
    for h = 2:length(nHidden)
        ip{h} = fp{h-1}*hiddenWeights{h-1};
        fp{h} = tanh(ip{h});
        ip{h}(:,1) = -inf; %Corect for bias unit
        fp{h}(:,1) = 1;    %Correct for bias unit
    end
    yhat = fp{end}*outputWeights;
  else
    yhat = tanh(X*outputWeights{1});
  end
end

fp{1}
outputWeights