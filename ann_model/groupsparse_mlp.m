clear all; close all;
%graphics_toolkit('fltk')
%addpath(genpath('minConf'))
%addpath(genpath('PQNexamples'))

%% Multinomial Logistic Regression

load eye_data;

for r=1:10

N = size(X,1);
Ntest = floor(N*0.2);
ind{r} = randperm(N);
Xtest = X(ind{r}(1:Ntest),:)/255;
Xtrain= X(ind{r}(Ntest+1:end),:)/255;
ytest = bsxfun(@times,g(ind{r}(1:Ntest),:), 1./[800,600]);
ytrain = bsxfun(@times,g(ind{r}(Ntest+1:end),:), 1./[800,600]);

lambdas = [0.001, 0.005, 0.01, 0.05]; 

nInstances = size(Xtrain,1);
nVars = size(Xtrain,2);
nTargets = 2;

% Initial guess of parameters
nHidden = [6];
HiddenShape = [3,2];
nParams = nVars*nHidden + nHidden*nTargets;
W_groupSparse = [randn(nVars*nHidden,1)/100; randn(nHidden*nTargets,1)/10]

for l=1:length(lambdas)

  % Set up Objective Function
  funObj = @(W)logisticmlp_obj(W,Xtrain,ytrain,nHidden);

  % Set up Groups (don't penalize bias)
  groups = [repmat([1:nVars-1]',1,nHidden);zeros(1,nHidden)];
  groups = [groups(:); zeros(nHidden*nTargets,1)];
  nGroups = max(groups);

  %Initialize auxiliary variables that will bound norm
  lambda = lambdas(l);
  alpha = zeros(nGroups,1);
  penalizedFunObj = @(W)auxGroupLoss(W,groups,lambda,funObj);

  % Set up L_1,2 Projection Function
  [groupStart,groupPtr] = groupl1_makeGroupPointers(groups);
  funProj = @(W)auxGroupL2Project(W,nParams,groupStart,groupPtr);

  %options.verbose='iter';
  %options.optTol=1e-16;
  %W_groupSparse = minFunc(funObj,W_groupSparse(:),options);

  %Solve optimization problem
  options.verbose=2;
  Walpha = minConf_SPG(penalizedFunObj,[W_groupSparse(:);alpha],funProj,options);
  W_groupSparse(:) = Walpha(1:nParams);
  W_groupSparse(abs(W_groupSparse) < 1e-5) = 0;

  %Extract parameters
  count = nVars*nHidden;
  Wih = reshape(W_groupSparse(1:count),[nVars,nHidden]);
  Who = reshape(W_groupSparse((count+1):end),[nHidden,nTargets]);

  %Display Parameters
  figure(1000);clf;
  cm = max(max(abs(Wih(1:end-1,:))));
  for i=1:nHidden
    subplot(HiddenShape(1),HiddenShape(2), i); 
    imagesc(reshape(Wih(1:end-1,i),[112,112]));colormap(covcolor); caxis([-cm,cm]);colorbar;
  end

  %Compute error
  yhat_test = logisticmlp_prediction(W_groupSparse,Xtest,ytest,nHidden);
  yhat_train = logisticmlp_prediction(W_groupSparse,Xtrain,ytrain,nHidden);


  %Display preditions  
  %{
  for i=1:size(Xtest,1)
    figure(1);
    imagesc(reshape(Xtest(i,1:end-1),[112,112]));colormap gray;
    figure(2);hold off;
    Pi = imresize(reshape(P(i,:),[3,4]),[600,800],'method','nearest');
    imagesc(Pi);colormap(gray);caxis([0,1]);hold on;
    plot(max(1,gtest(i,1)),max(1,gtest(i,2)),'ro','markerfacecolor','r','markersize',50);
    drawnow;
    pause(1)
  end
  %}

  %Store results
  results(l,r).W        = W_groupSparse;
  results(l,r).ErrTrain = mean(sqrt(sum((112*(ytest - yhat_test)).^2,2)));
  results(l,r).ErrTest  = mean(sqrt(sum((112*(ytrain - yhat_train)).^2,2)));
  results(l,r).lambda   = lambdas(l);
  results(l,r).sparsity = 1-nnz(sum(abs(Wih(1:end-1,:)),2))/(nVars-1);

  s(l,r) = results(l,r).sparsity;
  err(l,r) = results(l,r).ErrTest;

  fprintf('Rep %d lambda %d: %.2f %.2f %2\n',r, lambdas(l),results(l,r).ErrTrain,results(l,r).ErrTest, results(l,r).sparsity);

  figure(999);clf; plot(s(1:l,r),err(1:l,r),'bo-')

  drawnow

end

end

figure;
semilogx((1-mean(s,2))*size(X,2),mean(err,2),'b-s','linewidth',3,'markerfacecolor','w');hold on
semilogx((1-mean(s,2))*size(X,2),(mean(err,2)-std(err,[],2)/sqrt(10)),'b');
semilogx((1-mean(s,2))*size(X,2),(mean(err,2)+std(err,[],2)/sqrt(10)),'b');
grid on                                                          
xlim([20,size(X,2)])
%ylim([0,8])
xlabel('Number of Active Pixels')
ylabel('Prediction Error')
title('Prediction Error vs Active Pixels')
exportfig(gcf,'results/error_vs_pixels.pdf','width',4,'height',3,'format','pdf','color','cmyk');

for l = 1:length(lambdas)
  figure;

  W_groupSparse = results(l,r).W;
  Wih = reshape(W_groupSparse(1:count),[nVars,nHidden]);

  cm = max(max(abs(Wih(1:end-1,:))));
  ha = tight_subplot(HiddenShape(1),HiddenShape(2),0.01, 0.15, 0.15 );
  for i=1:nHidden
    axes(ha(i)); 
    imagesc(reshape(Wih(1:end-1,i),[112,112]));colormap(covcolor); caxis([-cm,cm]);axis tight;
    set(gca,'xtick',[],'ytick',[]) 
  end

  axes('Position', [0.1 0.15 0.85 0.7], 'Visible', 'off');
  c=colorbar;
  colormap(covcolor);
  caxis([-cm,cm]);
  exportfig(gcf,sprintf('results/weights_lambda%d.pdf',lambdas(l)),'width',4,'height',3,'format','pdf','color','cmyk');
end

figure;
ha = tight_subplot(1,length(lambdas), 0.01, 0.1, 0.1 );
for l = 1:length(lambdas)
  W_groupSparse = results(l,r).W;
  Wih = reshape(W_groupSparse(1:count),[nVars,nHidden]);
  mask = sum(abs(Wih),2)==0;
  axes(ha(l)); 
  imagesc(reshape(mask(1:end-1),[112,112])) ;colormap(gray); caxis([0,1]);
  set(gca,'xtick',[],'ytick',[]);
  title(sprintf('Pixels = %d',nnz(mask(1:end-1)==0) )) 
end
exportfig(gcf,'results/pixel_masks.pdf','width',8,'height',0.89,'format','pdf','color','cmyk');


figure;
numpix = (1-s)*size(X,2);
mean_pix = mean(numpix,2);
stderr_pix = std(numpix,[],2)/sqrt(10);
loglog(lambdas,mean_pix,'b-s','linewidth',3,'markerfacecolor','w');hold on
loglog(lambdas, mean_pix - stderr_pix,'b');
loglog(lambdas, mean_pix + stderr_pix,'b');
grid on                                                          
xlim([0,max(lambdas)])
ylim([1,size(X,2)])
xlabel('Regularization Parameter')
ylabel('Number of Active Pixels')
title('Active Pixels vs Regularization')
exportfig(gcf,'results/pixels_vs_regularization.pdf','width',4,'height',3,'format','pdf','color','cmyk');

figure;
perr = err;
mean_err = mean(perr,2);;
std_err = std(perr,[],2)/sqrt(10);
semilogx(lambdas, mean_err,'b-s','linewidth',3,'markerfacecolor','w');hold on
semilogx(lambdas, mean_err - std_err,'b');
semilogx(lambdas, mean_err + std_err,'b');
grid on                                                          
xlim([0,max(lambdas)])
%ylim([0,30])
xlabel('Regularization Parameter')
ylabel('Prediction Error')
title('Prediction Error vs Regularization')
exportfig(gcf,'results/err_vs_regularization.pdf','width',4,'height',3,'format','pdf','color','cmyk');

save(sprintf('results/results_k%d.mat',nHidden))