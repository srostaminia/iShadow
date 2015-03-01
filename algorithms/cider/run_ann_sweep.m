function run_ann_sweep(result_dir,input_dir,do_uniquefy,nDim,scaleVect,sub_ann,contrast_method)
%function run_ann_sweep(result_dir, X, gout,avgRadEllipse,nDim,scaleVect)
addpath('~/iShadow/algorithms/cider');
addpath('../ann/lib');
addpath('../ann/run_ann');

origin = pwd;

cd(result_dir);
lambda_folders = cellstr(strsplit(ls));

load(input_dir);

[~, test_ind] = gen_crossval_ind(X, do_uniquefy, gout);

for a=1:5
    XCell{a}=X(test_ind{a},:);
    goutCell{a}=gout(test_ind{a},:);
    
    if nDim==3
        sqrtRadEllipseCell{a}=sqrtRadEllipse(test_ind{a},:);
    end
end

for i=1:length(lambda_folders)-1
    cd(lambda_folders{i});
    
    rep_files = cellstr(strsplit(ls('v7*')));
    
    
    for j=1:length(rep_files)-1
        fprintf('%s - %d\n',lambda_folders{i},j);
        
        X=XCell{j};
        
        
        [ind,pred,radii]= ann(X, rep_files{j},nDim,scaleVect,contrast_method);
        
        gout=goutCell{j};
        
        if nDim==3
            sqrtRadEllipse=sqrtRadEllipseCell{j};
        end
        
        %Cider model
        filter = logical(sum(pred,2));
        %CENTER
        center.diff.raw = sqrt(sum((gout - pred).^2,2));
        
        mean_ann.center.diff = mean(center.diff.raw(filter,:));
        mean_ann.center.diff_deg=0.32*mean_ann.center.diff;
        
        %%ind
        indActivePercent=100*length(ind)/(112*112);
        
        if nDim==3
            %AREA
            area.pred=pi*radii.^2;
            area.diff=(pi*(sqrtRadEllipse.^2-radii.^2)); %in pixels
            area.trueArea=pi*sqrtRadEllipse.^2;
            area.diff_perc=100*abs(area.diff)./(area.trueArea);
            
            mean_ann.area.diff_perc=mean(area.diff_perc(filter,:));
            
            radius.diff=sqrtRadEllipse-radii;
            
            mean_ann.radius.diff=mean(radius.diff(filter,:));
            
            save('-V7',sprintf('ann_%srep%d.mat',sub_ann,j),'gout','avgRadEllipse','sqrtRadEllipse',...
                'ind','pred','radii','filter','center','area','radii','mean_ann',...
                'indActivePercent');
            
        else
            save('-V7',sprintf('ann_%srep%d.mat',sub_ann,j),'gout',...
                'ind','pred','filter','center','mean_ann',...
                'indActivePercent');
        end
        
        
        %pred=predicted center, gout=ground truth center,
    end
    
    cd('..');
end

cd(origin);


end