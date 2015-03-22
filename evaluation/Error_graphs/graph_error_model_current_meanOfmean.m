%function [errStEachSub,meanErrMatForAllSub,annUsedStEachLambdaEachSub,meanAnnUsedForAllLambdaEachSub]=graph_error_models(subList, rootDir, modelName)

%clear
%addpath('~/iShadow/algorithms/cider/algorithms/cider')
%author- yamin
%subList={'yamin','addison','akshaya','duncan','mike'};
%rootDir='~/iShadow/algorithms/cider/data/';
%modelName='cider' % 'ANN' %'both'

%graph uniquefy0 and uniquefy1 separately->unique
%higher lambda->more regularization/restriction->smaller subsampling
%***average for each users

%Error of predicted pupil center for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'
function [errStEachSub,err_line_StEachSub,meanErrAllSub,stdErrAllSub,indPerForAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub,meanAnnUsedForAllSub,stdAnnUsedForAllSub,meanLineUsedForAllSub,stdLineUsedForAllSub]=graph_error_model_current_meanOfmean(indFile,linestyle,modelName,subList,rootDir,lightingSubFolder,objToPlot,subModelName,compareLine,lighting)
%function [errStEachSub,meanErrAllSub,stdErrAllSub,indPerForAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub,meanAnnUsedForAllSub,stdAnnUsedForAllSub,meanLineUsedForAllSub,stdLineUsedForAllSub]=graph_error_model_current(indFile,linestyle,modelName,subList,rootDir,lightingSubFolder,objToPlot,subModelName)

uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
uni=uniFolderList{1};
lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
if strcmp(lighting,'ohsocold')
  lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416'};
lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416'};

end

lambdaStrList=fliplr(lambdaStrList);
lambdaFolderList=fliplr(lambdaFolderList);

nLambda=length(lambdaStrList);
%errList=cell(1,length(subList));
nUni=length(uniFolderList);
annUsedStEachLambdaEachSub =struct;

errStEachSub = struct;
err_line_StEachSub = struct;
indStEachSub = struct;
%.raw.uniquefy0.yamin=?each subject field 5x10
%.mean.uniquefy0.yamin=? each subject field 1x10
%.std.uniquefy0.yamin=? each subject field 1x10

%errStAllSub=struct; %over 6 users
%.raw.uniquefy0=?each subject field usersx10
%.mean.uniquefy0=? each subject field 1x10
%.std.uniquefy0=? each subject field 1x10

nUsers=length(subList);
nRep=5;

%fieldList = unique(subList);
%annFreq=zeros(1,size(subList));

totalFrameEachSub=zeros(nUsers,1);

%ori=pwd;
% cd (rootDir);
% subNameFolderCell=cellstr(strsplit(ls));
% cd(ori);

