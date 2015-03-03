function [W,alpha] = update_mlp(Xtrain,ytrain,nHidden,lambda,W_groupSparse,alphainit, maxiter)
%   addpath('minConf');

  nInstances = size(Xtrain,1);
  nVars = size(Xtrain,2);
  nTargets = size(ytrain,2);
  nParams = nVars*nHidden + nHidden*nTargets;
  
  % Set up Objective Function
  funObj = @(W)logisticmlp_obj(W,Xtrain,ytrain,nHidden);

  % Set up Groups (don't penalize bias)
  if(~isempty(nHidden))
    groups = [repmat([1:nVars-1]',1,nHidden);zeros(1,nHidden)];
    groups = [groups(:); zeros(nHidden*nTargets,1)];
  else
    groups = [repmat([1:nVars-1]',1,nTargets);zeros(1,nTargets)];
    groups = groups(:);
  end
  nGroups = max(groups);
  
  if(nargin>=6 && ~isempty(alphainit))
    alpha = alphainit;
  else
    alpha = zeros(nGroups,1);
  end

  %Initialize auxiliary variables that will bound norm
  penalizedFunObj = @(W)auxGroupLoss(W,groups,lambda,funObj);

  % Set up L_1,2 Projection Function
  [groupStart,groupPtr] = groupl1_makeGroupPointers(groups);
  funProj = @(W)auxGroupL2Project(W,nParams,groupStart,groupPtr);

  %Solve optimization problem
%   options.verbose=2;
  options.verbose=2;
  options.maxIter = maxiter;

  options.optTol = 1e-5; %default 1e-5
  options.progTol  = 1e-9; %default 1e-9

  Walpha = minConf_SPG(penalizedFunObj,[W_groupSparse(:);alpha],funProj,options);
  W = Walpha(1:nParams);
  W(abs(W) < 1e-5) = 0;
  alpha = Walpha((1+nParams):end);
