graphics_toolkit('fltk')
addpath(genpath('minConf'))
addpath(genpath('PQNexamples'))

%% Multinomial Logistic Regression

load eye_data;

for r=1:1

gxi = ceil(4*g(:,1)/800);
gxi(gxi==0) = 1;

gyi = ceil(3*g(:,2)/600);
gyi(gyi==0) = 1;

y = 3*(gxi-1) + gyi;


N = size(X,1);
Ntest = floor(N*0.2);
ind{r} = randperm(N);
Xtest = X(ind{r}(1:Ntest),:);
ytest = y(ind{r}(1:Ntest));
Xtrain= X(ind{r}(Ntest+1:end),:);
ytrain= y(ind{r}(Ntest+1:end));


lambdas = linspace(0,10,5);

nInstances = size(Xtrain,1);
nVars = size(Xtrain,2);
nClasses = max(max(ytrain),max(ytest));

% Initial guess of parameters
W_groupSparse = zeros(nVars,nClasses);

for l=1:length(lambdas)

  % Set up Objective Function
  funObj = @(W)mySoftmaxLoss(W,Xtrain,ytrain,nClasses);

  % Set up Groups (don't penalized bias)
  groups = [repmat([1:nVars-1]',1,nClasses);zeros(1,nClasses)];
  groups = groups(:);
  nGroups = max(groups);

  % Initialize auxiliary variables that will bound norm
  lambda = lambdas(l);
  alpha = zeros(nGroups,1);
  penalizedFunObj = @(W)auxGroupLoss(W,groups,lambda,funObj);

  % Set up L_1,2 Projection Function
  [groupStart,groupPtr] = groupl1_makeGroupPointers(groups);
  funProj = @(W)auxGroupL2Project(W,nVars*(nClasses),groupStart,groupPtr);

  % Solve with PQN
  fprintf('\nComputing group-sparse multinomial logistic regression parameters...\n');
  options.verbose=2;
  Walpha = minConf_SPG(penalizedFunObj,[W_groupSparse(:);alpha],funProj,options);

  % Extract parameters from augmented vector
  W_groupSparse(:) = Walpha(1:nVars*(nClasses));
  W_groupSparse(abs(W_groupSparse) < 1e-5) = 0;

  % Check selected variables
  fprintf('Total number of variables selected: %d (of %d)\n',nnz(sum(W_groupSparse(1:end-1,:),2)),nVars);

  figure(l);
  cm = max(max(abs(W_groupSparse(1:end-1,:))));
  for i=1:12
    subplot(3,4,i); 
    imagesc(reshape(W_groupSparse(1:end-1,i),[112,112]));colormap(covcolor); caxis([-cm,cm]);colorbar;
  end


  [junk yhat_test]  = max(Xtest*W_groupSparse,[],2);
  [junk yhat_train] = max(Xtrain*W_groupSparse,[],2);

  fprintf('Rep %d lambda %d: %.2f %.2f\n',r, lambdas(l),mean(ytest ~= yhat_test),mean(ytrain ~= yhat_train) );

  results(l,r).W        = W_groupSparse;
  results(l,r).ErrTrain = mean(ytrain ~= yhat_train);
  results(l,r).ErrTest  = mean(ytest ~= yhat_test);
  results(l,r).lambda   = lambdas(l);
  results(l,r).sparsity = 1-nnz(sum(abs(results(l).W(1:end-1,:)),2))/nVars;

  s(l,r) = results(l,r).sparsity;
  err(l,r) = results(l,r).ErrTest;
  figure(999+l);clf; plot(s(1:l,r),err(1:l,r),'bo-')

  drawnow

end

end

figure;
semilogx((1-mean(s,2))*size(X,2),mean(err,2)*100,'b-s','linewidth',3,'markerfacecolor','w');hold on
semilogx((1-mean(s,2))*size(X,2),(mean(err,2)-std(err,[],2)/sqrt(10)) *100,'b');
semilogx((1-mean(s,2))*size(X,2),(mean(err,2)+std(err,[],2)/sqrt(10)) *100,'b');
grid on                                                          
xlim([20,size(X,2)])
ylim([0,8])
xlabel('Number of Active Pixels')
ylabel('Prediction Error (%)')
title('Prediction Error vs Active Pixels')
exportfig(gcf,'results/error_vs_pixels.pdf','width',4,'height',3,'format','pdf','color','cmyk');

for l = 1:length(lambdas)
  figure(l);
  W_groupSparse = results(l,r).W;
  cm = max(max(abs(W_groupSparse(1:end-1,:))));
  ha = tight_subplot(3,3, 0.01, 0.15, 0.15 );
  for i=1:9
    axes(ha(i)); 
    imagesc(reshape(W_groupSparse(1:end-1,i),[112,112]));colormap(covcolor); caxis([-cm,cm]);axis equal
    set(gca,'xtick',[],'ytick',[]) 
  end
  axes('Position', [0.1 0.15 0.85 0.7], 'Visible', 'off');
  c=colorbar;
  colormap(covcolor);
  caxis([-cm,cm]);
  exportfig(gcf,sprintf('results/weights_lambda%d.pdf',lambdas(l)),'width',4,'height',3,'format','pdf','color','cmyk');
end

figure;
ha = tight_subplot(1,9, 0.01, 0.1, 0.1 );
for l = 1:length(lambdas)
  W_groupSparse = results(l,r).W;
  mask = sum(abs(W_groupSparse),2)==0;
  axes(ha(l)); 
  imagesc(reshape(mask(1:end-1),[112,112])) ;colormap(gray); caxis([0,1]);
  set(gca,'xtick',[],'ytick',[]);
  title(sprintf('Pixels = %d',nnz(mask(1:end-1)==0) )) 
end
exportfig(gcf,'results/pixel_masks.pdf','width',8,'height',0.89,'format','pdf','color','cmyk');


figure;
ha = tight_subplot(3,3, 0.01, 0.0, 0.12 );
for i=1:9
  m = mean(X(y==i,:),1);
  axes(ha(i)); 
  imagesc(reshape(m(1:end-1),[112,112]));colormap(gray);axis equal
  axis off
  set(gca,'xtick',[],'ytick',[]) 
end
exportfig(gcf,sprintf('results/train_means.pdf',lambdas(l)),'width',4,'height',3,'format','pdf','color','cmyk');

figure;
ha = tight_subplot(3,3, 0.01, 0.0, 0.12 );
for i=1:9
  m = mean(Xtest(ytest==i,:),1);
  axes(ha(i)); 
  imagesc(reshape(m(1:end-1),[112,112]));colormap(gray);axis equal
  axis off
  set(gca,'xtick',[],'ytick',[]) 
end
exportfig(gcf,sprintf('results/test_means.pdf',lambdas(l)),'width',4,'height',3,'format','pdf','color','cmyk');


figure;
numpix = (1-s)*size(X,2);
mean_pix = mean(numpix,2);
stderr_pix = std(numpix,[],2)/sqrt(10);
semilogy(lambdas,mean_pix,'b-s','linewidth',3,'markerfacecolor','w');hold on
semilogy(lambdas, mean_pix - stderr_pix,'b');
semilogy(lambdas, mean_pix + stderr_pix,'b');
grid on                                                          
xlim([0,max(lambdas)])
ylim([1,size(X,2)])
xlabel('Regularization Parameter')
ylabel('Number of Active Pixels')
title('Active Pixels vs Regularization')
exportfig(gcf,'results/pixels_vs_regularization.pdf','width',4,'height',3,'format','pdf','color','cmyk');

figure;
perr = err*100;
mean_err = mean(perr,2);;
std_err = std(perr,[],2)/sqrt(10);
plot(lambdas, mean_err,'b-s','linewidth',3,'markerfacecolor','w');hold on
plot(lambdas, mean_err - std_err,'b');
plot(lambdas, mean_err + std_err,'b');
grid on                                                          
xlim([0,max(lambdas)])
ylim([0,10])
xlabel('Regularization Parameter')
ylabel('Prediction Error (%)')
title('Prediction Error vs Regularization')
exportfig(gcf,'results/err_vs_regularization.pdf','width',4,'height',3,'format','pdf','color','cmyk');
