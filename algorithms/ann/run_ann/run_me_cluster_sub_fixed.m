clear all; close all;
graphics_toolkit('fltk')
addpath('~/ann_model')
addpath(genpath('~/ann_model/minConf'))
addpath(genpath('~/ann_model/PQNexamples'))
pkg load statistics;

% params(1).data_name = 'james';
% params(1).exper_type = 'full';

%Set to location of gaze data file
%File should contain:
% * X: eye image data in (n x 12545) matrix with one image per row
%   last column needs to be appended column of 1's. 
% * Out: scene image data in (n x 12544) matrix with one row per image
% * gout: gaze coordinates in (n x 2) matrix
% gaze_data_file = sprintf('~/training_sets/eye_data_%s_auto.mat',params(1).data_name);

gaze_data_file = 'eye_data.mat'

% results_folder = sprintf('~/output/%s/%s/results',params(1).data_name,params(1).exper_type);
% model_folder = sprintf('~/output/%s/%s/models',params(1).data_name,params(1).exper_type);

results_folder = 'results'
model_folder = 'model'

%Regularization parameter range.  
%Must go from low to high values
%Larger values give sparser models
params(1).lambdas = [logspace(-4,-1,10)]
%params(1).lambdas = [logspace(-3,-2,2)];
% params(1).lambdas = [0.001000, 0.010000, 0.100000]

%Set to 1 to remove duplicate data from the training set
params(1).uniquefy = 1;

params(1).data_limit = load('data_limit.txt');

%Number of hidden units. nHidden-1 will be real hidden units
%and 1 will be a bias unit
params(1).nHidden = 7;

%Layout of hidden units for plotting. The product of these values
%must be equal to nHidden-1.
params(1).hiddenShape = [2,3];

%Initialization method. Can be 'strips' or 'rand'
params(1).init   = 'strips';

sub_idx = load('init.txt')

if sub_idx == 0
  params(1).subset = 'l1';
elseif sub_idx == 1
  params(1).subset = 'kmed';
elseif sub_idx == 2
  params(1).subset = 'rand';
end

%Max number of neural network training epochs/function evals
params(1).maxiter = 250;

%Run all results from scratch
%Set to 0 to continue a partial run
from_scratch = 1;


%Set randomization seed
seed = 136824521;

%Do not modify code below this point
%-----------------------------------
fprintf('Loading data...');
load(gaze_data_file); %load data file
fprintf('Done.\n');
rand('seed',seed); %set rand seed
randn('seed',seed); %set randn seed

X2=X; %Non-scaled data

g          = bsxfun(@times,gout,1./[111,112]); %Normalize gaze matrix
X          = [mean_contrast_adjust_nosave(X), ones(size(X,1),1)];
[N,nVars]  = size(X); %get data matrix size
num_reps   = 5;

% for c=[1]
  % r=1
