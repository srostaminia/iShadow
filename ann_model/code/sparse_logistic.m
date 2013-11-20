addpath(genpath('L1General'))
addpath(genpath('L1GeneralExamples'))

%% Multinomial Logistic Regression
load eye_data;

% Generate Data
nInstances = size(X,1);
nVars = size(X,2);
nClasses = 9;

% Initialize Weights and Objective Function
w_init = zeros(nVars,nClasses-1);
w_init = w_init(:);
funObj = @(w)SoftmaxLoss2(w,X,y,nClasses);

% Set up regularizer
lambda = 2000*ones(nVars,nClasses-1);
lambda(end,:) = 0; % Don't regularize bias elements
lambda = lambda(:);

% Maximum Likelihood
%fprintf('\nComputing Maximum Likelihood Multinomial Logistic Regression Coefficients\n');
%mfOptions.Method = 'lbfgs';
%wMLR_ML = minFunc(funObj,w_init,mfOptions);
%wMLR_ML = reshape(wMLR_ML,nVars,nClasses-1);

% L2-Regularized Logistic Regression
%fprintf('\nComputing L2-Regularized Multinomial Logistic Regression Coefficients...\n');
%funObjL2 = @(w)penalizedL2(w,funObj,lambda);
%wMLR_L2 = minFunc(funObjL2,w_init,mfOptions);
%wMLR_L2 = reshape(wMLR_L2,nVars,nClasses-1);

% L1-Regularized Logistic Regression
fprintf('\nComputing L1-Regularized Multinomial Logistic Regression Coefficients...\n');
wMLR_L1 = L1General2_PSSgb(funObj,w_init,lambda);
wMLR_L1 = reshape(wMLR_L1,nVars,nClasses-1);



% Report Number of non-zeros
fprintf('Number of Features Selected by Maximum Likelihood Multinomial Logistic classifier: %d (out of %d)\n',nnz(wMLR_ML(2:end,:)),(nVars-1)*(nClasses-1));
%fprintf('Number of Features Selected by L2-regualrized Multinomial Logistic classifier: %d (out of %d)\n',nnz(wMLR_L2(2:end,:)),(nVars-1)*(nClasses-1));
fprintf('Number of Features Selected by L1-regualrized Multinomial Logistic classifier: %d (out of %d)\n',nnz(wMLR_L1(2:end,:)),(nVars-1)*(nClasses-1));
%fprintf('Number of Features Selected by Elastic-Net Multinomial Logistic classifier: %d (out of %d)\n',nnz(wMLR_L1L2(2:end,:)),(nVars-1)*(nClasses-1));

% Show Stem Plots
figure;
clf;hold on;
subplot(2,2,1);
stem(wMLR_ML(:),'r');
title('Maximum Likelihood Multinomial Logistic');
subplot(2,2,2);
stem(wMLR_L2(:),'b');
title('L2-Regularized Multinomial Logistic');
subplot(2,2,3);
stem(wMLR_L1(:),'g');
title('L1-Regularized Multinomial Logistic');
subplot(2,2,4);
stem(wMLR_L1L2(:),'c');
title('Elastic-Net Multinomial Logistic');

% Compute training error
[junk yhat] = max(X*[wMLR_L1 zeros(nVars,1)],[],2);
fprintf('Classification error rate on training data for L1-regularied Multinomial Logistic: %.2f\n',sum(y ~= yhat)/length(y));
pause;