for i=1:nUsers
    
    subName=subList{i}; %subject name
    %indFile=i;
    subNameFolder=strcat(subName,lightingSubFolder);%subNameFolderCell{indFile};%
    
    
    errMatEachSub=zeros(nRep,nLambda); %errSt 5x10
    err_line_MatEachSub=zeros(nRep,nLambda);
    annUsedMatEachSub=zeros(nRep,nLambda);
    indPerMatEachSub=zeros(nRep,nLambda);
    
    j=1;
    
    for k=1:nLambda
        
        lambdaFolder=lambdaFolderList{k};
        %lambdaStr=lambdaStrList{k};
        
        for m=1:nRep
            
            
            dir= fullfile(rootDir,strcat(subNameFolder),'results',lambdaFolder,strcat(modelName,'_',subModelName,'rep',num2str(m),'.mat'));
            %dir= fullfile(rootDir,strcat(subNameFolder),uni,'results',lambdaFolder,strcat(modelName,'_',subModelName,'rep',num2str(m),'.mat'));
            
            data=load(dir);
            
            if compareLine
                cider_dir= fullfile(rootDir,strcat(subNameFolder),'results',lambdaFolder,strcat('cider','_',subModelName,'rep',num2str(m),'.mat'));
                % cider_dir= fullfile(rootDir,strcat(subNameFolder),uni,'results',lambdaFolder,strcat('cider','_',subModelName,'rep',num2str(m),'.mat'));
               cider_data=load(cider_dir);
            end
            
            %size(pi*((data.radii.true(data.radii.true>0)).^2))
            %size(areaTrue)
            if strcmp(objToPlot,'area')
                rad_filter=data.radii.true>0;
                areaTrue=pi*(data.radii.true).^2;
                area_raw=100*abs(data.area.pred-areaTrue)./areaTrue;
                %errMatEachSub(m,k,j)=data.mean_model.area.diff_perc;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                
                %trueA=[areaTrue(filter) data.radii.true(filter) data.area.pred(filter) data.radii.pred(filter)];
                
                if strcmp(modelName,'cider')
                    filter=logical(sum(data.center.pred,2)) & rad_filter;
                    filter_line=filter & ~data.ann_used & rad_filter;
                    errMatEachSub(m,k,j)=mean(area_raw(filter));
                    err_line_MatEachSub(m,k,j)=mean(area_raw(filter_line));%data.mean_line.area.diff_perc;
                    annUsedMatEachSub(m,k,j)=data.perc_AnnUsed;
                else
                    if compareLine
                        errMatEachSub(m,k,j)=mean(area_raw(rad_filter & ~cider_data.ann_used));%data.mean_model.area.diff_perc;

                    else
                        errMatEachSub(m,k,j)=mean(area_raw(rad_filter));%data.mean_model.area.diff_perc;
   
                    end
                end
                
            elseif strcmp(objToPlot,'pupilLoc-deg')
                
                if strcmp(modelName,'cider')
                    err_line_MatEachSub(m,k,j)=data.mean_line.center.diff_deg;
                    annUsedMatEachSub(m,k,j)=data.perc_AnnUsed;
                    
                elseif strcmp(modelName,'ann')
                    if compareLine
                        data.mean_model.center.diff_deg=mean(0.32*abs(data.center.diff.raw(~cider_data.ann_used)));%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                        
                    end
                end
                errMatEachSub(m,k,j)=data.mean_model.center.diff_deg;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);

            elseif strcmp(objToPlot,'pupilcentPix')
                
                
                if strcmp(modelName,'cider')
                    %data.err;%
                    err_line_MatEachSub(m,k,j)=data.mean_line.center.diff;%data.err_Line;%
                    annUsedMatEachSub(m,k,j)=data.perc_AnnUsed;
                elseif strcmp(modelName,'ann') && compareLine
                    
                    data.mean_model.center.diff=mean(abs(data.center.diff.raw(~cider_data.ann_used)));%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                end
                
                errMatEachSub(m,k,j)=data.mean_model.center.diff;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);

                %errMatEachSub(m,k,j)=data.center_err;
            elseif strcmp(objToPlot,'radius')
                rad_filter=data.radii.true>0;
                rad_raw=abs(data.radii.pred-data.radii.true);
                
                %abs(data.mean_model.radius.diff);%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                
                if strcmp(modelName,'cider')
                    filter=logical(sum(data.center.pred,2)) & rad_filter;
                    filter_line=filter & ~data.ann_used & rad_filter;
                    err_line_MatEachSub(m,k,j)=mean(rad_raw(filter_line));
                    errMatEachSub(m,k,j)=mean(rad_raw(filter));
                    annUsedMatEachSub(m,k,j)=data.perc_AnnUsed;
                  
                else
                    
                    if compareLine
                        errMatEachSub(m,k,j)=mean(abs(rad_raw(rad_filter & ~cider_data.ann_used)));%data.mean_model.area.diff_perc;

                    else
                        errMatEachSub(m,k,j)=mean(rad_raw(rad_filter));%data.mean_model.area.diff_perc;
   
                    end
                    
                end
            end
            
            indPerMatEachSub(m,k,j)=data.indActivePercent;
            
        end
        
        
        
    end
    
    %errMatEachSub
    
    if strcmp(modelName,'cider')
        annUsedStEachLambdaEachSub.raw.(uni).(subName)=annUsedMatEachSub(:,:,j); %5x10
        annUsedStEachLambdaEachSub.mean.(uni).(subName)=round(mean(annUsedMatEachSub(:,:,j),1));
        annUsedStEachLambdaEachSub.std.(uni).(subName)=std(annUsedMatEachSub(:,:,j),0,1);
        
        
        err_line_StEachSub.raw.(uni).(subName)=err_line_MatEachSub(:,:,j);
        err_line_StEachSub.mean.(uni).(subName)=mean(err_line_MatEachSub(:,:,j),1);
        err_line_StEachSub.std.(uni).(subName)=std(err_line_MatEachSub(:,:,j),0,1);
        
    end
    
    errStEachSub.raw.(uni).(subName)=errMatEachSub(:,:,j);
    errStEachSub.mean.(uni).(subName)=mean(errMatEachSub(:,:,j),1);
    errStEachSub.std.(uni).(subName)=std(errMatEachSub(:,:,j),0,1);
    
    % indStEachSub.(uni).(subName)=indPerMatEachSub(:,j);
    
    indStEachSub.raw.(uni).(subName)=indPerMatEachSub(:,:,j);
    indStEachSub.mean.(uni).(subName)=mean(indPerMatEachSub(:,:,j),1);
    % indStEachSub.std.(uni).(subName)=std(indPerMatEachSub(:,:,j),0,1);
