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




function [errStEachSub,meanErrAllSub,stdErrAllSub,indPerForAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub,meanAnnUsedForAllSub,stdAnnUsedForAllSub,meanLineUsedForAllSub,stdLineUsedForAllSub]=graph_error_model_current(indFile,linestyle,modelName,subList,rootDir,lightingSubFolder,objToPlot,subModelName)

uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
uni=uniFolderList{1};
lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
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

ori=pwd;
cd (rootDir);
subNameFolderCell=cellstr(strsplit(ls));
cd(ori);

for i=1:nUsers
    
    subName=subList{i}; %subject name
    indFile=i;
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
            
            
            dir= fullfile(rootDir,strcat(subNameFolder),uni,'results',lambdaFolder,strcat(modelName,'_',subModelName,'rep',num2str(m),'.mat'));
            
            data=load(dir);
            
            
            if strcmp(objToPlot,'area')
                if strcmp(modelName,'cider')
                    errMatEachSub(m,k,j)=data.mean_cider.area.diff_perc;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                    err_line_MatEachSub(m,k,j)=data.mean_line.area.diff_perc;
                    annUsedMatEachSub(m,k,j)=100*sum(data.ann_used)/length((data.ann_used));
                    
                elseif strcmp(modelName,'ann')
                    errMatEachSub(m,k,j)=data.mean_ann.area.diff_perc;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                end
            elseif strcmp(objToPlot,'pupilLoc_deg')
                
                if strcmp(modelName,'cider')
                    errMatEachSub(m,k,j)=data.mean_cider.center.diff_deg;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                    err_line_MatEachSub(m,k,j)=data.mean_line.center.diff_deg;
                    annUsedMatEachSub(m,k,j)=100*sum(data.ann_used)/length((data.ann_used));
                    
                elseif strcmp(modelName,'ann')
                    errMatEachSub(m,k,j)=data.mean_ann.center.diff_deg;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                end
            elseif strcmp(objToPlot,'pupilLoc_pixel')
                
                if strcmp(modelName,'cider')
                    %data.err;%
                    errMatEachSub(m,k,j)=data.mean_cider.center.diff;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                    err_line_MatEachSub(m,k,j)=data.mean_line.center.diff;%data.err_Line;%
                    annUsedMatEachSub(m,k,j)=100*sum(data.ann_used)/length((data.ann_used));
                    
                    
                    %                     figure;
                    %                     subplot(2,2,1);
                    %                     histogram(100*mat2gray(data.center.diff.raw((data.filter & ~data.ann_used),:)));
                    %                     xlabel('Error percent of pupil center location')
                    %                     ylabel('Count')
                    %                     title('line model: error distribution')
                    %                     subplot(2,2,2);
                    %                     histogram(100*mat2gray(data.center.diff.raw(data.filter,:)));
                    %                     title('cider model: error distribution')
                    %                     xlabel('Error percent of pupil center location')
                    %                     ylabel('Count')
                    %                     subplot(2,2,3);
                    %                     histogram(100*mat2gray(data.center.diff.raw((data.filter & data.ann_used),:)));
                    %                     title('ann model: error distribution')
                    %                     xlabel('Error percent of pupil center location')
                    %                     ylabel('Count')
                    %                     suptitle('Pupil Center Error: All data including training data')
                    %                     figure;
                    %
                    
                    
                    
                elseif strcmp(modelName,'ann')
                    %data.err;%
                    errMatEachSub(m,k,j)=data.mean_ann.center.diff;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                end
                %errMatEachSub(m,k,j)=data.center_err;
            elseif strcmp(objToPlot,'radius')
                if strcmp(modelName,'cider')
                    errMatEachSub(m,k,j)=data.mean_cider.radius.diff;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                    err_line_MatEachSub(m,k,j)=data.mean_line.radius.diff;
                    annUsedMatEachSub(m,k,j)=100*sum(data.ann_used)/length((data.ann_used));
                    
                elseif strcmp(modelName,'ann')
                    errMatEachSub(m,k,j)=data.mean_ann.radius.diff;%mean(data.perAreaDiff);%100*data.diffArea./(pi*data.sqrtRadEllipse(( logical(sum(data.pred,2))),:).^2);
                end
            end
            
            %indPerMatEachSub(m,k,j)=data.indActivePercent;
            
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
    indStEachSub.std.(uni).(subName)=std(indPerMatEachSub(:,:,j),0,1);
end



%compute mean for all subjects
meanErrMatForAllSub=zeros(nUsers,nLambda);
meanErrMat_line_ForAllSub=zeros(nUsers,nLambda);
indPerMatForAllSub=zeros(nUsers,nLambda);
meanAnnUsedMatForAllSub=zeros(nUsers,nLambda);

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
        meanErrMat_line_ForAllSub(i,:)=err_line_StEachSub.mean.(uni).(subName);
    end
    
end
%% CIDER: Graph Subplot individual user's error
figure;

for i=1:nUsers

    subName=subList{i};
    errorbar(1:1:nLambda,meanErrMatForAllSub(i,:),errStEachSub.std.(uni).(subName));
    hold on;
