function run_cider_sweep(result_dir, X, gout)
    origin = pwd;
    
    cd(result_dir);
    lambda_folders = cellstr(strsplit(ls));
    
    for i=1:length(lambda_folders)-1
        cd(lambda_folders{i});
        
        rep_files = cellstr(strsplit(ls('v7*')));
        
        for j=1:length(rep_files)-1
            fprintf('%s - %d\n',lambda_folders{i},j);
            
            [chord_length,pred,radii,ann_used] = cider(X, rep_files{j}, 400, 0.22, 'circle_edge', 0);
            
            filter = logical(sum(pred,2));
            dist = sqrt(sum((gout(filter) - pred(filter)).^2,2));
            err = mean(dist);            
            
            save('-V7',sprintf('cider_rep%d.mat',j),'chord_length','pred','radii','ann_used','dist','err','gout');
            
        end
        
        cd('..');
    end
    
    cd(origin);

end