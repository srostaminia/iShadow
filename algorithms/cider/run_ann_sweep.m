function run_ann_sweep(result_dir, X, gout,nDim,scaleVect)
%function run_ann_sweep(result_dir, X, gout,avgRadEllipse,nDim,scaleVect)
    addpath('~/iShadow/algorithms/cider');
    addpath('../ann/lib');
    addpath('../ann/run_ann');

origin=pwd;

cd(result_dir);
lambda_folders = cellstr(strsplit(ls));

for i=1:length(lambda_folders)-1
    cd(lambda_folders{i});
    
    rep_files = cellstr(strsplit(ls('v7*')));
    
    for j=1:length(rep_files)-1
        fprintf('%s - %d\n',lambda_folders{i},j);
        
        
        
        if nDim==3
            [pred,radii]= ann(X, rep_files{j},nDim,scaleVect);
        else
            [pred]= ann(X, rep_files{j},nDim,scaleVect);
        end
        
        
        
        filter = logical(sum(pred,2));
        dist = sqrt(sum((gout(filter,:) - pred(filter,:)).^2,2));
        err = mean(dist);
        
        
        if nDim==3
            save('-V7',sprintf('ann_rep%d.mat',j),'pred','err','gout','avgRadEllipse','radii');
            
        else
            save('-V7',sprintf('ann_rep%d.mat',j),'pred','err','gout');
            
        end
        
        
        %pred=predicted center, gout=ground truth center,
    end
    
    cd('..');
end

cd(origin);

fprintf('FINISHED');

end