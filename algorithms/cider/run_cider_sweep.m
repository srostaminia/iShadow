function run_cider_sweep(result_dir, X, gout,nDim,scaleVect)
%function run_cider_sweep(result_dir, X, gout,avgEllipseRad,nDim,scaleVect)
addpath('~/iShadow/algorithms/cider');
addpath('../ann/lib');
addpath('../ann/run_ann');

origin = pwd;

cd(result_dir);
lambda_folders = cellstr(strsplit(ls));

for i=1:length(lambda_folders)-1
    cd(lambda_folders{i});
    
    rep_files = cellstr(strsplit(ls('v7*')));
    
    for j=1:length(rep_files)-1
        fprintf('%s - %d\n',lambda_folders{i},j);
        
        [chord_length,pred,radii,ann_used] = cider(X, rep_files{j}, 400, 0.22, 'circle_edge', 0,nDim,scaleVect);
        
        %Cider model
        filter = logical(sum(pred,2));
        dist = sqrt(sum((gout(filter,:) - pred(filter,:)).^2,2));
        err = mean(dist);
        
        %line model
        filter_Line = logical(sum(pred,2)) & ~ann_used;
        dist_Line = sqrt(sum((gout(filter_Line,:) - pred(filter_Line,:)).^2,2));
        err_Line = mean(dist_Line);
        
        
        
        if nDim==3
            save('-V7',sprintf('cider_rep%d.mat',j),'chord_length','pred','radii','ann_used','dist','err','gout','err_Line');
        else
            
            save('-V7',sprintf('cider_rep%d.mat',j),'chord_length','pred','radii','ann_used','dist','err','gout','err_Line','avgEllipseRad');
        end
        %pred=predicted center, gout=ground truth center,
        %radii=estimated radii
        %err=cider error
        %err_Line=line model error
    end
    
    cd('..');
end

cd(origin);

fprintf('FINISHED');
end