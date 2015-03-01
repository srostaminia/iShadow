function run_ann_sweep(result_dir,input_dir,nDim,scaleVect,sub_ann,contrast_method)
%function run_ann_sweep(result_dir, X, gout,avgRadEllipse,nDim,scaleVect)
addpath('~/iShadow/algorithms/cider');
addpath('../ann/lib');
addpath('../ann/run_ann');

input_data=load(input_dir);
X=input_data.X;
gout=input_data.gout;
sqrtRadEllipse=input_data.sqrtRadEllipse;

if nDim==3
    sqrtRadEllipse=input_data.sqrtRadEllipse;
end

origin = pwd;

cd(result_dir);
lambda_folders = cellstr(strsplit(ls));

load(input_dir);


for i=1:length(lambda_folders)-1
    cd(lambda_folders{i});
    
    rep_files = cellstr(strsplit(ls('v7*')));
        
    
    for j=1:length(rep_files)-1
        fprintf('%s - %d\n',lambda_folders{i},j);
        
        [ind,pred,radii]= ann(X, rep_files{j},nDim,scaleVect,contrast_method);      
        
        filter = logical(sum(pred,2));
        dist = sqrt(sum((gout(filter,:) - pred(filter,:)).^2,2));
        center_err = mean(dist);
        
        area=pi*radii.^2;
        diffArea=abs(pi*(sqrtRadEllipse(filter,:).^2-radii(filter,:).^2)); %in pixels
        area_err=mean(diffArea);
        
        perAreaDiff_raw=100*diffArea./(pi*sqrtRadEllipse(filter,:).^2);
        perAreaDiff=mean(perAreaDiff_raw);

        indActivePercent=100*length(ind)/(112*112);
        
        

        if nDim==3
            save('-V7',sprintf(strcat('ann',sub_ann,'_rep%d.mat'),j),'dist','pred','center_err','area_err','area','diffArea','gout','avgRadEllipse','sqrtRadEllipse','radii','indActivePercent','perAreaDiff','perAreaDiff_raw');
            
        else
            save('-V7',sprintf(strcat('ann',sub_ann,'rep%d.mat'),j),'dist','pred','center_err','gout','indActivePercent');
            
        end
        
        
        %pred=predicted center, gout=ground truth center,
    end
    
    cd('..');
end

cd(origin);

fprintf('FINISHED');

end