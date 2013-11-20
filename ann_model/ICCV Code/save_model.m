function save_model(file,mdir)

      load(file);
      W_groupSparse = these_results.W;
      params = these_params;

      nHidden = params.nHidden;
      HiddenShape = params.hiddenShape;
      nVars = 12545;
      nTargets = 2;

      %Extract parameters
      count = nVars*nHidden;
      Wih = reshape(W_groupSparse(1:count),[nVars,nHidden]);
      Who = reshape(W_groupSparse((count+1):end),[nHidden,nTargets]);
      mask = reshape(sum(abs(Wih(1:end-1,:)),2)>0,[112,112]);
      nnz(mask)
      
      %[col,row,val]=find(mask);
      %maskL   = [row col];
      %player1 = Wih(2:end, 1:end-1);
      %player1 = player1(:,mask(:)==1); 

      [col,row,val]=find(mask');
      maskL   = [row,col];
      
      player1 = Wih(1:end-1,2:end);
      for i = 1:size(maskL,1)
        player1S(i,:) = player1(sub2ind([112,122],maskL(i,1),maskL(i,2)),:); 
      end      
      player2 = Who(2:end,:);

      dlmwrite([mdir,'/nn_bh.txt'],Wih(end,2:end),'delimiter', ' ');
      dlmwrite([mdir,'/nn_bo.txt'],Who(1,:),'delimiter', ' ');
      dlmwrite([mdir,'/nn_wih.txt'],player1S,'delimiter', ' ');
      dlmwrite([mdir,'/nn_who.txt'],player2,'delimiter', ' ');
      dlmwrite([mdir,'/nn_mask.txt'],maskL,'delimiter', ' ');