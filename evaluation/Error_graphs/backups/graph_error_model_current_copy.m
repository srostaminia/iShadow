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




function [errStEachSub,meanErrAllSub,stdErrAllSub,indPerForAllSub,meanErr_line_ForAllSub,stdErr_line_ForAllSub]=graph_error_model_current(modelName,subList,rootDir,lightingSubFolder)

uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
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

for i=1:length(subList)
    
    subName=subList{i};
    %errMatEachSub=zeros(nRep,nUni,length(lambdaFolderList)); %errSt 5x10
    indPerMatEachSub=zeros(nUni,length(lambdaFolderList));
    
    err_line_MatEachSub=zeros(nRep,nUni,length(lambdaFolderList));
    annUsedMatEachSub=zeros(nRep,nUni,length(lambdaFolderList));
    
    for j=1:length(uniFolderList)
        
        uni=uniFolderList{j};
        
        for k=1:length(lambdaFolderList)
            
            lambdaFolder=lambdaFolderList{k};
            
            for m=1:nRep
                
                dir= fullfile(rootDir,strcat(subName,lightingSubFolder),uni,'results',lambdaFolder,strcat(modelName,'_rep',num2str(m),'.mat'));
                data=load(dir);
                
                errMatEachSub(m,k,j)=mean(abs(data.sqrtRadEllipse-data.radii));
                indPerMatEachSub(m,k,j)=data.indActivePercent;
                
                if strcmp(modelName,'cider')
                    err_line_MatEachSub(m,k,j)=data.err_Line;
                    annUsedMatEachSub(m,k,j)=sum(data.ann_used);
                end
            end
        end
        
        
        if strcmp(modelName,'cider')
            annUsedStEachLambdaEachSub.raw.(uni).(subName)=annUsedMatEachSub(:,:,j); %5x10
            annUsedStEachLambdaEachSub.mean.(uni).(subName)=round(mean(annUsedMatEachSub(:,:,j),1));
            annUsedStEachLambdaEachSub.std.(uni).(subName)=std(annUsedMatEachSub(:,:,j),0,1);
        end
        
        errStEachSub.raw.(uni).(subName)=errMatEachSub(:,:,j);
        errStEachSub.mean.(uni).(subName)=mean(errMatEachSub(:,:,j),1);
        errStEachSub.std.(uni).(subName)=std(errMatEachSub(:,:,j),0,1);
        
        % indStEachSub.(uni).(subName)=indPerMatEachSub(:,j);
        
        indStEachSub.raw.(uni).(subName)=indPerMatEachSub(:,:,j);
        indStEachSub.mean.(uni).(subName)=mean(indPerMatEachSub(:,:,j),1);
        indStEachSub.std.(uni).(subName)=std(indPerMatEachSub(:,:,j),0,1);
        
        err_line_StEachSub.raw.(uni).(subName)=err_line_MatEachSub(:,:,j);
        err_line_StEachSub.mean.(uni).(subName)=mean(err_line_MatEachSub(:,:,j),1);
        err_line_StEachSub.std.(uni).(subName)=std(err_line_MatEachSub(:,:,j),0,1);
        
    end
    
end

if strcmp(modelName,'cider')
    totalFrameEachSub(i,:)=length(data.ann_used);
 
    if size(data.ann_used,1)~=size(data.gout,1)
        fprintf('ERROR: totalFrame is wrong. Check dimension\n\n');
        
    end
    
end

%compute mean for all subjects
meanErrMatForAllSub=zeros(nUsers,nLambda);
meanErrMat_line_ForAllSub=zeros(nUsers,nLambda);
indPerMatForAllSub=zeros(nUsers,nLambda);

for i=1:nUsers
    subName=subList{i};
    meanErrMatForAllSub(i,:)=errStEachSub.mean.uniquefy_0.(subName);
    meanErrMat_line_ForAllSub(i,:)=err_line_StEachSub.mean.uniquefy_0.(subName);
    indPerMatForAllSub(i,:)=indStEachSub.mean.uniquefy_0.(subName);
end

if strcmp(modelName,'cider')
    meanAnnUsedForEachLambdaEachSub=zeros(nUsers,nLambda);
    
    for i=1:nUsers %nUsers x nLambda: 6x10
        subName=subList{i};
        meanAnnUsedForEachLambdaEachSub(i,:)=annUsedStEachLambdaEachSub.mean.uniquefy_0.(subName);
        stdAnnUsedForEachLambdaEachSub(i,:)=annUsedStEachLambdaEachSub.std.uniquefy_0.(subName);
    end
end
%size(meanAnnUsedForEachLambdaEachSub)
%meanAnnUsedForEachLambdaEachSub

%% CIDER: Graph Subplot individual user's error
figure;
uni='uniquefy_0'%%%f
for i=1:nUsers  
    subName=subList{i};
    
    subplot(2,3,i);
    errorbar(1:1:nLambda,errStEachSub.mean.(uni).(subName),errStEachSub.std.(uni).(subName));
    ylim([0, max(errStEachSub.mean.(uni).(subName))+max(errStEachSub.std.(uni).(subName))+0.1])
    xlim([0 nLambda+1])
    ylabel('Error (pixels)');
    xlabel('ANN size (incrementing)');
    title(strcat('subject: ',subName));
    
end

