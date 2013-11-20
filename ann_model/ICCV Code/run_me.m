clear all; close all;
%graphics_toolkit('fltk')
addpath(genpath('minConf'))
addpath(genpath('PQNexamples'))

seeds = [12312,423,314,7686,247,146,11484,57853,934579,63193];

load eye_data2_manual_clean;
g=gout;

num_reps = 10;
lambdas = logspace(-4,-1,10); 

nVars = size(X,2);
params(1).nHidden = 7;
params(1).init = 'strips';
params(1).hiddenShape = [2,3];

params(2).nHidden = 7;
params(2).init = 'rand';
params(2).hiddenShape = [2,3];

params(3).nHidden = 3;
params(3).init = 'strips';
params(3).hiddenShape = [1,2];

params(4).nHidden = 13;
params(4).init = 'strips';
params(4).hiddenShape = [3,4];

params(5).nHidden = 3;
params(5).init = 'rand';
params(5).hiddenShape = [1,2];

params(6).nHidden = 13;
params(6).init = 'rand';
params(6).hiddenShape = [3,4];


for c=5:6
  for r=1:num_reps
    close all; 
    rand('seed',seeds(r));
    randn('seed',seeds(r));
    global_suffix = sprintf('init_%s_k%d',params(c).init,params(c).nHidden);

    N = size(X,1);
    Ntest = floor(N*0.2);
    ind{r} = randperm(N);
    Xtest = X(ind{r}(1:Ntest),:)/255;
    Xtrain= X(ind{r}(Ntest+1:end),:)/255;

    ytest = bsxfun(@times,g(ind{r}(1:Ntest),:), 1./[112,112]);
    ytrain = bsxfun(@times,g(ind{r}(Ntest+1:end),:), 1./[112,112]);

    Ntrain = size(Xtrain,1);

    %Initial guess of parameters
    switch(params(c).init)
      case 'rand'
        Winit = [];
      case 'strips'
        ns = (params(c).nHidden-1)/2;
        yinit = zeros(Ntrain,params(c).nHidden);
        for s=1:ns
          yinit(:,1+s) = define_target(ytrain,[0,1, -s/(ns+1)],'linear');
          %figure;plot(ytrain(yinit(:,1+s)==1,1),ytrain(yinit(:,1+s)==1,2),'ro') 
          %hold on;plot(ytrain(yinit(:,1+s)==-1,1),ytrain(yinit(:,1+s)==-1,2),'bo') 
          yinit(:,(ns+1)+s) = define_target(ytrain,[1,0, -s/(ns+1)],'linear');
          %figure;plot(ytrain(yinit(:,4+s)==1,1),ytrain(yinit(:,4+s)==1,2),'ro') 
          %hold on;plot(ytrain(yinit(:,4+s)==-1,1),ytrain(yinit(:,4+s)==-1,2),'bo') 
        end
        Winit = train_mlp(Xtrain,yinit,[],0);
        Winit = reshape(Winit,nVars,params(c).nHidden);
        yinit(:,1)=1;
        b{1} = regress(ytrain(:,1), yinit);
        b{2} = regress(ytrain(:,2), yinit);

        %Display Parameters
        figure(1001);clf;
        cm = max(max(abs(Winit(1:end-1,:))));
        for i=1:params(c).nHidden-1
            subplot(params(c).hiddenShape(1),params(c).hiddenShape(2), i); 
            imagesc(reshape(Winit(1:end-1,i+1),[112,112]));colormap(covcolor); caxis([-cm,cm]);colorbar;
        end
        Winit = [Winit(:); b{1}; b{2}];
    end

    for l=1:length(lambdas)

      lambda = lambdas(l);

      local_suffix = sprintf('init_%s_k%d_lambda%f',params(c).init,params(c).nHidden,lambda);
      fname = sprintf('results/results_%s_rep%d.mat',local_suffix,r)
      if(exist(fname)); continue;end;

      W_groupSparse = train_mlp(Xtrain,ytrain,params(c).nHidden,lambda,Winit);
      Winit = W_groupSparse;
      these_results= test_model(W_groupSparse,Xtrain,ytrain,Xtest,ytest,params(c))
      results(l,r) = these_results;

      sparse(l,r) =  results(l,r).sparsity;
      err(l,r) = results(l,r).ErrTest;

      fprintf('Rep %d lambda %d: %.2f %.2f %2\n',r, lambdas(l),results(l,r).ErrTrain,results(l,r).ErrTest, results(l,r).sparsity);

      figure(999);clf; semilogx(sparse(1:l,r),err(1:l,r),'bo-')

      drawnow

      these_params = params(c);
      fname = sprintf('results/results_%s_rep%d.mat',local_suffix,r);
      save(fname,'these_results','lambda','these_params','r','c','l','ind');

      figure;
      count = nVars*params(c).nHidden;
      Wih = reshape(W_groupSparse(1:count),[nVars,params(c).nHidden]);
      cm = max(max(abs(Wih(1:end-1,:))));
      for i=1:params(c).nHidden-1
        subplot(params(c).hiddenShape(1),params(c).hiddenShape(2), i); 
        imagesc(reshape(Wih(1:end-1,i+1),[112,112]));colormap(covcolor); caxis([-cm,cm]);
        set(gca,'xtick',[],'ytick',[]) 
      end
      axes('Position', [0.1 0.15 0.85 0.7], 'Visible', 'off');
      cb=colorbar;
      colormap(covcolor);
      caxis([-cm,cm]);
      fname = sprintf('results/weights_%s_rep%d.pdf',local_suffix,r);
      exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk');

      figure;
      mask = sum(abs(Wih),2)==0;
      imagesc(reshape(mask(1:end-1),[112,112])) ;colormap(gray); caxis([0,1]);
      set(gca,'xtick',[],'ytick',[]);
      title(sprintf('Pixels = %d',nnz(mask(1:end-1)==0) ));
      fname = sprintf('results/pixel_masks_%s_rep%d.pdf',local_suffix,r);
      exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk'); 

      figure;
      hist(results(l,r).dist_test,1:50);
      title('Test Error Histogram');
      fname = sprintf('results/test_error_hist_%s_rep%d.pdf',local_suffix,r);
      xlabel('Gaze Prediction Error')
      ylabel('Frequency')
      exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk'); 

      figure;
      hist(results(l,r).dist_train,1:50);
      title('Train Error Histogram');
      xlabel('Gaze Prediction Error')
      fname = sprintf('results/train_error_hist_%s_rep%d.pdf',local_suffix,r);
      ylabel('Frequency') 
      exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk'); 


    end
  end
end

%collect_results