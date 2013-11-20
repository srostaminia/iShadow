
lambdas = logspace(-4,-1,10); 
params(1).init = 'rand';
params(1).hiddenShape = [2,3];
params(1).nHidden = 7;

for c=1:1
  for r=1:1
    for l=1:length(lambdas)
      lambda = lambdas(l);
      local_suffix =  sprintf('init_%s_k%d_lambda%f',params(c).init,params(c).nHidden,lambda);
      fname = sprintf('matlab_models/results_%s_rep%d.mat',local_suffix,r);
      mdir = ['model_',local_suffix];
      mkdir(mdir);
      save_model(fname,mdir)
    end
  end
end  