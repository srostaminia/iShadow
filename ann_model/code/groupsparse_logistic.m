addpath(genpath('/nfs/mar/data1/marlin/projects/SensEye/gazepredict/minConf'))
addpath(genpath('/nfs/mar/data1/marlin/projects/SensEye/gazepredict/PQNexamples'))

%% Multinomial Logistic Regression
load eye_data;

N = size(X,1);
Ntest = floor(N*0.2);
ind = randperm(N);
Xtest = X(ind(1:Ntest),:);
ytest = y(ind(1:Ntest));
X     = X(ind(Ntest+1:end),:);
y     = y(ind(Ntest+1:end));


lambdas = [0,1,10,100,1000,10000]

for l=1:length(lambdas)

  % Generate Data
  nInstances = size(X,1);
  nVars = size(X,2);
  nClasses = 9;

  % Initial guess of parameters
  W_groupSparse = zeros(nVars,nClasses-1);

  % Set up Objective Function
  funObj = @(W)SoftmaxLoss2(W,X,y,nClasses);

  % Set up Groups (don't penalized bias)
  groups = [repmat([1:nVars-1]',1,nClasses-1);zeros(1,nClasses-1)];
  groups = groups(:);
  nGroups = max(groups);

  % Initialize auxiliary variables that will bound norm
  lambda = 1000;
  alpha = zeros(nGroups,1);
  penalizedFunObj = @(W)auxGroupLoss(W,groups,lambda,funObj);

  % Set up L_1,2 Projection Function
  [groupStart,groupPtr] = groupl1_makeGroupPointers(groups);
  funProj = @(W)auxGroupL2Project(W,nVars*(nClasses-1),groupStart,groupPtr);

  % Solve with PQN
  fprintf('\nComputing group-sparse multinomial logistic regression parameters...\n');
  Walpha = minConf_SPG(penalizedFunObj,[W_groupSparse(:);alpha],funProj);

  % Extract parameters from augmented vector
  W_groupSparse(:) = Walpha(1:nVars*(nClasses-1));
  W_groupSparse(abs(W_groupSparse) < 1e-4) = 0;

  figure
  subplot(1,2,1);
  imagesc(W_groupSparse~=0);colormap gray
  title('Sparsity Pattern');
  ylabel('variable');
  xlabel('class label');
  subplot(1,2,2);
  imagesc(W_groupSparse);colormap gray
  title('Variable weights');
  ylabel('variable');
  xlabel('class label');

  % Check selected variables
  fprintf('Total number of variables selected: %d (of %d)\n',nnz(sum(W_groupSparse(1:end-1,:),2)),nVars);

  figure();
  cm = max(max(abs(W_groupSparse(1:end-1,:))));
  for i=1:8
    subplot(3,3,i); 
    imagesc(reshape(W_groupSparse(1:end-1,i),[112,112]));colormap(covcolor); caxis([-cm,cm]);colorbar;
  end

  [junk yhat_test]  = max(Xtest*[W_groupSparse, zeros(nVars,1)],[],2);
  [junk yhat_train] = max(X*[W_groupSparse, zeros(nVars,1)],[],2);

  fprintf('lambda %d: %.2f\n',lambdas(l),mean(ytest ~= yhat_test),mean(y ~= yhat_train) );

end