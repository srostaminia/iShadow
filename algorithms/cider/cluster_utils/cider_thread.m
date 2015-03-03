% function cider_thread(result_dir, X, gout,nDim,scaleVect)
function cider_thread(data_file)

addpath(genpath('~/ann_model/'));
addpath(genpath('~/cider_model/'));

load(data_file,'X','gout');

nDim = size(gout,2);

if nDim == 2
    scaleVect == [112, 111];
elseif nDim == 3
    scalevect == [112, 111, 112];
else
    fprintf('Invalid gout width: %d, in data file %s - aborting this thread',nDim,data_file);
    return;
end
    

origin = pwd;

if exist('../uniquefy_0/results','dir')
    cd('../unqiuefy_0/results');
elseif exist('../results','dir')
    cd('../results');
else
    fprintf('Cannot find results folder in %s - aborting this thread',data_file(1:end-4));
end
    

lambda_folders = cellstr(strsplit(ls));

for i=1:length(lambda_folders)-1
    cd(lambda_folders{i});
    
    rep_files = cellstr(strsplit(ls('v7*')));
    
    for j=1:length(rep_files)-1
        fprintf('%s - %d\n',lambda_folders{i},j);
        
        [chord_length,pred,radii,ann_used] = cider(X, rep_files{j}, 400, 0.22, 'circle_edge', 0,nDim,scaleVect);
        
        %Cider model
        filter = logical(sum(pred,2));
        dist = sqrt(sum((gout - pred).^2,2));
        %dist = sqrt(sum((gout(filter,:) - pred(filter,:)).^2,2));
        err = mean(dist(filter,:));
        
        
        %line model
        filter_Line = logical(sum(pred,2)) & ~ann_used;
         %size(filter_Line)
        
        dist_Line = dist(filter_Line);
        err_Line = mean(dist_Line);
        
        
        
        if nDim==2
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