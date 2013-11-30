function load_model(img)

      nTargets = 2;
      bi  = dlmread('models/nn_bi.txt',Wih(2:end,:),'delimiter', ' ');
      bh  = dlmread('models/nn_bh.txt',Who(1,:),'delimiter', ' ');
      Wih = dlmread('models/nn_wih.txt',player1,'delimiter', ' ');
      Who = dlmread('models/nn_who.txt',player2,'delimiter', ' ');
      mask = dlmread('models/nn_mask.txt',maskL,'delimiter', ' ');

      for  

    ip = X*inputWeights;
    fp{1} = tanh(ip{1});
    ip{1}(:,1) = -inf; %Correct bias unit
    fp{1}(:,1) = 1;    %Correct for bias unit
    for h = 2:length(nHidden)
        ip{h} = fp{h-1}*hiddenWeights{h-1};
        fp{h} = tanh(ip{h});
        ip{h}(:,1) = -inf; %Corect for bias unit
        fp{h}(:,1) = 1;    %Correct for bias unit
    end
    yhat = fp{end}*outputWeights;
  else
    yhat = tanh(X*outputWeights{1});
  end
