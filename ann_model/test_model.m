function results = test_model(W_groupSparse,Xtrain,ytrain,Xtest,ytest,params)

      nHidden = params.nHidden;
      HiddenShape = params.hiddenShape;
      nVars = size(Xtrain,2);
      nTargets = size(ytrain,2);

      %Extract parameters
      count = nVars*nHidden;
      Wih = reshape(W_groupSparse(1:count),[nVars,nHidden]);
      Who = reshape(W_groupSparse((count+1):end),[nHidden,nTargets]);

      %Display Parameters
      % figure(1000);clf;
      % cm = max(max(abs(Wih(1:end-1,:))));
      % for i=1:nHidden-1
      %   subplot(HiddenShape(1),HiddenShape(2), i); 
      %   imagesc(reshape(Wih(1:end-1,i+1),[111,112]));colormap(covcolor); caxis([-cm,cm]);colorbar;
      % end

      %Compute error
      nTargets = size(ytrain,2);
      yhat_test = logisticmlp_prediction(W_groupSparse,Xtest,nHidden,nTargets);
      yhat_train = logisticmlp_prediction(W_groupSparse,Xtrain,nHidden,nTargets);

      dist_test = sqrt(sum((112*(ytest - yhat_test)).^2,2));
      dist_train = sqrt(sum((112*(ytrain - yhat_train)).^2,2));

      %Store results
      results.yhat_test= yhat_test;
      results.yhat_train= yhat_train;
      results.ytest= ytest;
      results.ytrain= ytrain;
      results.dist_test=dist_test;
      results.dist_train=dist_train;

      results.W        = W_groupSparse;
      results.ErrTrain = mean(dist_train);
      results.ErrTest  = mean(dist_test);
      %results.lambda   = params.lambdas;
      results.sparsity = 1-nnz(sum(abs(Wih(1:end-1,:)),2))/(nVars-1);

      %[foo,ind] = sort(dist_train,'descend');
      %figure(999)
      %for i=1:size(Xtrain,1)
      %  [dist_train(ind(i)), ytrain(ind(i),:), yhat_train(ind(i),:)]
      %  imagesc(reshape(Xtrain(ind(i),2:end),[112,112]));colormap(gray); caxis([0, 1]);
      %  input('next>>');
      %end
      