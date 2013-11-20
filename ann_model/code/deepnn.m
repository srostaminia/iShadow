clear all 
close all


%% Deep Network with Sparse Connections

fig = 1000;
lambda = 2;
options.maxIter = 100; % Restart more often for non-convex
options.quadraticInit = 2; % Use quadratic initialization of line search
options.verbose = 0;

% Generate non-linear regression data set
nInstances = 200;
nVars = 1;
[X,y] = makeData('regressionNonlinear2',nInstances,nVars);

X = [ones(nInstances,1) X];
nVars = nVars+1;

% Train neural network w/ multiple hiden layers
nHidden = [9 9 9 9 9 9 9 9 9];
nParams = nVars*nHidden(1);
for h = 2:length(nHidden);
    nParams = nParams+nHidden(h-1)*nHidden(h);
end
nParams = nParams+nHidden(end);

funObj = @(weights)MLPregressionLoss_efficient(weights,X,y,nHidden);
fprintf('Training neural network for regression...\n');
lambdaL2 = 1e-3;
wMLP = randn(nParams,1);
funObjL2 = @(w)penalizedL2(w,funObj,lambdaL2);
for i = 1:250
    fprintf('Training:');
    w_old = wMLP;
    wMLP = L1General2_PSSgb(funObjL2,wMLP,lambda*ones(nParams,1),options);
    fprintf(' (nnz = %d, max change = %f)\n',nnz(wMLP),norm(w_old-wMLP,inf));
    if norm(w_old-wMLP,inf) < 1e-5
        break;
	end
end

% Plot results
figure;
hold on
Xtest = [-5:.05:5]';
Xtest = [ones(size(Xtest,1),1) Xtest];
yhat = MLPregressionPredict_efficient(wMLP,Xtest,nHidden);
plot(X(:,2),y,'.');
h=plot(Xtest(:,2),yhat,'g-');
set(h,'LineWidth',3);
legend({'Data','Deep Neural Net'});

% Form weights
inputWeights = reshape(wMLP(1:nVars*nHidden(1)),nVars,nHidden(1));
offset = nVars*nHidden(1);
for h = 2:length(nHidden)
    hiddenWeights{h-1} = reshape(wMLP(offset+1:offset+nHidden(h-1)*nHidden(h)),nHidden(h-1),nHidden(h));
    offset = offset+nHidden(h-1)*nHidden(h);
end
outputWeights = wMLP(offset+1:offset+nHidden(end));

% Make adjacency matrix
adj = zeros(nVars+sum(nHidden)+1);
for i = 1:nVars
    for j = 1:nHidden(1)
        if abs(inputWeights(i,j)) > 1e-4
            adj(i,nVars+j) = 1;
        end
    end
end
for h = 1:length(nHidden)-1
    for i = 1:nHidden(h)
        for j = 1:nHidden(h+1)
            if abs(hiddenWeights{h}(i,j)) > 1e-4
                adj(nVars+sum(nHidden(1:h-1))+i,nVars+sum(nHidden(1:h))+j) = 1;
            end
        end
    end
end
for i = 1:nHidden(end)
    if abs(outputWeights(i)) > 1e-4
        adj(nVars+sum(nHidden(1:end-1))+i,end) = 1;
    end
end

labels = cell(length(adj),1);
for i = 1:nVars
    labels{i,1} = sprintf('x_%d',i);
end
for h = 1:length(nHidden)
    i = i + 1;
    labels{i,1} = sprintf('b_%d',h);
    for j = 2:nHidden(h)
        i = i + 1;
        labels{i,1} = sprintf('h_%d_%d',h,j);
    end
end
labels{end,1} = 'y';

% Plot Network
figure;
drawGraph(adj,'labels',labels);
title('Neural Network');