suptitle({strcat(modelName,': Error of predicted pupil center for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});
plotFormatQ_final(pathToSave,plotFileName,0);

%% CIDER: One Graph Subplot individual user's error

figure;
for i=1:nUsers
    
    subName=subList{i};
    errorbar(1:1:nLambda,errStEachSub.mean.(uni).(subName),errStEachSub.std.(uni).(subName));
    %meanErrMatForAllSub(i,:)=errStEachSub.mean.uniquefy_0.(subName);
    hold on;
end

hold off;
%ylim([0, 2.9]) %10.1
xlim([0 nLambda+1])
ylabel('Error (pixels)');
xlabel('ANN size (incrementing)');
%title(strcat('subject: ',subName));
legend(subList);

suptitle({strcat(modelName,': Error of predicted pupil center for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});
plotFormatQ_final(pathToSave,plotFileName,0);

%% CIDER: AVG Graph over all user's errors

meanErrAllSub=mean(meanErrMatForAllSub,1);
stdErrAllSub=std(meanErrMatForAllSub,0,1);
indPerForAllSub=mean(indPerMatForAllSub,1);

figure;
errorbar(1:1:nLambda,meanErrAllSub,stdErrAllSub);
ylim([0 max(meanErrAllSub+stdErrAllSub+0.5)])
xlim([0 nLambda+1])
ylabel('Average Error (pixels)');
xlabel('ANN size (incrementing)');

suptitle({strcat(modelName,': Average error of predicted pupil center for different ANN sizes'),'Each of the ten points is obtained by averaging the mean errors for',' 5 repetitions of the experiment for different users.','ANN size increases from left to right.'});
plotFormatQ_final(pathToSave,plotFileName,0);



%% Line model CIDER: Graph Subplot individual user's error

if strcmp(modelName,'cider')
    modelName='lineCider'
    
    figure;
    % uniFolderList={'uniquefy_0'};%,'uniquefy_1'};
    % lambdaFolderList={'subset_l1_init_strips_k7_lambda0.000100','subset_l1_init_strips_k7_lambda0.000215','subset_l1_init_strips_k7_lambda0.000464','subset_l1_init_strips_k7_lambda0.001000','subset_l1_init_strips_k7_lambda0.002154','subset_l1_init_strips_k7_lambda0.004642','subset_l1_init_strips_k7_lambda0.010000','subset_l1_init_strips_k7_lambda0.021544','subset_l1_init_strips_k7_lambda0.046416','subset_l1_init_strips_k7_lambda0.100000'};
    % lambdaStrList={'lambda0.000100','lambda0.000215','lambda0.000464','lambda0.001000','lambda0.002154','lambda0.004642','lambda0.010000','lambda0.021544','lambda0.046416','lambda0.100000'};
    % lambdaStrList=fliplr(lambdaStrList);
    % lambdaFolderList=fliplr(lambdaFolderList);
    
    %nLambda=length(lambdaStrList);
    
    uni='uniquefy_0'%%%f
    
    for i=1:nUsers
        
        subName=subList{i};
        
        subplot(2,3,i);
        errorbar(1:1:nLambda,err_line_StEachSub.mean.(uni).(subName),err_line_StEachSub.std.(uni).(subName));
        ylim([0, max(err_line_StEachSub.mean.(uni).(subName))+max(err_line_StEachSub.std.(uni).(subName))+0.1])
        xlim([0 nLambda+1])
        ylabel('Error (pixels)');
        xlabel('ANN size (incrementing)');
        title(strcat('subject: ',subName));
        
    end
    
    suptitle({strcat(modelName,': Error of predicted pupil center for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});
    plotFormatQ_final(pathToSave,plotFileName,0);

    %% Line CIDER: One Graph Subplot individual user's error
    
    figure;
    for i=1:nUsers
        
        subName=subList{i};
        errorbar(1:1:nLambda,err_line_StEachSub.mean.(uni).(subName),err_line_StEachSub.std.(uni).(subName));
        %meanErrMatForAllSub(i,:)=err_line_StEachSub.mean.uniquefy_0.(subName);
        hold on;
    end
    
    hold off;
    %ylim([0, 2.9]) %10.1
    xlim([0 nLambda+1])
    ylabel('Error (pixels)');
    xlabel('ANN size (incrementing)');
    %title(strcat('subject: ',subName));
    legend(subList);
    
    suptitle({strcat(modelName,': Error of predicted pupil center for 6 different users for different ANN sizes'),'Each of the ten points represents the average of errors for 5 repetitions of the experiment.','ANN size increases from left to right.'});
    plotFormatQ_final(pathToSave,plotFileName,0);

    %% Line CIDER: AVG Graph over all user's errors
    
    meanErr_line_ForAllSub=mean(meanErrMat_line_ForAllSub,1);
    stdErr_line_ForAllSub=std(meanErrMat_line_ForAllSub,0,1);
    
    figure;
    errorbar(1:1:nLambda,meanErr_line_ForAllSub,stdErr_line_ForAllSub);
    ylim([0 max(meanErr_line_ForAllSub+stdErr_line_ForAllSub+0.5)])
    xlim([0 nLambda+1])
    ylabel('Average Error (pixels)');
    xlabel('ANN size (incrementing)');
    
    suptitle({strcat(modelName,': Average error of predicted pupil center for different ANN sizes'),'Each of the ten points is obtained by averaging the mean errors for',' 5 repetitions of the experiment for different users.','ANN size increases from left to right.'});
    plotFormatQ_final(pathToSave,plotFileName,0);

else
    meanErr_line_ForAllSub=0;
    stdErr_line_ForAllSub=0;
end

end