function [orig_err, upd_err] = update_compare_ann(param_file, data_file, batch_size, iters)

addpath(genpath('/Users/ammayber/iShadow/algorithms/ann/lib'));

load(param_file, 'these_results', 'ind', 'lambda');
load(data_file, 'X', 'gout');

rseed = 136824521;

rand('seed',rseed); %set rand seed
randn('seed',rseed); %set randn seed

N = size(X,1);
Ntest = floor(0.2 * N);

X(:,ind(1:end-1)) = mean_contrast_adjust_nosave(X(:,ind(1:end-1)));
X = [X, ones(size(X,1),1)];

y = gout;
y_scaled = bsxfun(@times,gout,1./[111,112]); %Normalize gaze matrix

upd_crossval = randperm(N);

Xtest = X(upd_crossval(1:Ntest),:);
ytest_scaled = y_scaled(upd_crossval(1:Ntest),:);
ytest = y(upd_crossval(1:Ntest),:);

Xtrain = X(upd_crossval(Ntest+1:end),:);
ytrain_scaled = y_scaled(upd_crossval(Ntest+1:end),:);
ytrain = y(upd_crossval(Ntest+1:end),:);

Ntrain = size(Xtrain,1)

upd_err = zeros([Ntrain 2]);

W_orig = these_results.W;
W_upd = params_compress(W_orig, ind, 12433, 7);
alpha0 = these_results.alpha0;

orig_pred = bsxfun(@times, logisticmlp_prediction(W_orig,Xtest,7,2), [112 111]);
orig_err = mean(sqrt(sum((orig_pred - ytest).^2,2)));

% W_upd = params_compress(W_orig, ind, 12433, 7);
% W_upd = update_mlp(Xtrain(:,ind), ytrain_scaled, 7, 0, W_upd, [], 250, 2);
% W_upd = params_expand(W_upd,ind,12433,7);
% 
% pred = bsxfun(@times, logisticmlp_prediction(W_upd,Xtest,7,2), [112 111]);
% upd_err = mean(sqrt(sum((pred - ytest).^2,2)));

iter = 1;
for i=(batch_size + 1):batch_size:Ntrain
    W_upd = update_mlp(Xtrain(i-(batch_size):i,ind), ytrain_scaled(i-(batch_size):i,:), 7, 0, W_upd, [], iters);
    W_upd_expand = params_expand(W_upd,ind,12433,7);

    pred = bsxfun(@times, logisticmlp_prediction(W_upd_expand,Xtest,7,2), [112 111]);

    upd_err(iter) = mean(sqrt(sum((pred - ytest).^2,2)));
    fprintf('%d : %f\n', i, upd_err(iter));
    iter = iter + 1;
end

upd_err(upd_err == 0) = [];