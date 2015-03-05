function run_cider_sweep(result_dir, input_dir,nDim,scaleVect,sub_cider,contrast_method)
%function run_cider_sweep(result_dir, X, gout,avgEllipseRad,nDim,scaleVect)
addpath('~/iShadow/algorithms/cider');
addpath('../ann/lib');
addpath('../ann/run_ann');

origin = pwd;

cd(result_dir);
lambda_folders = cellstr(strsplit(ls));

load(input_dir);

for i=1:length(lambda_folders)-1
    cd(lambda_folders{i});
    
    rep_files = cellstr(strsplit(ls('v7*')));
    
    for j=1:length(rep_files)-1
        
        
        fprintf('%s - %d\n',lambda_folders{i},j);
        
        [ind,chord_length,pred,radii,ann_used] = cider(X, rep_files{j}, 400, 0.22, 'circle_edge', 0,nDim,scaleVect,contrast_method);
        
        % % %
        %Cider model
        filter = logical(sum(pred,2));
        %CENTER        
        center.diff.raw = sqrt(sum((gout - pred).^2,2));
        
        mean_cider.center.diff = mean(center.diff.raw(filter,:));
        mean_cider.center.diff_deg=0.32*mean_cider.center.diff;
        
        % % %
        %line model
        filter_Line = logical(sum(pred,2)) & ~ann_used;
        %CENTER
        mean_line.center.diff = mean(center.diff.raw(filter_Line,:));
        mean_line.center.diff_deg=0.32*mean_line.center.diff;
        
        %%ind
        indActivePercent=100*length(ind)/(112*112);
        
        totalFrame=size(ann_used,1);
        perc_AnnUsed=100*sum(ann_used)/totalFrame;
        perc_LineUsed=100-perc_AnnUsed;
        
        if nDim==3
            %AREA
            area.pred=pi*radii.^2;
            area.diff=(pi*(sqrtRadEllipse.^2-radii.^2)); %in pixels
            area.trueArea=pi*sqrtRadEllipse.^2;
            area.diff_perc=100*abs(area.diff)./(area.trueArea);
            
            mean_cider.area.diff_perc=mean(area.diff_perc(filter,:));
            mean_line.area.diff_perc=mean(area.diff_perc(filter_Line,:));
            
            save('-V7',sprintf(strcat('cider',sub_cider,'_rep%d.mat'),j),'gout','avgRadEllipse','sqrtRadEllipse',...
                'ind','chord_length','pred','radii','ann_used','filter','center','area','radii','mean_cider',...
                'filter_Line','mean_line','indActivePercent','perc_AnnUsed','perc_LineUsed');
            
        else
            save('-V7',sprintf(strcat('cider',sub_cider,'_rep%d.mat'),j),'gout',...
                'ind','chord_length','pred','ann_used','filter','center','mean_cider',...
                'filter_Line','mean_line','indActivePercent','perc_AnnUsed','perc_LineUsed');
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