end



%compute mean for all subjects
meanErrMatForAllSub=zeros(nUsers,nLambda);
meanErrMat_line_ForAllSub=zeros(nUsers,nLambda);
indPerMatForAllSub=zeros(nUsers,nLambda);
meanAnnUsedMatForAllSub=zeros(nUsers,nLambda);
meanLineUsedMatForAllSub=zeros(nUsers,nLambda);

for i=1:nUsers
    subName=subList{i};
    meanErrMatForAllSub(i,:)=errStEachSub.mean.(uni).(subName);
    indPerMatForAllSub(i,:)=indStEachSub.mean.(uni).(subName);
end


if strcmp(modelName,'cider')
    %stdAnnUsedForAllSub=zeros(nUsers,nLambda);
    
    for i=1:nUsers %nUsers x nLambda: 6x10
        subName=subList{i};
        meanAnnUsedMatForAllSub(i,:)=annUsedStEachLambdaEachSub.mean.(uni).(subName);
        meanLineUsedMatForAllSub(i,:)=100-annUsedStEachLambdaEachSub.mean.(uni).(subName);

        meanErrMat_line_ForAllSub(i,:)=err_line_StEachSub.mean.(uni).(subName);
        
%         FreqAnnUsedMatForAllSub(i,:)=annUsedStEachLambdaEachSub.mean.(uni).(subName);
%         FreqLineUsedMatForAllSub(i,:)=100-annUsedStEachLambdaEachSub.mean.(uni).(subName);

    end
    
end
%% CIDER: Graph Subplot individual user's error
%figure;


%save(fullfile('annSize.mat'),'indPerMatForAllSub');



for i=1:nUsers
    
    subName=subList{i};
    errorbar(1:1:nLambda,meanErrMatForAllSub(i,:),errStEachSub.std.(uni).(subName),linestyle,'markers',10);
    hold on;
end

% hold off;
% limsy=get(gca,'YLim');
% set(gca,'Ylim',[0 limsy(2)]);
% %ylim([0, 2.9]) %10.1
% xlim([0 nLambda+1])
% ylabel('Error (pixels)');
% xlabel('ANN size (incrementing)');
% legend(subList);
%
% suptitle(sprintf('%s: Error for %i different users',modelName,nUsers));
% pathToSave='';plotFileName='';plotFormatQ_final(pathToSave,plotFileName,0);

%% CIDER: AVG Graph over all user's errors

meanErrAllSub=mean(meanErrMatForAllSub,1);
stdErrAllSub=std(meanErrMatForAllSub,0,1);
indPerForAllSub=mean(indPerMatForAllSub,1);

if strcmp(modelName,'cider')
    
    meanErr_line_ForAllSub=mean(meanErrMat_line_ForAllSub,1);
    stdErr_line_ForAllSub=std(meanErrMat_line_ForAllSub,0,1);
    
    meanAnnUsedForAllSub=round(mean(meanAnnUsedMatForAllSub,1));
    stdAnnUsedForAllSub=(std(meanAnnUsedMatForAllSub,0,1));
    
    meanLineUsedForAllSub=round(mean(meanLineUsedMatForAllSub,1));
    stdLineUsedForAllSub=(std(meanLineUsedMatForAllSub,0,1));
else
    meanErr_line_ForAllSub=0;
    stdErr_line_ForAllSub=0;
    
    meanAnnUsedForAllSub=0;
    stdAnnUsedForAllSub=0;
    
    meanLineUsedForAllSub=0;
    stdLineUsedForAllSub=0;
end



end

%inside strcmp(--'target')
% figure;
% subplot(2,2,1);
% histogram(100*mat2gray(data.center.diff.raw.raw((data.filter & ~data.ann_used),:)));
% xlabel('Error percent of pupil center location')
% ylabel('Count')
% title('line model: error distribution')
% subplot(2,2,2);
% histogram(100*mat2gray(data.center.diff.raw.raw(data.filter,:)));
% title('cider model: error distribution')
% xlabel('Error percent of pupil center location')
% ylabel('Count')
% subplot(2,2,3);
% histogram(100*mat2gray(data.center.diff.raw.raw((data.filter & data.ann_used),:)));
% title('ann model: error distribution')
% xlabel('Error percent of pupil center location')
% ylabel('Count')
% suptitle('Pupil Center Error: All data including training data')
% figure;

