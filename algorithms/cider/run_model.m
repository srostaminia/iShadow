function run_model(rep_dir,X,repNum,modelName,sub_model,contrast_method,nDim,scaleVect)
%subLists={'addison','akshaya','shuo','duncan','mike','malai','kaituo'};
%uniList={'uniquefy_0','uniquefy_1'};

% % %


if strcmp(modelName,'cider')
    [ind,chord_length,pred,radii,ann_used] = cider_save(X, rep_dir, 400, 0.22, 'circle_edge', 0,nDim,scaleVect,contrast_method);
elseif strcmp(modelName,'ann')
    [ind,pred,radii]= ann_save(X, rep_file,nDim,scaleVect,contrast_method);
end
% % %
%Cider model
filter = logical(sum(pred,2));
%CENTER
center.diff.raw = sqrt(sum((gout - pred).^2,2));

mean_cider.center.diff = mean(center.diff.raw(filter,:));
mean_cider.center.diff_deg=0.32*mean_cider.center.diff;

if strcmp(modelName,'cider')
    % % %
    %line model
    filter_Line = logical(sum(pred,2)) & ~ann_used;
    %CENTER
    mean_line.center.diff = mean(center.diff.raw(filter_Line,:));
    mean_line.center.diff_deg=0.32*mean_line.center.diff;
end

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
    
    if strcmp(modelName,'cider')
        mean_line.area.diff_perc=mean(area.diff_perc(filter_Line,:));
    end
    
    if strcmp(modelName,'ann')
        save('-V7',sprintf(strcat('ann_%srep%d.mat'),sub_model,repNum),'gout','avgRadEllipse','sqrtRadEllipse',...
                'ind','pred','radii','filter','center','area','radii','mean_ann',...
                'indActivePercent');
    elseif strcmp(modelName,'cider')
         save('-V7',sprintf(strcat('cider_%srep%d.mat'),sub_model,repNum),'gout','avgRadEllipse','sqrtRadEllipse',...
        'ind','chord_length','pred','radii','ann_used','filter','center','area','radii','mean_cider',...
        'filter_Line','mean_line','indActivePercent','perc_AnnUsed','perc_LineUsed');
    
    end
   
else
    
    if strcmp(modelName,'ann')
        save('-V7',sprintf(strcat('ann_%srep%d.mat'),sub_model,repNum),'gout',...
                'ind','pred','filter','center','mean_ann',...
                'indActivePercent');
    elseif strcmp(modelName,'cider')
        save('-V7',sprintf(strcat('cider_%srep%d.mat'),sub_model,repNum),'gout',...
                'ind','chord_length','pred','ann_used','filter','center','mean_cider',...
                'filter_Line','mean_line','indActivePercent','perc_AnnUsed','perc_LineUsed');
    end
    
end
%pred=predicted center, gout=ground truth center,
%radii=estimated radii
%err=cider error
%err_Line=line model error



%fprintf('FINISHED');