end

hold off;
limsy=get(gca,'YLim');
set(gca,'Ylim',[0 limsy(2)]);
%ylim([0, 2.9]) %10.1
xlim([0 nLambda+1])
ylabel('Error (pixels)');
xlabel('ANN size (incrementing)');
legend(subList);

suptitle({strcat(modelName,': Error of predicted pupil center for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});
pathToSave='';plotFileName='';plotFormatQ_final(pathToSave,plotFileName,0);

%% CIDER: AVG Graph over all user's errors

meanErrAllSub=mean(meanErrMatForAllSub,1);
stdErrAllSub=std(meanErrMatForAllSub,0,1);
indPerForAllSub=mean(indPerMatForAllSub,1);

figure;
if strcmp(modelName,'cider')
    %figure;
        errorbar(1:1:nLambda,errStEachSub.mean.(uni).(subName),errStEachSub.std.(uni).(subName),strcat(linestyle,'r'),'markers',10);

    %errorbar(1:1:nLambda,meanErrAllSub,stdErrAllSub,strcat(linestyle,'r'),'markers',10);
else
        errorbar(1:1:nLambda,errStEachSub.mean.(uni).(subName),errStEachSub.std.(uni).(subName),strcat(linestyle,'b'),'markers',10);

    %errorbar(1:1:nLambda,meanErrAllSub,stdErrAllSub,strcat(linestyle,'b'),'markers',10);
end
hold on;
% limsy=get(gca,'YLim');
% set(gca,'Ylim',[0 limsy(2)]);
% %ylim([0 max(meanErrAllSub+stdErrAllSub+0.5)])
% xlim([0 nLambda+1])
% ylabel('Average Error (pixels)');
% xlabel('ANN size (incrementing)');
% suptitle(strcat(modelName,': Pupil Area'))

%suptitle({strcat(modelName,': Average error of predicted pupil center for different ANN sizes'),'Each of the ten points is obtained by averaging the mean errors for',' 5 repetitions of the experiment for different users.','ANN size increases from left to right.'});
%pathToSave='';plotFileName='';plotFormatQ_final(pathToSave,plotFileName,0);


%% Line CIDER: One Graph Subplot individual user's error

if strcmp(modelName,'cider')
    
    %     figure;
    %     for i=1:nUsers
    %
    %         subName=subList{i};
    %         errorbar(1:1:nLambda,err_line_StEachSub.mean.(uni).(subName),err_line_StEachSub.std.(uni).(subName));
    %         %meanErrMatForAllSub(i,:)=err_line_StEachSub.mean.(uni).(subName);
    %         hold on;
    %     end
    %
    %     hold off;
    %     limsy=get(gca,'YLim');
    %     set(gca,'Ylim',[0 limsy(2)]);
    %     xlim([0 nLambda+1])
    %     ylabel('Error (pixels)');
    %     xlabel('ANN size (incrementing)');
    %     %title(strcat('subject: ',subName));
    %     legend(subList);
    %
    %     suptitle({strcat(modelName,': Error of predicted pupil center for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});
    %     pathToSave='';plotFileName='';plotFormatQ_final(pathToSave,plotFileName,0);
    %
    %% Line CIDER: AVG Graph over all user's errors
    
    meanErr_line_ForAllSub=mean(meanErrMat_line_ForAllSub,1);
    stdErr_line_ForAllSub=std(meanErrMat_line_ForAllSub,0,1);
    
    meanAnnUsedForAllSub=mean(meanAnnUsedMatForAllSub,1);
    stdAnnUsedForAllSub=std(meanAnnUsedMatForAllSub,0,1);
    
    meanLineUsedForAllSub=round(mean(100-meanAnnUsedMatForAllSub,1));
    stdLineUsedForAllSub=round(std(100-meanAnnUsedMatForAllSub,0,1));
    
    %figure;
    errorbar(1:1:nLambda,meanErr_line_ForAllSub,stdErr_line_ForAllSub,strcat(linestyle,'m'),'markers',10);
    %         limsy=get(gca,'YLim');
    %         set(gca,'Ylim',[0 limsy(2)]);
    %ylim([0 max(meanErr_line_ForAllSub+stdErr_line_ForAllSub+0.5)])
    xlim([0 nLambda+1])
    %ylim([0 12]);
    ylabel('Average Error (pixels)');
    xlabel('ANN size (incrementing)');
    %suptitle('lineCider: Pupil Area')
    
    suptitle(strcat('Error of ',objToPlot,': ',subNameFolder));
    
    
    
    
    
    %suptitle({strcat('lineCider',': Average error of predicted pupil center for different ANN sizes'),'Each of the ten points is obtained by averaging the mean errors for',' 5 repetitions of the experiment for different users.','ANN size increases from left to right.'});
    pathToSave='';plotFileName='';plotFormatQ_final(pathToSave,plotFileName,0);
    
else
    meanErr_line_ForAllSub=0;
    stdErr_line_ForAllSub=0;
    
    meanAnnUsedForAllSub=0;
    stdAnnUsedForAllSub=0;
end



end