function [error_means,error_std]=collect_cider_results(result_dir)
    origin = pwd;
    
    cd(result_dir);
    lambda_folders = sort(cellstr(strsplit(ls)));
    lambda_folders = lambda_folders(2:end);
    
    error_means = zeros(length(lambda_folders) - 1, 1);
    error_std = zeros(length(lambda_folders) - 1, 1);
    
    for i=1:length(lambda_folders)
        cd(lambda_folders{i});
        
        cider_files = sort(cellstr(strsplit(ls('cider*'))));
        cider_files = cider_files(2:end);
        
        rep_errs = zeros(length(cider_files) - 1, 1);
        for j=1:length(cider_files)           
            
            load(cider_files{j}, 'err');
            rep_errs(j) = err;
            
        end
        
        error_means(i) = mean(rep_errs);
        error_std(i) = std(rep_errs);
        
        cd('..');
    end
    
    cd(origin);

end