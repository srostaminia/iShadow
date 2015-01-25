function addVar_mat(result_dir, X, gout)
    addpath('~/iShadow/algorithms/cider');
    addpath('../ann/lib');
    addpath('../ann/run_ann');

    origin = pwd;
    
    cd(result_dir);
    lambda_folders = cellstr(strsplit(ls));
    
    for i=1:length(lambda_folders)-1
        cd(lambda_folders{i});
        
        rep_files = cellstr(strsplit(ls('cider*')));
        
        for j=1:length(rep_files)-1
            fprintf('%s - %d\n',lambda_folders{i},j);
            
            load(rep_files{j}, 'pred','gout');
            %[chord_length,pred,radii,ann_used] = cider(X, rep_files{j}, 400, 0.22, 'circle_edge', 0);
            
%             %Cider model
%             filter = logical(sum(pred,2));
%             dist = sqrt(sum((gout(filter) - pred(filter)).^2,2));
%             err = mean(dist);    
%             
            %line model
            filter_Line = logical(sum(pred,2)) & ~ann_used;
            dist_Line = sqrt(sum((gout(filter_Line) - pred(filter_Line)).^2,2));
            err_Line = mean(dist_Line); 
            
            save('-V7',sprintf('cider_rep%d.mat',j),'err_Line','-append');
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