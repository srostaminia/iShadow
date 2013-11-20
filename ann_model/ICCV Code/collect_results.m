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

for c=1:length(params)

  for r=1:num_reps
    rand('seed',seeds(r));
    randn('seed',seeds(r));
    global_suffix = sprintf('init_%s_k%d',params(c).init,params(c).nHidden);

    for l=1:length(lambdas)
      lambda = lambdas(l);
      local_suffix = sprintf('init_%s_k%d_lambda%f',params(c).init,params(c).nHidden,lambda);
      fname = sprintf('results/results_%s_rep%d.mat',local_suffix,r);
      load(fname,'these_results');
      sparse(l,r) =  these_results.sparsity;
      err(l,r)    = these_results.ErrTest;
    end
  end

  figure;
  semilogx((1-mean(sparse,2))*size(X,2),mean(err,2),'b-s','linewidth',3,'markerfacecolor','w');hold on
  semilogx((1-mean(sparse,2))*size(X,2),(mean(err,2)-std(err,[],2)/sqrt(10)),'b');
  semilogx((1-mean(sparse,2))*size(X,2),(mean(err,2)+std(err,[],2)/sqrt(10)),'b');
  grid on                                                          
  xlim([20,size(X,2)])
  %ylim([0,8])
  xlabel('Number of Active Pixels')
  ylabel('Prediction Error')
  title('Prediction Error vs Active Pixels')
  fname = sprintf('summary/error_vs_pixels_%s.pdf',global_suffix);
  exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk');

  figure;
  numpix = (1-sparse)*size(X,2);
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

  fname = sprintf('summary/pixels_vs_regularization_%s.pdf',global_suffix);
  exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk'); 

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
  fname = sprintf('summary/err_vs_regularization_%s.pdf',global_suffix);
  exportfig(gcf,fname,'width',4,'height',3,'format','pdf','color','cmyk'); 

end