c = 1;
for r = 1:num_reps
  % sparsities = 1-params(c).lambdas;
  % sparsities(1:2)=[];
  global_suffix = sprintf('subset_%s_init_%s_k%d',params(c).subset,params(c).init,params(c).nHidden)

  N = size(X,1);

  if params(c).data_limit > 0
    Ntest = (N - params(c).data_limit);
  else
    Ntest = floor(N*0.2);
  end

  ind = randperm(N);
  
  Xtest = X(ind(1:Ntest),:);
  Xtest2 = X2(ind(1:Ntest),:);

  if (params(c).uniquefy)
    [~, unique_train_ind, ~] = unique(round(gout(ind(Ntest+1:end),:) * 2) / 2, 'rows');
    unique_train_ind = unique_train_ind(randperm(length(unique_train_ind)));  % Undo the sorting done by the unique function

    Xtrain= X(unique_train_ind,:);
    Xtrain2= X2(unique_train_ind,:);

    ytest = g(ind(1:Ntest),:);
    ytrain = g(unique_train_ind,:);
  else
    Xtrain= X(ind(Ntest+1:end),:);
    Xtrain2= X2(ind(Ntest+1:end),:);

    ytest = g(ind(1:Ntest),:);
    ytrain = g(ind(Ntest+1:end),:);
  end

  Ntrain = size(Xtrain,1);

  switch(params(c).init)
    case 'rand'
      Winit0 = [];
      alpha0 =[];
    case 'strips'
      ns = (params(c).nHidden-1)/2;
      yinit = zeros(Ntrain,params(c).nHidden);
      col_covered = [min(ytrain(:,2)), (max(ytrain(:,2)) - min(ytrain(:,2)))];
      row_covered = [min(ytrain(:,1)), (max(ytrain(:,1)) - min(ytrain(:,1)))];
      for s=1:ns
        yinit(:,1+s) = define_target(ytrain,[0,1, -s*col_covered(2)/(ns+1) - col_covered(1)],'linear');
        yinit(:,(ns+1)+s) = define_target(ytrain,[1,0, -s*row_covered(2)/(ns+1) - row_covered(1)],'linear');
      end
      Winit = train_mlp(Xtrain,yinit,[],0,[],[],params(c).maxiter);
      Winit = reshape(Winit,nVars,params(c).nHidden);
      yinit(:,1)=1;
      b{1} = regress(ytrain(:,1), yinit);
      b{2} = regress(ytrain(:,2), yinit);

      Winit0 = [Winit(:); b{1}; b{2}];
      alpha0 = [];
  end
  Winitsub = Winit0;


  for l=1:length(params(c).lambdas) 
    lambda = params(c).lambdas(l)        
    local_suffix=sprintf('subset_%s_init_%s_k%d_lambda%f',params(c).subset,params(c).init,params(c).nHidden,lambda);
    [foo1,foo2,foo3]=mkdir([sprintf('%s/',results_folder),local_suffix]);
    fname = sprintf('%s/%s/rep%d.mat',results_folder, local_suffix,r);
    if(~exist(fname) | from_scratch);

      switch(params(c).subset)
        case 'l1'
          [Winit0,alpha0] = train_mlp(Xtrain,ytrain,params(c).nHidden,lambda,Winitsub,alpha0,params(c).maxiter);
          [Winitsub,ind] = params_mask(Winit0,nVars,params(c).nHidden);

        case 'rand'
          K = max(1,round(111*112*(1-lambda)));
          ind = rect_grid_randsample([111 112],K);
          if(~isempty(Winit))
            Winitsub = params_mask(Winit0,nVars,params(c).nHidden,ind);
          else
            Winitsub = Winit0;
          end
        case 'kmed'
          K = max(1,round(111*112*(1-lambda)));
          ind = rect_grid_kmedioids([111 112],K);
          if(~isempty(Winit0))
            Winitsub = params_mask(Winit0,nVars,params(c).nHidden,ind);
          else
            Winitsub = Winit0;
          end
      end   

      if (ind(end) ~= 12433)
        error('Bias not included in subsampled model!!')
        exit
      end
      
      Xtrain_sub  = [mean_contrast_adjust_nosave(Xtrain2(:,ind(1:end-1) )),ones(size(Xtrain2,1),1)];

      W_groupSparse = train_mlp(Xtrain_sub,ytrain,params(c).nHidden,0,Winitsub,[],params(c).maxiter);
      W_groupSparse = params_expand(W_groupSparse,ind,nVars,params(c).nHidden);
      Winitsub      = W_groupSparse;

      Xtrain_rescale = [Xtrain2(:,:), ones(size(Xtrain2,1),1)];
      Xtrain_rescale(:,ind) = Xtrain_sub;

      Xtest_sub = [mean_contrast_adjust_nosave(Xtest2(:,ind(1:end-1) )),ones(size(Xtest2,1),1)];
      Xtest_rescale = [Xtest2(:,:), ones(size(Xtest2,1),1)];
      Xtest_rescale(:,ind) = Xtest_sub;

      these_results= test_model(W_groupSparse,Xtrain_rescale,ytrain,Xtest_rescale,ytest,params(c));
      these_results.alpha0 = alpha0;

      these_params = params(c);
      fname = sprintf('%s/%s/rep%d.mat',results_folder,local_suffix,r);
      save(fname,'these_results','lambda','these_params','r','c','l','ind');

      
      %Save model for glasses hardware
      fname = sprintf('%s/%s/rep%d',model_folder,local_suffix,r);
      save_model(W_groupSparse,params,fname)
      
    else
      %continue 
      load(fname,'these_results');
      W_groupSparse = these_results.W;
    end 

    clear results;
    results(l,r) = these_results;

    sparse(l,r) =  results(l,r).sparsity;
    err(l,r) = results(l,r).ErrTest;

    pix = (1-results(l,r).sparsity)*111*112;

    fprintf('Rep %d lambda %f  pix %d: %.2f %.2f \n',r, params(c).lambdas(l),pix,results(l,r).ErrTrain,results(l,r).ErrTest);

    % figure(999);clf; semilogx(sparse(1:l,r),err(1:l,r),'bo-')

    % drawnow

    % figure;
    % count = nVars*params(c).nHidden;
    % Wih = reshape(W_groupSparse(1:count),[nVars,params(c).nHidden]);
    % cm = max(max(abs(Wih(1:end-1,:))));
    % ha = tight_subplot(params(c).hiddenShape(1),params(c).hiddenShape(2), 0.01, 0.01, 0.01)
    % for i=1:params(c).nHidden-1
    %   axes(ha(i));
    %   imagesc(reshape(Wih(1:end-1,i+1),[111,112]));colormap(covcolor); caxis([-cm,cm]);
    %   set(gca,'xtick',[],'ytick',[]) 
    % end
    % colormap(covcolor);
    % caxis([-cm,cm]);
    % fname = sprintf('results/%s/weights_rep%d.png',local_suffix,r);
    % % exportfig(gcf,fname,'width',3,'height',2,'format','png','color','cmyk','Resolution',300);
    % print(fname,'-dpng'); %,'-S900,600');

    % figure;
    % mask = sum(abs(Wih),2)==0;
    % maskimg = double(reshape(mask(1:end-1),[111,112]));
    % imagesc(maskimg) ;colormap(gray); caxis([0,1]);
    % set(gca,'xtick',[],'ytick',[]);
    % title(sprintf('Pixels = %d',nnz(mask(1:end-1)==0) ));
    % fname = sprintf('results/%s/pixel_masks_rep%d.pdf',local_suffix,r);
    % % exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk'); 
    % print(fname,'-dpdf'); %,'-S1200,900');

    % figure;
    % hist(results(l,r).dist_test,1:100);
    % ylim([0,40])
    % title('Test Error Histogram');
    % fname = sprintf('results/%s/test_error_hist_rep%d.pdf',local_suffix,r);
    % xlabel('Gaze Prediction Error')
    % ylabel('Frequency')
    % %pretty_graph(gcf,gca);l
    % % exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk'); 
    % print(fname,'-dpdf'); %,'-S1200,900');

    % figure;
    % hist(results(l,r).dist_train,1:100);
    % title('Train Error Histogram');
    % xlabel('Gaze Prediction Error')
    % fname = sprintf('results/%s/train_error_hist_rep%d.pdf',local_suffix,r);
    % ylabel('Frequency') 
    % drawnow;
    % % exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk'); 
    % print(fname,'-dpdf'); %,'-S1200,900');
    
    % %Plot Results
    % for n=1:4
    %   figure()
    %   pred =  logisticmlp_prediction(W_groupSparse,X(n,:),params.nHidden,2);
    %   hold off;
    %   eye_img = reshape(X(n,1:end-1),[111,112]);
    %   out_img = reshape(double(Out(n,1:end))/1000,[111,112]);
    %   imagesc([fliplr(eye_img), out_img]);
    %   hold on;
    %   plot(112+112*pred(1),111*pred(2),'r+','markersize',20,'linewidth',3);
    %   plot(112+112*g(n,1),111*g(n,2),'bo','markersize',20,'linewidth',3);
    %   axis off;
    %   colormap(gray);
    %   drawnow;
    %   fname = sprintf('results/%s/train_error_hist_img%d.pdf',local_suffix,n);
    %   print(fname,'-dpdf');
    % end
  end
end


